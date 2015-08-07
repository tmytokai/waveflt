#if 0

// file mixing

#ifdef WIN32
#include <windows.h>
#endif

#include "wave.h"
#include "waveformat.h"

#define CHR_BUF 256 

HANDLE HdMixFile = NULL;  // handle of file
BYTE* LpMixFileBuffer = NULL;  // buffer to read file
WaveFormat  WaveFmtMixFile;
unsigned long long N64MixFileOffset; // offset of file
double DbMixFileStart; // start time in mixing file


//------------------------------
// open mix file
BOOL OpenMixFile(char* szMixFile,
				 WaveFormat waveOrgFmt, 
				 DWORD dwBufSize, // points of data in buffer
				 double dMixStartTime, // start time in mixing file
				 char* lpszErr
				 ){
	
	unsigned long long n64FileDataSize;

	DbMixFileStart = dMixStartTime;

	WaveFmtMixFile.read(szMixFile);
	n64FileDataSize = WaveFmtMixFile.get_datasize();
	N64MixFileOffset = WaveFmtMixFile.get_offset();
	
	if(WaveFmtMixFile.rate() != waveOrgFmt.rate()){
		if(lpszErr) wsprintf(lpszErr,"Rate of mixing file must be the same as input.\n");
		return false;
	}

	if(WaveFmtMixFile.channels() != waveOrgFmt.channels()){
		if(lpszErr) wsprintf(lpszErr,"Channels of mixing file must be the same as input.\n");
		return false;
	}

	HdMixFile = CreateFile(szMixFile,GENERIC_READ, 0, 0,OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if(HdMixFile == INVALID_HANDLE_VALUE){
		if(lpszErr) wsprintf(lpszErr,"Could nod open '%s'\n",szMixFile);
		HdMixFile = NULL;
		return false;
	}	

	LpMixFileBuffer = (BYTE*)malloc(dwBufSize*WaveFmtMixFile.block() +1024); 

	return true;
}



//----------------------------------------
// initiailze
void ClearMixFile(){

	LARGE_INTEGER LI;

	if(!HdMixFile) return;

	// move the pointer 
	LI.QuadPart = + (LONGLONG)(DbMixFileStart*WaveFmtMixFile.rate())*WaveFmtMixFile.block();
	LI.QuadPart += N64MixFileOffset;

	SetFilePointer(HdMixFile,LI.LowPart, &LI.HighPart,FILE_BEGIN);
}



//----------------------------------------
// close
void CloseMixFile(){

	if(HdMixFile) CloseHandle(HdMixFile);
	HdMixFile = NULL;

	if(LpMixFileBuffer) free(LpMixFileBuffer);
	LpMixFileBuffer = NULL;

}



//----------------------------------------
// file mixing
void MixFile(WaveFormat waveOrgFmt, 
			 double* lpFilterBuf[2], // L-R
			 DWORD dwPoints, // points
			 LONGLONG n64OutSize, // total output size of data
			 double dMixLevel[2],  // mixing level
			 double dMixStartTime // mixing start time in original file
			 ){

	DWORD i,dwPos;
	DWORD dwReadSize,dwOffset;
	DWORD dwByte;
	BYTE* lpBuffer2;
	LONGLONG n64StartPoint,n64OutPoint;
	double dLevel[2],dMaxLevel;

	if(!HdMixFile) return;

	n64OutPoint = n64OutSize/waveOrgFmt.block();
	n64StartPoint = (LONGLONG)(dMixStartTime*waveOrgFmt.rate());
	if(n64OutPoint > n64StartPoint) dwOffset = 0;
	else if(n64OutPoint + dwPoints < n64StartPoint) return;  // before start time
	else dwOffset = (DWORD)(n64StartPoint - n64OutPoint);

	// read data from mixing file
	dwReadSize = (dwPoints-dwOffset) * WaveFmtMixFile.block();
	ReadFile(HdMixFile,LpMixFileBuffer,dwReadSize,&dwByte,NULL);
	if(dwByte == 0) return;
	
	dMaxLevel = WaveFmtMixFile.get_max_level();
	lpBuffer2 = LpMixFileBuffer;
	dwPos = dwOffset;
	
	if(waveOrgFmt.channels() == 2){ // stereo

		for(i=0;i< dwByte;i+=WaveFmtMixFile.block()){
			
			lpFilterBuf[0][dwPos] *= dMixLevel[0];
			lpFilterBuf[1][dwPos] *= dMixLevel[0];
			WaveLevel(dLevel,lpBuffer2,WaveFmtMixFile);
			lpFilterBuf[0][dwPos] += dLevel[0]/dMaxLevel * dMixLevel[1];
			lpFilterBuf[1][dwPos] += dLevel[1]/dMaxLevel * dMixLevel[1];
		
			dwPos++;
			lpBuffer2 += WaveFmtMixFile.block();
		}
	}
	else // mono
	{
		for(i=0;i< dwByte;i+=WaveFmtMixFile.block()){
			
			lpFilterBuf[0][dwPos] *= dMixLevel[0];
			WaveLevel(dLevel,lpBuffer2,WaveFmtMixFile);
			lpFilterBuf[0][dwPos] += dLevel[0]/dMaxLevel * dMixLevel[1];
			
			dwPos++;
			lpBuffer2 += WaveFmtMixFile.block();
		}
	}
	
}



// EOS

#endif