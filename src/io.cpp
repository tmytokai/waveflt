// I/O

#include <stdio.h>
#include <string.h>
#include <io.h> // _setmode
#include <fcntl.h> // _O_BINARY

#include "waveformat.h"

#define CHR_BUF 256 


const unsigned int ReadData( FILE* fp, unsigned char* buffer, const unsigned int size )
{
	unsigned int readsize, remain;

	readsize = 0;
	remain = size;
	while( readsize < remain ){
		unsigned int tmp = fread(buffer+readsize,1,remain,fp);
		readsize += tmp;
		remain -= tmp;
		if( tmp == 0 || remain == 0) break;
	}
				
	return readsize;
}


const __int64 SeekStdin( unsigned char* buffer,
			   unsigned int buffersize,
			   const __int64 pos_seek,
			   const __int64 pos_current  // pos_seek > pos_current
			   )
{
	if( pos_seek <= pos_current) return 0;

	unsigned int readsize, remain;

	readsize = 0;
	remain = (unsigned int)(pos_seek - pos_current);
	if( remain > buffersize ) remain = buffersize;

	while( readsize < remain ){
		unsigned int tmp = fread(buffer+readsize,1,remain,stdin);
		readsize +=tmp;
		remain -= tmp;
		if( tmp == 0 || remain == 0) break;
	}

	return readsize;
}


const unsigned int WriteData( FILE* fp, const unsigned char* buffer, const unsigned int size )
{
	if( fp  == NULL ) return -1;
	return fwrite( buffer, 1, size, fp);
}


const bool WriteTextData( FILE* fp, double* buffer[2], const unsigned int size, const WAVFMT format)
{
	if( fp == NULL) return true;

	for( unsigned int pos = 0; pos < size ; pos++){

		char out[CHR_BUF];
		int level = (int)buffer[0][pos];
		_snprintf(out, CHR_BUF,"%12d",level);
		if(format.channels == 2){
			level = (int)buffer[1][pos];
			_snprintf(out, CHR_BUF,"%s %12d",out,level);
		}
		_snprintf(out, CHR_BUF,"%s\r\n",out);
		
		fwrite( out, 1 ,strlen(out), fp);
	}

	return true;
}


const bool OpenReadFile( FILE** fp, const char* filename )
{
	// stdin
	if(strncmp(filename,"stdin", 5)==0){
		*fp = stdin;
#ifdef WIN32
		_setmode(_fileno(stdin),_O_BINARY); // set binary mode
#endif
	}
	else if(strncmp(filename,"nosignal",8)==0){ // nosignal
			*fp = NULL;
	}
	else // storage
	{
		*fp = fopen(filename,"rb");
		if(fp == NULL) {
			fprintf(stderr,"\nCannot open '%s'\n",filename);
			return false;;
		}	
	}

	return true;
}


const bool OpenWriteFile( FILE** fp, const char* filename )
{
	// stdout
	if(strncmp(filename,"stdout", 6)==0) {
		*fp = stdout;
#ifdef WIN32
		_setmode(_fileno(stdout),_O_BINARY); 
#endif
	}
	else if(strncmp(filename,"null",4)==0){ // NULL
		*fp = NULL;
	}
	else // storage
	{
		*fp = fopen(filename,"wb");
		if(fp == NULL) {
			fprintf(stderr,"\nCannot open '%s'\n",filename);
			return false;;
		}	
	}

	return true;

}