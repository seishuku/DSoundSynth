#include <dsound.h>
#include <stdio.h>
#include <math.h>
#include <conio.h>

LPDIRECTSOUND8 lpDS;
LPDIRECTSOUNDBUFFER lpDSB;
DSBUFFERDESC dsbd;
HWND hWnd;

int SampleRate=44100;

short SineTable[256]; // Sine wave lookup table
unsigned short phase=0; // Current phase in sine wave

const float TwoPI=2.0f*3.1415926f;

float midi[127]; // MIDI note to frequency table

unsigned char Done=0;

int InitDSound(void)
{
	WAVEFORMATEX wfx;

	if(DirectSoundCreate8(NULL, &lpDS, NULL)!=DS_OK)
		return 0;

	if(IDirectSound8_SetCooperativeLevel(lpDS, hWnd, DSSCL_NORMAL)!=DS_OK)
		return 0;

	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag=WAVE_FORMAT_PCM;
	wfx.nChannels=2;
	wfx.wBitsPerSample=16;
	wfx.nSamplesPerSec=SampleRate;
	wfx.nBlockAlign=4;
	wfx.nAvgBytesPerSec=wfx.nSamplesPerSec*wfx.nChannels*(wfx.wBitsPerSample>>3);

	memset(&dsbd, 0, sizeof(DSBUFFERDESC));
	dsbd.dwSize=sizeof(DSBUFFERDESC);
	dsbd.dwFlags=DSBCAPS_CTRLPOSITIONNOTIFY|DSBCAPS_GETCURRENTPOSITION2;
	dsbd.dwBufferBytes=wfx.nAvgBytesPerSec;
	dsbd.lpwfxFormat=&wfx;

	if(IDirectSound_CreateSoundBuffer(lpDS, &dsbd, &lpDSB, NULL)!=DS_OK)
		return 0;

	return 1;
}

void PlayNote(float freq)
{
	LPVOID lpvWrite;
	short *ptr, output;
	DWORD dwLength;

	IDirectSoundBuffer_Stop(lpDSB);

	if(IDirectSoundBuffer_Lock(lpDSB, 0, dsbd.dwBufferBytes, &lpvWrite, &dwLength, NULL, 0, DSBLOCK_ENTIREBUFFER)==DS_OK)
	{
		ptr=(short *)lpvWrite;

		float duration=0.38f*SampleRate;
		float attackTime=0.0001f*SampleRate;
		float decayTime=0.1f*SampleRate;
		float decayStart=duration-decayTime;
		float peakAmp=1.0f;
		float startAmp=0.0f;
		float endAmp=0.0f;
		float volume=startAmp;

		float envInc=(peakAmp-startAmp)/attackTime;

		for(int i=0;i<SampleRate;i++)
		{
			if(i<(int)duration)
			{
				if(i<attackTime||i>decayStart)
					volume+=envInc;
				else if(i==decayStart)
				{
					envInc=endAmp-volume;

					if(decayTime>0)
						envInc/=decayTime;
				}
				else
					volume=peakAmp;

				output=((float)SineTable[phase>>8]*volume);
				phase+=(65535.0f*freq/SampleRate);

				ptr[0]=output;
				ptr[1]=output;
				ptr+=2;
			}
			else
			{
				ptr[0]=0;
				ptr[1]=0;
				ptr+=2;
			}
		}

		IDirectSoundBuffer_Unlock(lpDSB, lpvWrite , dwLength , NULL , 0);
	}

	IDirectSoundBuffer_SetCurrentPosition(lpDSB, 0);
	IDirectSoundBuffer_Play(lpDSB, 0 , 0 , 0);
}

int main(int argc, char *argv[])
{
	SetConsoleTitle("Simple DirectSound Synth");
	hWnd=FindWindow(NULL, "Simple DirectSound Synth");

	if(!InitDSound())
		exit(-1);

	// Generate MIDI -> Frequency table
	for(int i=0;i<127;i++)
		midi[i]=440.0f*pow(2, ((float)i-69.0f)/12.0f);

	// Generate sine wave table
	for(int i=0;i<256;i++)
		SineTable[i]=(short)(32767.0f*sin(TwoPI*((float)i/256.0f)));

	printf("Simple synthesizer\n\nSharps: S D G H J K\nWhole notes: Z X C V B N M\n\nQ to quit\n\n");

	while(!Done)
	{
		if(_kbhit())
		{
			switch(getch())
			{
				case 'q':
					Done=1;
					break;

				case 'z':
					PlayNote(midi[60]); // C
					printf("C %3.3fHz\n", midi[60]);
					break;

				case 's':
					PlayNote(midi[61]); // C#
					printf("C# %3.3fHz\n", midi[61]);
					break;

				case 'x':
					PlayNote(midi[62]); // D
					printf("D %3.3fHz\n", midi[62]);
					break;

				case 'd':
					PlayNote(midi[63]); // D#
					printf("D# %3.3fHz\n", midi[63]);
					break;

				case 'c':
					PlayNote(midi[64]); // E
					printf("E %3.3fHz\n", midi[64]);
					break;

				case 'v':
					PlayNote(midi[65]); // F
					printf("F %3.3fHz\n", midi[65]);
					break;

				case 'g':
					PlayNote(midi[66]); // F#
					printf("F# %3.3fHz\n", midi[66]);
					break;

				case 'b':
					PlayNote(midi[67]); // G
					printf("G %3.3fHz\n", midi[67]);
					break;

				case 'h':
					PlayNote(midi[68]); // G#
					printf("G# %3.3fHz\n", midi[68]);
					break;

				case 'n':
					PlayNote(midi[69]); // A
					printf("A %3.3fHz\n", midi[69]);
					break;

				case 'j':
					PlayNote(midi[70]); // A#
					printf("A# %3.3fHz\n", midi[70]);
					break;

				case 'm':
					PlayNote(midi[71]); // B
					printf("B %3.3fHz\n", midi[71]);
					break;

				case 'k':
					PlayNote(midi[72]); // B#
					printf("B# %3.3fHz\n", midi[72]);
					break;

				default:
					break;
			}
		}
	}

	IDirectSoundBuffer_Release(lpDSB);
	IDirectSound_Release(lpDS);

	return 0;
}
