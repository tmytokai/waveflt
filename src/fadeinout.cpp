// fade in/out

#ifdef WIN32
#include <windows.h>
#endif

//---------------------------------
// linear fade in / out 
void FADEINOUT(double* lpFilterBuf, // filter buffer
				   DWORD dwPointsInBuf, // points of data 

				   WAVEFORMATEX waveFmt,
				   DWORD dwFadeIn, // byte, size of fade in
				   DWORD dwFadeOut, // byte, size of fade out

				   LONGLONG n64TotalOutSize, // byte, current total size of output
				   LONGLONG n64TotalSize // byte, total size of input file
				   ){

	DWORD i;
	double dGain;
	LONGLONG n64Pos,n64FadeOutPos;

	if(dwFadeIn == 0 && dwFadeOut == 0) return;

	n64Pos = n64TotalOutSize;
	dGain = 1.0;
	n64FadeOutPos = n64TotalSize - dwFadeOut;
	
	for(i=0;i< dwPointsInBuf;i++){

		if(n64Pos <= dwFadeIn){  // in
			dGain =min(1.0,(double)n64Pos/dwFadeIn);
			lpFilterBuf[i] = dGain * lpFilterBuf[i];
		}
		else if(n64Pos >= n64FadeOutPos){ //out
			dGain = max(0.0, 1.0 - (double)(n64Pos-n64FadeOutPos)/dwFadeOut);
			lpFilterBuf[i] = dGain * lpFilterBuf[i];
		}		
	
		n64Pos+=waveFmt.nBlockAlign;
	}
	
	return;
}

// EOF