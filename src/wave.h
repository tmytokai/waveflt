#ifndef _WAVE_H
#define _WAVE_H

#include <stdio.h>

#include "waveformat.h"


void SetWaveFormat(WAVFMT& format, 
				   const unsigned short tag,	
				   const unsigned short channels, 
				   const unsigned int rate, 
				   const unsigned short bits );

void WriteWaveHeader ( FILE* fp, const WAVFMT& format, const __int64 size, const bool extchunk );

const bool IsWaveFormatValid( const WAVFMT& format,  char* errmsg );

const bool GetWaveFormat(const char* filename, // name or 'stdin'
				   WAVFMT& format, 
				   unsigned long long& datasize, // data size (byte)
				   unsigned long long& offset, // offset to data chunk (byte)
				   char* errmsg );

void WaveLevel(double dLevel[2],  // output of left and right
			   const unsigned char* lpWaveData,  // input
			   const WAVFMT& format);

const double GetMaxWaveLevel(const WAVFMT& format);

#endif