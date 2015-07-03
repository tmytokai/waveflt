// play sound

#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

#include "waveformat.h"


// status
#define ID_PREPARE 0
#define ID_OPEN 1
#define ID_CLOSEWAVE 3

#define WAVEHDR_BUFNUM 2  // number of wavehdr

HWAVEOUT HWaveOut = NULL;  // handle of device
WAVEHDR WaveHdr[WAVEHDR_BUFNUM]; // buffer of wavehdr
WORD WCurWaveHdr = 0;  
WORD PlayStatus; // status
HANDLE HSemaphore; // handle of semaphore (for Ctrl+c)


//-------------------------------------------------------------------
// create buffer of wavehdr
BOOL SetWaveHdr(LPWAVEHDR lpWaveHdr, LONG nHdrNumber,DWORD dwWaveBlockByte)
{
	LONG i;
	LPBYTE lpWaveData;
	
	lpWaveData =(LPBYTE)malloc(sizeof(BYTE)*dwWaveBlockByte*nHdrNumber);
	
	if(lpWaveData == NULL){
		fprintf(stderr, "Could not allocate memory.");
		return false;
	}
	
	// setup wavehdr
	for(i=0;i<nHdrNumber;i++){
		memset(&lpWaveHdr[i], 0, sizeof(WAVEHDR));
		lpWaveHdr[i].lpData  = (LPSTR)lpWaveData+dwWaveBlockByte*i;	
		lpWaveHdr[i].dwBufferLength = dwWaveBlockByte; // buffer size
	}
	
	return true;
}



//-------------------------------------------------------------------
// free buffer
BOOL DelWaveHdr(LPWAVEHDR lpWaveHdr)
{
	
	if(lpWaveHdr[0].lpData){
		free(lpWaveHdr[0].lpData);
		lpWaveHdr[0].lpData=NULL;
	}
	
	return true;
}



//-------------------------------------------------------------------
// call back function
VOID CALLBACK MyWaveOutProc(HWAVEOUT hWaveOut,UINT msg,DWORD inst,DWORD dwP1,DWORD dwP2)
{
	LPWAVEHDR lpWaveHdr; 
	
	switch (msg) {
		
	case WOM_OPEN: 
		
		PlayStatus = ID_OPEN;
		
		break;
		

	case WOM_DONE:
		
		if(PlayStatus == ID_OPEN){
			
			lpWaveHdr = (LPWAVEHDR)dwP1;

			// now, this buffer can be used
			lpWaveHdr->dwUser = 0;
		}
		
		break;
		
	case WOM_CLOSE: 
		
		PlayStatus = ID_CLOSEWAVE;
		
		break;
		
	default:
        break;
    }
}



//-------------------------------------------------------------------
// open device
BOOL OpenWaveDevice(UINT uDeviceID,WAVFMT waveFmt,DWORD dwWaveBlockByte)
{
	int i;
	
	if(HWaveOut != NULL) return false;
	
	if(uDeviceID == -1) uDeviceID = WAVE_MAPPER;

	if(!SetWaveHdr(WaveHdr,WAVEHDR_BUFNUM,dwWaveBlockByte)) return false;
	
	WAVEFORMATEX waveFmtEx;
	memcpy(&waveFmtEx,&waveFmt,sizeof(WAVFMT));
	waveFmtEx.cbSize = 0;
	PlayStatus = ID_PREPARE;
	if(waveOutOpen(&HWaveOut, uDeviceID,
		&waveFmtEx, (DWORD)MyWaveOutProc, 0, CALLBACK_FUNCTION ) != MMSYSERR_NOERROR ){
		
		fprintf(stderr, "Could not open wave device(ID = %d)\n",uDeviceID);
		
		DelWaveHdr(WaveHdr);
		return false;
	} 
	
	// wait for open
	i=0;
	while(PlayStatus != ID_OPEN && i<50){
		Sleep(50);
		i++; 
	}

	fprintf(stderr, "\nopen wave device: device ID = %d\n",uDeviceID);
	
	for(i=0;i<WAVEHDR_BUFNUM;i++)
		waveOutPrepareHeader(HWaveOut,&WaveHdr[i],sizeof(WAVEHDR));

	// create semaphore
	HSemaphore = CreateSemaphore(NULL,1,1,NULL);

	return true;
}



//-------------------------------------------------------------------
// play
BOOL PlayWave(BYTE* lpBuffer,DWORD dwWriteByte){

	if(WaitForSingleObject(HSemaphore,50) == WAIT_TIMEOUT) return false;

	if(HWaveOut == NULL || PlayStatus != ID_OPEN) return false;
	
	// wait for ready
	while(WaveHdr[WCurWaveHdr].dwUser) Sleep(50);

	// copy the data to buffer
	CopyMemory(WaveHdr[WCurWaveHdr].lpData,lpBuffer,dwWriteByte);

	// send the data to device
	WaveHdr[WCurWaveHdr].dwBufferLength = dwWriteByte;
	if(dwWriteByte!=0) waveOutWrite(HWaveOut,&WaveHdr[WCurWaveHdr],sizeof(WAVEHDR));
	WaveHdr[WCurWaveHdr].dwUser = 1;

	// move to next buffer
	WCurWaveHdr = (WCurWaveHdr+1)&(WAVEHDR_BUFNUM-1);

	ReleaseSemaphore(HSemaphore,1,NULL);

	return true;
}

	
//-------------------------------------------------------------------
// close device
BOOL CloseWaveDevice(BOOL bForceStop){

	int i;
	
	WaitForSingleObject(HSemaphore,INFINITE);

	if(HWaveOut == NULL) return FALSE;

	// wait for stop
	WCurWaveHdr = (WCurWaveHdr-1)&(WAVEHDR_BUFNUM-1);
	if(!bForceStop) while(WaveHdr[WCurWaveHdr].dwUser) Sleep(50);

	waveOutReset(HWaveOut);

	// unprepare wavehdr
	for(i=0;i<WAVEHDR_BUFNUM;i++){
		waveOutUnprepareHeader(HWaveOut,&WaveHdr[i],sizeof(WAVEHDR));
		WaveHdr[WCurWaveHdr].dwUser = 1;
	}

	// close device
	if(waveOutClose(HWaveOut)!=MMSYSERR_NOERROR)
		fprintf(stderr, "\n\nCould not close wave device.");
	else{
		i=0;
		while(PlayStatus != ID_CLOSEWAVE && i<50){
			Sleep(50);
			i++;
		}
		fprintf(stderr, "close wave device\n");
		HWaveOut = NULL;
	}
	
	DelWaveHdr(WaveHdr);

	// delete semaphore
	CloseHandle(HSemaphore);

	return true;
}

//EOF