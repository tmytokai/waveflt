// common wave functions

#ifdef WIN32
#include <windows.h>
#define USEWIN32API  // use win32 APIs
#endif

#include <stdio.h>
#include <io.h> // _setmode
#include <fcntl.h> // _O_BINARY
#include <assert.h>

#include "wave.h"

#define CHR_BUF 256 


void SetWaveFormat(WAVFMT& format, 
				   const unsigned short tag,	
				   const unsigned short channels, 
				   const unsigned int rate, 
				   const unsigned short bits )
{
	format.tag = tag;
	format.channels = channels;
	format.rate = rate;
	format.avgbyte = (unsigned int)(rate*channels*bits/8);
	format.block = (unsigned short)(channels*bits/8);
	format.bits = bits;
}


void WriteWaveHeader( HANDLE hdWriteFile, const WAVFMT& format, const LONGLONG n64WaveDataSize ,const BOOL bUseExtChunk )
{	
	DWORD dwFoo;
	DWORD dwByte; 
	LARGE_INTEGER LI; 
	DWORD dwFileType;
	DWORD dwHeadSize;

	dwHeadSize = 44;
	if( bUseExtChunk ) dwHeadSize += (8 + 8 + 4);

	if(hdWriteFile == NULL) return;

	// get type of file
	dwFileType = GetFileType(hdWriteFile);
	
	// move file pointer
	if(dwFileType != FILE_TYPE_PIPE){
		LI.QuadPart = 0;
		SetFilePointer(hdWriteFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
	}
	
	// RIFF (8)
	WriteFile(hdWriteFile, "RIFF",4, &dwByte, NULL);
	if(n64WaveDataSize >= 0xFFFFFFFF-dwHeadSize+8) dwFoo = 0xFFFFFFFF; // = 4G
	else dwFoo = ((DWORD)n64WaveDataSize + dwHeadSize) - 8;
	WriteFile(hdWriteFile, &dwFoo,sizeof(DWORD), &dwByte, NULL);

	// WAVE (4)
	WriteFile(hdWriteFile, "WAVE",4, &dwByte, NULL);

	// format chunk (24)
	WriteFile(hdWriteFile, "fmt ",4, &dwByte, NULL);
	dwFoo = 16;
	WriteFile(hdWriteFile, &dwFoo,sizeof(DWORD), &dwByte, NULL);
	WriteFile(hdWriteFile, &format, sizeof(WAVFMT), &dwByte, NULL);
	
	// waveflt chunk (extra chunk)
	if(bUseExtChunk){
			WriteFile(hdWriteFile, "wflt",4, &dwByte, NULL);
			dwFoo = sizeof(LONGLONG) + sizeof(DWORD);
			WriteFile(hdWriteFile, &dwFoo,sizeof(DWORD), &dwByte, NULL);
			WriteFile(hdWriteFile, &n64WaveDataSize, sizeof(LONGLONG), &dwByte, NULL);
			dwFoo = 0;
			WriteFile(hdWriteFile, &dwFoo,sizeof(DWORD), &dwByte, NULL);
	}
	
	// data chunk (8  + datasize)
	WriteFile(hdWriteFile, "data",4, &dwByte, NULL);
	if(n64WaveDataSize >= 0xFFFFFFFF-dwHeadSize+8) dwFoo = 0xFFFFFFFF-dwHeadSize+8; // = 4G
	else dwFoo = (DWORD)n64WaveDataSize;
	WriteFile(hdWriteFile, &dwFoo,sizeof(DWORD), &dwByte, NULL);
}



const bool IsWaveFormatValid( const WAVFMT& format,  char* errmsg )
{
	assert( errmsg );

	if( format.tag != WAVE_FORMAT_IEEE_FLOAT && format.tag != WAVE_FORMAT_PCM ){
		strncpy( errmsg, "Not PCM format.\n", CHR_BUF);
		return false;
	}
 
	if( format.channels != 1 && format.channels != 2){
		_snprintf( errmsg, CHR_BUF ,"'%d channels' is not supported.\n", format.channels);
		return false;
	}	

	if( format.bits != 8 && format.bits != 16 && format.bits != 24 && format.bits != 32 && format.bits != 64){
		_snprintf(errmsg, CHR_BUF, "'%d bit' is not supported.\n", format.bits);
		return false;
	}
	
	if(format.avgbyte != format.channels*format.rate*format.bits/8){
		strncpy(errmsg,"Invalid wave format.\n", CHR_BUF);
		return false;
	}

	return true;
}



enum
{
	ID_err = 0,
	ID_riff,
	ID_wave,
	ID_fmt,
	ID_data,
	ID_fact,
	ID_bext,
	ID_cue,
	ID_list,
	ID_plst,
	ID_junk,
	ID_wflt, // extension
	ID_unknown
};


const int GetChunkID( FILE* fp,	 char* chunk,  unsigned int& chunksize )
{
	assert( fp );
	assert( chunk );

	unsigned int byte;
	chunksize = 0;

	byte = fread( chunk, 1, 4, fp);
	if(byte != 4) return ID_err;

	if( strncmp(chunk,"WAVE",4) ==0 ) return ID_wave;

	byte = fread(&chunksize,1,sizeof(unsigned int),fp);
	if(byte != sizeof(unsigned int)) return ID_err;

	if( strncmp(chunk,"RIFF",4) ==0 ) return ID_riff;
	if( strncmp(chunk,"fmt ",4) ==0 ) return ID_fmt;
	if( strncmp(chunk,"data",4) ==0 ) return ID_data;
	if( strncmp(chunk,"fact",4) ==0 ) return ID_fact;
	if( strncmp(chunk,"bext",4) ==0 ) return ID_bext;
	if( strncmp(chunk,"cue ",4) ==0 ) return ID_cue;
	if( strncmp(chunk,"LIST",4) ==0 ) return ID_list;
	if( strncmp(chunk,"plst",4) ==0 ) return ID_plst;
	if( strncmp(chunk,"JUNK",4) ==0 ) return ID_junk;
	if( strncmp(chunk,"wflt",4) ==0 ) return ID_wflt;

	return ID_unknown;
}


const bool GetWaveFormat(const char* filename, // name or 'stdin'
				   WAVFMT& format, 
				   unsigned long long& datasize, // data size (byte)
				   unsigned long long& offset, // offset to data chunk (byte)
				   char* errmsg )
{
	assert( filename );
	assert( errmsg );

	FILE* fp = NULL;
	bool ret = false;

	datasize = 0;
	offset = 0;
	errmsg[0] = '\0';

	// stdin
	if(strncmp(filename,"stdin", 5)==0){
		fp = stdin;
#ifdef WIN32
		_setmode(_fileno(stdin),_O_BINARY); // set binary mode
#endif
	}
	// file
	else{
		fp = fopen(filename,"rb");
		if(fp == NULL) {
			_snprintf(errmsg, CHR_BUF, "Cannot open '%s'\n", filename);
			goto L_ERR;
		}
	}

	char chunk[5] = {0};
	unsigned int chunksize;

	if( GetChunkID( fp, chunk, chunksize ) != ID_riff ){
		strcpy(errmsg,"This is not wave file.\n");
		goto L_ERR;	
	}
	offset += 8;

	while(1){

		unsigned int byte;
		const int id = GetChunkID( fp, chunk, chunksize );

		if( id == ID_err ){
			strncpy(errmsg,"Invalid wave header.\n", CHR_BUF);
			goto L_ERR;	
		}
		else if( id == ID_unknown ){
			_snprintf(errmsg, CHR_BUF, "Unknown chunk '%s'.\n", chunk);
			goto L_ERR;	
		}
		else if( id == ID_wave ){
			offset += 4;
			continue;
		}
		else if( id == ID_fmt ){
			offset += 8;
			byte = fread(&format,1,chunksize,fp);
			if(byte != chunksize){
				strncpy(errmsg, "Invalid fmt chunk.\n", CHR_BUF);
				goto L_ERR;
			}
			offset += byte;  
			if(!IsWaveFormatValid(format, errmsg)) goto L_ERR;
		}
		else if( id == ID_wflt ){
			offset += 8;
			byte = fread( &datasize, 1, sizeof(unsigned long long), fp);
			if(byte != sizeof(unsigned long long)){
				strncpy(errmsg, "Invalid wflt chunk.\n", CHR_BUF);
				goto L_ERR;
			}
			offset += byte;  
		}
		else if( id == ID_data ){
			offset += 8;
			if( datasize == 0) datasize = chunksize;
			break;
		}
		else{
			offset += (8 + chunksize);
			__int64 pos64 = offset;
			_fseeki64( fp , pos64, SEEK_SET);
		}
	}
			
	ret = true;

	L_ERR: 

	// close file
	if( fp && fp != stdin ) fclose(fp);

	return ret;
}




//-------------------------------------------------------------------
// seek from stdin
LONGLONG SeekStdin(LPBYTE lpBuffer,
			   DWORD dwBufSize,
			   LONGLONG n64SeekPointer,
			   LONGLONG n64CurFilePointer  // n64SeekPointer > n64CurFilePointer
			   )
{
	
	DWORD dwByte,dwReadByte;
	LONGLONG n64ReadSize,n64TotalByte;

	if(n64SeekPointer <= n64CurFilePointer) return 0;

	n64ReadSize = n64SeekPointer - n64CurFilePointer;
	n64TotalByte = 0;

	while(1){

		if(n64ReadSize > n64TotalByte + dwBufSize) dwReadByte = dwBufSize;
		else dwReadByte = (DWORD)(n64ReadSize - n64TotalByte);
		if(dwReadByte == 0) break;

#ifdef USEWIN32API // use win32 API 
		ReadFile(GetStdHandle(STD_INPUT_HANDLE),lpBuffer,dwReadByte,&dwByte, NULL);
#else  
		dwByte = fread(lpBuffer,1,dwReadByte,stdin);
#endif	
		if(dwByte == 0) break;
		n64TotalByte += dwByte;
	}

	return n64TotalByte;
}


//-------------------------------------------------------------------
// change byte-data to double-data 
void WaveLevel(double dLevel[2],  // output of left and right
			   const BYTE* lpWaveData,  // input
			   const WAVFMT& format)
{ 	
	long i;
	short data[2];
	long nData[2];
	double dData[2];
	float fData[2];

	nData[1] = 0;
	
	if(format.bits==8){ // 8 bit
		for(i=0;i<format.channels;i++) nData[i] = (long)lpWaveData[i]-0x80;
	}
	else if(format.bits==16){	// 16 bit
		memcpy(data,lpWaveData,sizeof(short)*format.channels);
		nData[0] = (long)data[0];
		nData[1] = (long)data[1];
	}
	else if(format.bits==24){	// 24 bit
		for(i=0;i<format.channels;i++){
			nData[i] = 0;
			memcpy((LPBYTE)(nData+i)+1,lpWaveData+3*i,3);
			nData[i] /= 256;
		}
	}
	else if(format.bits==32 && format.tag == WAVE_FORMAT_PCM){	// 32 bit long
		memcpy(nData,lpWaveData,sizeof(long)*format.channels);
	}
	else if(format.bits==32 && format.tag == WAVE_FORMAT_IEEE_FLOAT){	// 32 bit float
		memcpy(fData,lpWaveData,sizeof(double)*format.channels);
		dLevel[0] = fData[0];
		dLevel[1] = fData[1];
		return;
	}	
	else if(format.bits==64){	// 64 bit double
		memcpy(dData,lpWaveData,sizeof(double)*format.channels);
		dLevel[0] = dData[0];
		dLevel[1] = dData[1];
		return;
	}

	dLevel[0] = (double)nData[0];
	dLevel[1] = (double)nData[1];
}



const double GetMaxWaveLevel(const WAVFMT& format)
{
	double ret = 0;

	switch(format.bits){

	case 8: 
		ret = 128; 
		break;
	case 16:
		ret = 32768; 
		break;
	case 24: 
		ret = 8388608; 
		break;
	case 64: 
		ret = 1; 
		break;
	case 32: 
		if(format.tag == WAVE_FORMAT_PCM) ret = 2147483648;
		else ret = 1;
		break;
	}

	return ret;
}
