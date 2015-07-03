#ifndef _WAVE_H
#define _WAVE_H

#include "waveformat.h"


void SetWaveFormat(WAVFMT& format, 
				   const unsigned short tag,	
				   const unsigned short channels, 
				   const unsigned int rate, 
				   const unsigned short bits );

void WriteWaveHeader( HANDLE hdWriteFile, const WAVFMT& format, const LONGLONG n64WaveDataSize ,const BOOL bUseExtChunk );

const bool IsWaveFormatValid( const WAVFMT& format,  char* errmsg );

const bool GetWaveFormat(const char* filename, // name or 'stdin'
				   WAVFMT& format, 
				   unsigned long long& datasize, // data size (byte)
				   unsigned long long& offset, // offset to data chunk (byte)
				   char* errmsg );

LONGLONG SeekStdin(LPBYTE lpBuffer,
			   DWORD dwBufSize,
			   LONGLONG n64SeekPointer,
			   LONGLONG n64CurFilePointer  // dwSeekPointer > dwCurFilePointer
			   );

void WaveLevel(double dLevel[2],  // output of left and right
			   const BYTE* lpWaveData,  // input
			   const WAVFMT& format);

const double GetMaxWaveLevel(const WAVFMT& format);

#endif