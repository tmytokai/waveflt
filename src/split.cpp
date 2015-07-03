// split file at no sound or at specified size

#ifdef WIN32
#include <windows.h>
#endif
#include <math.h>

#include "waveformat.h"

// mode of nosound
#define NOSND_NOT			0	// no split
#define NOSND_EXEC			1	// searching no sound part
#define NOSND_SEARCH		2	// searching sound part after splitting, then move to NOSND_EXEC
#define NOSND_CUT			3	// deleting no sound part after splitting, then move to NOSND_EXEC
#define NOSND_HEADCUTONLY	4	// deleting no sound part,then move to NOSND_NOT 
#define NOSND_STOP			5	// stop recording of lockon if no sound part is continuing

// type of normalizer
#define NORMAL_NOT			0
#define NORMAL_EXEC			1  // execute normalizer
#define NORMAL_PEAKBYDB		2  // searching peak
#define NORMAL_AVG			3  // searching average
#define NORMAL_RMS			4  // searching RMS

#define MAX_SPLIT	128 // max of split number

DWORD NOSND_dwMode; // mode
DWORD NOSND_dwInitMode; // initial mode
DWORD NOSND_dwMaxSuspendPoint; // max of suspend points
DWORD NOSND_dwPoint = 0; // current points of no sound 
DWORD NOSND_dwSuspend = 0; // current points of suspension


//------------------------------------------
// clear
VOID ClearNOSOUND(){

	NOSND_dwMode = NOSND_dwInitMode; 
	NOSND_dwPoint = 0; 
	
	if(NOSND_dwMode == NOSND_EXEC) NOSND_dwSuspend = NOSND_dwMaxSuspendPoint; 
	else NOSND_dwSuspend = 0;
}



//---------------------------------------------------
// initialize
void PrepareNOSOUND(DWORD dwNoSndMode, DWORD dwNoSndSuspend, long nSamplesPerSec 
){
	
	NOSND_dwInitMode = dwNoSndMode;
	NOSND_dwMaxSuspendPoint = (DWORD)((double)dwNoSndSuspend*nSamplesPerSec/1000.);

	ClearNOSOUND();
}



//------------------------------------------
// split at no sound part
VOID NOSOUND(double* lpFilterBuf[2], // buffer
			  DWORD* lpdwPointsInBuf, // points of data in buffer
			   BOOL* lpbChangeFile, // if *lpbChangeFile = true, change output file
			   /* obsolete
			   HWND hWnd, // HWND of LockOn
			   */
			   long nSamplesPerSec,
			   DWORD dwChn,

			   DWORD dwNoSndTime,
			   DWORD dwNoSndM1toM2, 
			   DWORD dwNoSndRecStop, 

			   double dNoSndBoundHead, // level of sound (dB)
			   double dNoSndBound // level of no sound (dB)
			   ){
	
	DWORD i,dwPos,dwCurPos,dwFoo;
	DWORD dwNoSndPoint,dwNoSndM1toM2Point,dwNoSndRecStopPoint;
	
	dwPos = 0;
	dwCurPos = 0;

	dNoSndBoundHead = pow(10,dNoSndBoundHead/20);
	dNoSndBound = pow(10,dNoSndBound/20);

	dwNoSndPoint = (DWORD)((double)dwNoSndTime * nSamplesPerSec/1000.);
	dwNoSndM1toM2Point = (DWORD)((double)dwNoSndM1toM2*nSamplesPerSec/1000.);
	dwNoSndRecStopPoint = (DWORD)((double)dwNoSndRecStop*nSamplesPerSec/1000.);
	
	// in the no sound part, searching sound part
	if(NOSND_dwMode == NOSND_SEARCH ||
		NOSND_dwMode == NOSND_CUT || 
		NOSND_dwMode == NOSND_HEADCUTONLY ||
		NOSND_dwMode == NOSND_STOP){
		
		if(dwChn == 2){ // stereo
			for(dwPos=0;dwPos<*lpdwPointsInBuf;dwPos++){
				if(fabs(lpFilterBuf[0][dwPos]) >= dNoSndBoundHead ||
					fabs(lpFilterBuf[1][dwPos]) >= dNoSndBoundHead){ // detect sound part
					NOSND_dwPoint = 0;
					break;
				}
				else NOSND_dwPoint++;
			}
			dwCurPos = dwPos;
		}
		else if(dwChn == 1){ // mono
			for(dwPos=0;dwPos<*lpdwPointsInBuf;dwPos++){
				if(fabs(lpFilterBuf[0][dwPos]) >= dNoSndBoundHead){ // detect sound part
					NOSND_dwPoint = 0;
					break;
				}
				else NOSND_dwPoint++;
			}
			dwCurPos = dwPos;
		}
				
		switch(NOSND_dwMode){
			
		case NOSND_CUT:
		case NOSND_HEADCUTONLY:
			
			// when whole buffer is no sound, set points of buffer to 0.
			if(NOSND_dwPoint) *lpdwPointsInBuf = 0;
			else
			{ 
				// sound part is detected,
				
				// cut no sound part
				dwFoo = *lpdwPointsInBuf-dwPos;
				for(i=0;i<dwChn;i++)
					memmove(lpFilterBuf[i],lpFilterBuf[i]+dwPos,sizeof(double)*dwFoo);
				*lpdwPointsInBuf = dwFoo;
				
				// change mode
				if(NOSND_dwMode == NOSND_CUT){
					
					// change mode and then suspend.
					NOSND_dwMode = NOSND_EXEC; 
					NOSND_dwPoint = 0;
					NOSND_dwSuspend = NOSND_dwMaxSuspendPoint; 
				}
				else if(NOSND_dwMode == NOSND_HEADCUTONLY) NOSND_dwMode = NOSND_NOT;  
			}
			
			break;
			
		case NOSND_SEARCH:
			
			// if sound part is detected, change mode and then suspend.
			if(!NOSND_dwPoint){
				
				NOSND_dwMode = NOSND_EXEC; 
				NOSND_dwPoint = 0;
				NOSND_dwSuspend = NOSND_dwMaxSuspendPoint; 
			}
			
			// if points of the no sound part execeed dwNoSndM1toM2Point, 
			// change mode to NOSND_CUT.
			if(NOSND_dwPoint >= dwNoSndM1toM2Point){
				
				// cut excess part
				*lpdwPointsInBuf -= (NOSND_dwPoint-dwNoSndM1toM2Point);
				NOSND_dwMode = NOSND_CUT;
			}
			
			break;
			
		case NOSND_STOP:
			break;
		}
	}

	// suspend
	if(NOSND_dwSuspend){

		if(NOSND_dwSuspend > *lpdwPointsInBuf){
			NOSND_dwSuspend -= *lpdwPointsInBuf;
		}
		else{ // finish suspend 
			dwCurPos = NOSND_dwSuspend;
			NOSND_dwSuspend = 0;
		}
	}

	if(NOSND_dwSuspend == 0)
	{
		// in the sound part, searching no sound part
		if(NOSND_dwMode == NOSND_EXEC)
		{

			if(dwChn == 2){ // stereo
				for(dwPos=dwCurPos; dwPos < *lpdwPointsInBuf 
					&& NOSND_dwPoint < dwNoSndPoint ;dwPos++){
					if(fabs(lpFilterBuf[0][dwPos]) < dNoSndBound &&
						fabs(lpFilterBuf[1][dwPos]) < dNoSndBound) NOSND_dwPoint++;
					else NOSND_dwPoint = 0;  // detect sound part
				}
			}
			else if(dwChn == 1){ // mono
				for(dwPos=dwCurPos; dwPos < *lpdwPointsInBuf 
					&& NOSND_dwPoint < dwNoSndPoint ;dwPos++){
					if(fabs(lpFilterBuf[0][dwPos]) < dNoSndBound) NOSND_dwPoint++;
					else NOSND_dwPoint = 0;  // detect sound part
				}
			}

			// if no sound part is detected, 
			if(NOSND_dwPoint >= dwNoSndPoint){

				*lpdwPointsInBuf = dwPos; 
				*lpbChangeFile = true;  // change file

				// change mode
				NOSND_dwMode = NOSND_dwInitMode; 
				NOSND_dwPoint = 0;
				if(NOSND_dwMode == NOSND_EXEC) NOSND_dwSuspend = NOSND_dwMaxSuspendPoint; 
				else NOSND_dwSuspend = 0;
				
			}
		}
	}
	
}




//-----------------------------------------------
// split file at specified size
VOID SPLIT(WAVFMT waveFmt, 
		   DWORD* lpdwPointsInBuf, // points in buffer
		   LONGLONG n64TotalOutSize, // total output size
		   DWORD DwCurSplitNo,
		   BOOL* bChangeFile, // if bChangeFile = true, then file is changed
		   LONGLONG* lpn64SplitByte, // if output size exceeds this size, then change file

		   double* lpdSplitTime, // for -split2,3
		   LONGLONG* lpn64SplitByteMalti // for -splitbm
		   ){
	
	LONGLONG n64FileSize;
	DWORD dwFoo;
	
	// get current output size
	n64FileSize = n64TotalOutSize +(*lpdwPointsInBuf)*waveFmt.block; 

	if(n64FileSize >= *lpn64SplitByte){  

		// decrease points of buffer
		*lpdwPointsInBuf = (DWORD)((*lpn64SplitByte-n64TotalOutSize)/waveFmt.block);
		*bChangeFile = true;
		
		dwFoo = DwCurSplitNo+1;
		
		// set next split size to *lpn64SplitByte
		if(dwFoo < MAX_SPLIT){
			
			// -split2,3
			if(lpdSplitTime[dwFoo])
				*lpn64SplitByte = (LONGLONG)((double)waveFmt.avgbyte*lpdSplitTime[dwFoo]);
			
			// -splitbm
			if(lpn64SplitByteMalti[dwFoo]) *lpn64SplitByte = lpn64SplitByteMalti[dwFoo];
		}
		
	}
}
  


// EOF