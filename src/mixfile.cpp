// file mixing

#include "filter.h"

HANDLE HdMixFile = NULL;  // handle of file
BYTE* LpMixFileBuffer = NULL;  // buffer to read file
WAVEFORMATEX  WaveFmtMixFile;
unsigned long long N64MixFileOffset; // offset of file
double DbMixFileStart; // start time in mixing file


//------------------------------
// open mix file
BOOL OpenMixFile(char* szMixFile,
				 WAVEFORMATEX waveOrgFmt, 
				 DWORD dwBufSize, // points of data in buffer
				 double dMixStartTime, // start time in mixing file
				 char* lpszErr
				 ){
	
	unsigned long long n64FileDataSize;
	char szErr[CHR_BUF];

	DbMixFileStart = dMixStartTime;

	if(!GetWaveFormat(szMixFile,&WaveFmtMixFile,&n64FileDataSize,&N64MixFileOffset,szErr)){
		if(lpszErr) strcpy(lpszErr,szErr);
		return false;
	}

	if(WaveFmtMixFile.nSamplesPerSec != waveOrgFmt.nSamplesPerSec){
		if(lpszErr) wsprintf(lpszErr,"Rate of mixing file must be the same as input.\n");
		return false;
	}

	if(WaveFmtMixFile.nChannels != waveOrgFmt.nChannels){
		if(lpszErr) wsprintf(lpszErr,"Channels of mixing file must be the same as input.\n");
		return false;
	}

	HdMixFile = CreateFile(szMixFile,GENERIC_READ, 0, 0,OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
	if(HdMixFile == INVALID_HANDLE_VALUE){
		if(lpszErr) wsprintf(lpszErr,"Could nod open '%s'\n",szMixFile);
		HdMixFile = NULL;
		return false;
	}	

	LpMixFileBuffer = (BYTE*)malloc(dwBufSize*WaveFmtMixFile.nBlockAlign +1024); 

	return true;
}



//----------------------------------------
// initiailze
void ClearMixFile(){

	LARGE_INTEGER LI;

	if(!HdMixFile) return;

	// move the pointer 
	LI.QuadPart = + (LONGLONG)(DbMixFileStart*WaveFmtMixFile.nSamplesPerSec)*WaveFmtMixFile.nBlockAlign;
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
void MixFile(WAVEFORMATEX waveOrgFmt, 
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

	n64OutPoint = n64OutSize/waveOrgFmt.nBlockAlign;
	n64StartPoint = (LONGLONG)(dMixStartTime*waveOrgFmt.nSamplesPerSec);
	if(n64OutPoint > n64StartPoint) dwOffset = 0;
	else if(n64OutPoint + dwPoints < n64StartPoint) return;  // before start time
	else dwOffset = (DWORD)(n64StartPoint - n64OutPoint);

	// read data from mixing file
	dwReadSize = (dwPoints-dwOffset) * WaveFmtMixFile.nBlockAlign;
	ReadFile(HdMixFile,LpMixFileBuffer,dwReadSize,&dwByte,NULL);
	if(dwByte == 0) return;
	
	dMaxLevel = GetMaxWaveLevel(WaveFmtMixFile);
	lpBuffer2 = LpMixFileBuffer;
	dwPos = dwOffset;
	
	if(waveOrgFmt.nChannels == 2){ // stereo

		for(i=0;i< dwByte;i+=WaveFmtMixFile.nBlockAlign){
			
			lpFilterBuf[0][dwPos] *= dMixLevel[0];
			lpFilterBuf[1][dwPos] *= dMixLevel[0];
			WaveLevel(dLevel,lpBuffer2,WaveFmtMixFile);
			lpFilterBuf[0][dwPos] += dLevel[0]/dMaxLevel * dMixLevel[1];
			lpFilterBuf[1][dwPos] += dLevel[1]/dMaxLevel * dMixLevel[1];
		
			dwPos++;
			lpBuffer2 += WaveFmtMixFile.nBlockAlign;
		}
	}
	else // mono
	{
		for(i=0;i< dwByte;i+=WaveFmtMixFile.nBlockAlign){
			
			lpFilterBuf[0][dwPos] *= dMixLevel[0];
			WaveLevel(dLevel,lpBuffer2,WaveFmtMixFile);
			lpFilterBuf[0][dwPos] += dLevel[0]/dMaxLevel * dMixLevel[1];
			
			dwPos++;
			lpBuffer2 += WaveFmtMixFile.nBlockAlign;
		}
	}
	
}



// EOS
