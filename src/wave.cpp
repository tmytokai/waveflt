// common wave function

#include "filter.h"


//-------------------------------------------------------------------
// set WAVE format
void SetWaveFmt(LPWAVEFORMATEX lpWaveFmt,WORD waveChn,
				DWORD waveRate,	WORD waveBit, WORD wTag)
{
	
	lpWaveFmt->wFormatTag = wTag;
	lpWaveFmt->nChannels = waveChn;  
	lpWaveFmt->nSamplesPerSec = waveRate; 
	lpWaveFmt->nAvgBytesPerSec = (DWORD)(waveRate*waveChn*waveBit/8); // byte/sec
	lpWaveFmt->nBlockAlign = (WORD)(waveChn*waveBit/8); // byte/block
	lpWaveFmt->wBitsPerSample = waveBit; 
	lpWaveFmt->cbSize = (WORD)0; // no extention
}



//-------------------------------------------------------------------
// write out wave header to file
#ifdef USEWIN32API  // use Windows APIs

VOID WriteWaveHeader(HANDLE hdWriteFile,LPWAVEFORMATEX lpWaveFmt,LONGLONG n64WaveDataSize
					 ,BOOL bUseExtChunk)
{
	
	DWORD dwFoo;
	DWORD dwByte; 
	LARGE_INTEGER LI; 
	DWORD dwFileType;
	DWORD dwHeadSize;

	dwHeadSize = WAVEHDRSIZE(bUseExtChunk);

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
	WriteFile(hdWriteFile, lpWaveFmt, sizeof(WAVEFORMATEX)-sizeof(WORD), &dwByte, NULL);
	
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

#else  // without win32 APIs

void EXPORT WriteWaveHeader(FILE* hdWriteFile,LPWAVEFORMATEX lpWaveFmt,LONGLONG n64WaveDataSize)
{
	
	DWORD dwFoo;
	DWORD dwHeadSize;

	dwHeadSize = WAVEHDRSIZE(BlExtChunkOfHdr);

	if(hdWriteFile == NULL) return;
	
	if(hdWriteFile != stdout) fseek(hdWriteFile,0,SEEK_SET);
	
	// RIFF (20 byte)
	fwrite("RIFF",1,4,hdWriteFile);
	if(n64WaveDataSize >= 0xFFFFFFFF-dwHeadSize+8) dwFoo = 0xFFFFFFFF; // = 4G
	else dwFoo = ((DWORD)n64WaveDataSize + dwHeadSize) - 8;
	fwrite(&dwFoo,1,sizeof(DWORD),hdWriteFile);
	fwrite("WAVE",1,4,hdWriteFile);

	// format chunk (16 byte)
	fwrite("fmt ",1,4,hdWriteFile);
	dwFoo = 16;
	fwrite(&dwFoo,1,sizeof(DWORD),hdWriteFile);
	fwrite(lpWaveFmt,1,16,hdWriteFile);

	// waveflt chunk (extra chunk)
	fwrite("wflt",1,4,hdWriteFile);
	dwFoo = sizeof(LONGLONG);
	fwrite(&dwFoo,1,sizeof(DWORD),hdWriteFile);
	fwrite(&n64WaveDataSize,1,sizeof(LONGLONG),hdWriteFile);
	

	// data chunk (8 byte)
	fwrite("data",1,4,hdWriteFile);
	if(n64WaveDataSize >= 0xFFFFFFFF-dwHeadSize+8) dwFoo = 0xFFFFFFFF-dwHeadSize+8; // = 4G 
	else dwFoo = (DWORD)n64WaveDataSize;
	fwrite(&dwFoo,1,sizeof(DWORD),hdWriteFile);
}

#endif




//-------------------------------------
// search for head of chunk, 
BOOL SearchChunk(HANDLE hdFile,char* szChunk, DWORD dwSize, 
				 BOOL bGetChkSize, // get chunk size
				 BOOL bStdin, // no seek
				 LPDWORD lpdwChkSize,LPDWORD lpdwOffset){

	DWORD dwPoint;
	DWORD dwByte;
	BYTE byteRead;
	char szCheck[5];
#ifdef USEWIN32API
	LARGE_INTEGER LI; 
#endif

	memset(szCheck,0,5);
	for(dwPoint = 0; dwPoint < dwSize; dwPoint++){
		
		if(!bStdin){
#ifdef USEWIN32API 
			LI.QuadPart = dwPoint;
			SetFilePointer(hdFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
#else
			fseek(hdFile,dwPoint,SEEK_SET);
#endif
		}
		
#ifdef USEWIN32API 
		ReadFile(hdFile,&byteRead,1,&dwByte, NULL);
#else  
		dwByte = fread(&byteRead,1,1,hdFile);
#endif
		if(dwByte != 1) return false;
		*lpdwOffset += dwByte;  

		if(byteRead == szChunk[0]){
			
#ifdef USEWIN32API 
			ReadFile(hdFile,szCheck,3,&dwByte, NULL);
#else  
			dwByte = fread(szCheck,1,3,hdFile);
#endif
			if(dwByte != 3) return false;
			*lpdwOffset += dwByte;  
			
			if(strcmp(szCheck,szChunk+1)==0) break;
		}

	}
	
	if(dwPoint == dwSize) return false;

	// chunk size
	if(bGetChkSize){
#ifdef USEWIN32API 
		ReadFile(hdFile,lpdwChkSize,sizeof(DWORD),&dwByte, NULL);
#else  
		dwByte = fread(lpdwChkSize,1,sizeof(DWORD),hdFile);
#endif
		if(dwByte != sizeof(DWORD)) return false;
		*lpdwOffset += dwByte;  
	}

	if(!bStdin){
		*lpdwOffset = dwPoint + 4;
		if(bGetChkSize) *lpdwOffset += sizeof(DWORD);
	}


	return true;
}



//------------------------------------------------------------------
// check whether format is valid
BOOL CheckWaveFormat(LPWAVEFORMATEX lpWaveFmt,char* lpszErr){

	DWORD waveRate,avgByte;	
	WORD waveChn,waveBit;

	if(lpWaveFmt->wFormatTag != WAVE_FORMAT_IEEE_FLOAT 
		|| lpWaveFmt->wFormatTag != WAVE_FORMAT_PCM // 
		){
		strcpy(lpszErr,"This data is not PCM format.\n");
	}
	
	waveChn = lpWaveFmt->nChannels;  
	waveRate = lpWaveFmt->nSamplesPerSec; 
	waveBit = lpWaveFmt->wBitsPerSample; 
	avgByte = lpWaveFmt->nAvgBytesPerSec; 

	if(waveChn != 1 && waveChn != 2){
		if(lpszErr) wsprintf(lpszErr,"'%d channel' is not supported.\n",waveChn);
		return false;
	}	

	if(waveBit != 8 && waveBit != 16 && waveBit != 24 && waveBit != 32 && waveBit != 64){
		if(lpszErr) wsprintf(lpszErr,"'%d bit' is not supported.\n",waveBit);
		return false;
	}
	
	if(avgByte != waveChn*waveRate*waveBit/8){
		if(lpszErr) strcpy(lpszErr,"Wave format is broken.\n");
		return false;
	}

	return true;
}


//-------------------------------------------------------------------
// get WAVE format and time, etc.   5/18/2002
BOOL GetWaveFormat(char* lpszFileName, // file name or 'stdin'
				   LPWAVEFORMATEX lpWaveFmt, 
				   LONGLONG* lpn64WaveDataSize, // size of data
				   LONGLONG* lpn64WaveOffset, // offset to data chunk
				   char* lpszErr 
				   )
{
	
#ifdef USEWIN32API
	HANDLE hdFile;
#else
	FILE* hdFile;
#endif
	DWORD dwCkSize; // chunk size
	DWORD dwByte;
	DWORD dwOffset; // offset to data
		
	char szErr[CHR_BUF];
	BOOL bStdin = false;
	LONGLONG n64DataSize;

	n64DataSize = 0;
	
	// open file
	if(strcmp(lpszFileName,"stdin")==0) bStdin = true;
	
	if(bStdin){ // stdin
		
#ifdef USEWIN32API
		hdFile	= GetStdHandle(STD_INPUT_HANDLE); 
#else
		hdFile = stdin;
#ifdef WIN32
		_setmode(_fileno(stdin),_O_BINARY); // binary mode
#endif
		
#endif
	}
	else{ // file
		
#ifdef USEWIN32API
		hdFile = CreateFile(lpszFileName,GENERIC_READ, 0, 0,OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if(hdFile == INVALID_HANDLE_VALUE){
			 wsprintf(szErr,"Could not open '%s'\n",lpszFileName);
			goto L_ERR;
		}						
#else
		hdFile = fopen(lpszFileName,"rb");
		if(hdFile == NULL) {
			wsprintf(szErr,"Could not open '%s'\n",lpszFileName);
			goto L_ERR;
		}	
#endif		
	}
	

	//------------------------------------------------------

	dwOffset = 0;
	
	// is this file 'RIFF' format?
	if(!SearchChunk(hdFile,"RIFF",4,true,bStdin,&dwCkSize,&dwOffset)){
		strcpy(szErr,"Could not find 'RIFF'. This is not wave file.\n");
		goto L_ERR;	
	}


	//--------------------------
	
	// search for 'WAVE'
	if(!SearchChunk(hdFile,"WAVE",128,false,bStdin,&dwCkSize,&dwOffset)){
		strcpy(szErr,"Wave header may be broken.\n");
		goto L_ERR;	
	}
	
	//--------------------------
	// format chunk

	// search for 'fmt_'
	if(!SearchChunk(hdFile,"fmt ",128,true,bStdin,&dwCkSize,&dwOffset)){
		strcpy(szErr,"Wave header may be broken.\n");
		goto L_ERR;	
	}

	// get format
#ifdef USEWIN32API 
	ReadFile(hdFile,lpWaveFmt,dwCkSize,&dwByte, NULL);
#else  
	dwByte = fread(lpWaveFmt,1,dwCkSize,hdFile);
#endif
	if(dwByte != dwCkSize){
		strcpy(szErr,"Wave header may be broken.\n");
		goto L_ERR;
	}
	dwOffset += dwByte;  
	
	// set cbSize to  0 
	lpWaveFmt->cbSize = 0; 
	
	// check format tag
	if(!CheckWaveFormat(lpWaveFmt,szErr)) goto L_ERR;

	//--------------------------
	// waveflt chunk (extra chunk)
	
	if(!bStdin){ // when stdin, endless mode is set, so ignore data size

		// search for 'wflt'
		if(SearchChunk(hdFile,"wflt",128,true,bStdin,&dwCkSize,&dwOffset)){

			// get file size
#ifdef USEWIN32API 
			ReadFile(hdFile,&n64DataSize,sizeof(LONGLONG),&dwByte, NULL);
#else  
			dwByte = fread(&n64DataSize,1,sizeof(LONGLONG),hdFile);
#endif
			if(dwByte != sizeof(LONGLONG)){
				strcpy(szErr,"Wave header may be broken.\n");
				goto L_ERR;
			}
			dwOffset += dwByte;  
			
			*lpn64WaveDataSize = n64DataSize;
		}
	}


	//--------------------------
	// data chunk

	// search for 'data'
	if(!SearchChunk(hdFile,"data",128,true,bStdin,&dwCkSize,&dwOffset)){
		strcpy(szErr,"Wave header may be broken.\n");
		goto L_ERR;	
	}
	
	if(lpn64WaveDataSize && n64DataSize == 0) *lpn64WaveDataSize = dwCkSize;
	
	// offset to data
	if(lpn64WaveOffset) *lpn64WaveOffset = dwOffset;

	// close handle
	if(!bStdin){
#ifdef USEWIN32API 
		CloseHandle(hdFile);
#else
		fclose(hdFile);
#endif
	}

	return true;
	
	//------------------------

L_ERR: 
	
	if(!bStdin){
#ifdef USEWIN32API 
		CloseHandle(hdFile);
#else
		fclose(hdFile);
#endif
	}
	if(lpszErr) strcpy(lpszErr,szErr);
	
	return false;
	
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
			   BYTE* lpWaveData,  // input
			   WAVEFORMATEX waveFmt){ 
	
	long i;
	short data[2];
	long nData[2];
	double dData[2];
	float fData[2];

	nData[1] = 0;
	
	if(waveFmt.wBitsPerSample==8){ // 8 bit
		for(i=0;i<waveFmt.nChannels;i++) nData[i] = (long)lpWaveData[i]-0x80;
	}
	else if(waveFmt.wBitsPerSample==16){	// 16 bit
		memcpy(data,lpWaveData,sizeof(short)*waveFmt.nChannels);
		nData[0] = (long)data[0];
		nData[1] = (long)data[1];
	}
	else if(waveFmt.wBitsPerSample==24){	// 24 bit
		for(i=0;i<waveFmt.nChannels;i++){
			nData[i] = 0;
			memcpy((LPBYTE)(nData+i)+1,lpWaveData+3*i,3);
			nData[i] /= 256;
		}
	}
	else if(waveFmt.wBitsPerSample==32 && waveFmt.wFormatTag == WAVE_FORMAT_PCM){	// 32 bit long
		memcpy(nData,lpWaveData,sizeof(long)*waveFmt.nChannels);
	}
	else if(waveFmt.wBitsPerSample==32 && waveFmt.wFormatTag == WAVE_FORMAT_IEEE_FLOAT){	// 32 bit float
		memcpy(fData,lpWaveData,sizeof(double)*waveFmt.nChannels);
		dLevel[0] = fData[0];
		dLevel[1] = fData[1];
		return;
	}	
	else if(waveFmt.wBitsPerSample==64){	// 64 bit double
		memcpy(dData,lpWaveData,sizeof(double)*waveFmt.nChannels);
		dLevel[0] = dData[0];
		dLevel[1] = dData[1];
		return;
	}

	dLevel[0] = (double)nData[0];
	dLevel[1] = (double)nData[1];
}



//-----------------------------------------------
// get max level of wave 
double GetMaxWaveLevel(WAVEFORMATEX waveFmt){

	double dRet;

	switch(waveFmt.wBitsPerSample){
	case 8: dRet = 127; break;
	case 16: dRet = 32767; break;
	case 24: dRet = 8388607; break;
	case 64: dRet = 1; break;
	case 32: 
		if(waveFmt.wFormatTag == WAVE_FORMAT_PCM) dRet = 2147483647;
		else dRet = 1;
		break;
	}

	return dRet;
}



//EOF