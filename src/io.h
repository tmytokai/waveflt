#ifndef _IO_H
#define _IO_H

const unsigned int ReadData( FILE* fp, unsigned char* buffer, const unsigned int size );
#ifdef WIN32
const __int64 SeekStdin( unsigned char* buffer,
			   unsigned int buffersize,
			   const __int64 pos_seek,
			   const __int64 pos_current  // pos_seek > pos_current
			   );
#endif
const unsigned int WriteData( FILE* fp, const unsigned char* buffer, const unsigned int size );
const bool WriteTextData( FILE* fp,  double* buffer[2], const unsigned int size, const WaveFormat format);
const bool OpenReadFile( FILE** fp, const char* filename );
const bool OpenWriteFile( FILE** fp, const char* filename );

#endif
