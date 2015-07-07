#ifndef _WAVE_H
#define _WAVE_H

class WaveFormat;

void WaveLevel(double dLevel[2],  // output of left and right
			   const unsigned char* lpWaveData,  // input
			   const WaveFormat& format);

#endif