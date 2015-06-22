// common functions for filter


#include "filter.h"


//---------------------------------------------
// get version of waveflt
DWORD GetVer(char* szVer){

	strcpy(szVer,VER_STR);

	return VER_NUM;
}


//------------------------------------
// DFT
void Dft(LPCOMPLEX x,// input 
		 LPCOMPLEX X,// output
		 DWORD N 
		 ){
	
	DWORD n;
	double w;
	DWORD k;

	memset(X,0,sizeof(COMPLEX)*N);

	w = (2*PI/N);  
	
	// W_N^(kn) = e^{-i*w*k*n} = cos[w*k*n] - i*sin[w*k*n] = cos - i*sin
	//
	// X += x[n]*W_N^k
	// = (R + i*Im)*(cos - i*sin)
	// = (R*cos+Im*sin)+i*(Im*cos-R*sin)
	for(k=0;k<N;k++){
		for(n = 0 ; n <= N-1 ; n++){
			X[k].r += (x[n].r*cos(w*k*n)+x[n].i*sin(w*k*n));
			X[k].i += (x[n].i*cos(w*k*n)-x[n].r*sin(w*k*n));
		}
	}

}



//-------------------------------------------------------------------
// IDFT
void Idft(LPCOMPLEX data, // input
		  LPCOMPLEX out, // output
		  DWORD N  
		  )
{
	DWORD k; 
	double foo;

	Dft(data,out,N);

	out[0].r /= N;
	out[0].i /= N;
	for(k = 1 ; k <= N/2 ; k++){

		// R part
		foo = out[k].r;
		out[k].r = out[N - k].r/N;
		out[N - k].r = foo/N;

		// omit I part
	}
}



//------------------------------------
// output the spector via DFT
double CalcSpec(LPCOMPLEX x,// input
				DWORD N, // number of data
				double freq,  // frequency
				double samp_freq // sampling frequency
				){
	DWORD n;
	COMPLEX X;
	double w;
	
	w = 2*PI*freq/samp_freq;  
	
	X.r = 0;
	X.i = 0;
	
	// e^{-i*w*n} = cos[w*n] - i*sin[w*n] = cos - i*sin
	//
	// X += x[n]*e^{-i*w*n}
	// = (R + i*Im)*(cos - i*sin)
	// = (R*cos+Im*sin)+i*(Im*cos-R*sin)
	for(n = 0 ; n <= N-1 ; n++){
		X.r += (x[n].r*cos(w*n)+x[n].i*sin(w*n));
		X.i += (x[n].i*cos(w*n)-x[n].r*sin(w*n));
	}
	
	return(sqrt(X.r*X.r+X.i*X.i));
}




//----------------------------------
// Modified Bessel Function I0
#define MAXSUM 20 
double I0(double x){
	
	int i;
	double k,foo,out;
	
	out = 0;
	k = 1;
	for(i=1;i<MAXSUM;i++){
		k *= i;
		foo = pow(x/2,i)/k;
		out += foo*foo;
	}
	
	return(out+1);
}
#undef MAXSUM


//----------------------------------
// alpha of kaiser window
double kaiserAlpha(double dDB){

	double alpha;

	if(dDB < -50) alpha = 0.1102*(-dDB-8.7);
	else if(dDB >= -21) alpha = 0;
	else alpha = 0.5842 * pow((double)(-dDB-21),0.4) + 0.07880*(-dDB-21);

	return alpha;
}



//----------------------------------
// KBD (Kaiser Bessel Derived) window
void KBDWin(double* win, DWORD dwSize,double alpha){

	double foo = 0;
	DWORD i;
	
	for (i=0; i<=dwSize/2; i++){
      foo += I0(PI*alpha* sqrt(1. - (4.*i/dwSize - 1.)*(4.*i/dwSize - 1.)));
      if(i<dwSize/2) win[i] = foo;
   }

   for (i=0; i<dwSize/2; i++) {
      win[i] = sqrt(win[i]/foo); // normalize
      win[dwSize-1-i] = win[i]; // copy to right part
   }

}




//-------------------------------------
// synthesize sine waves
void AddSinCurve(double* lpFilterBuf,
				 DWORD dwPointsInBuf,

				 DWORD dwRate,
				 DWORD dwChn,
				 DWORD dwNum, //number of waves  (if 0,  then initialize)
				 LPDWORD dwFreq,double* lpdbDb,LPDWORD dwPhase){

	static DWORD dwTime[2] = {0,0};
	DWORD i,num;
	DWORD dwCurTime;
	double dbLevel,w,dbPhase;
	
	if(dwNum == 0) {
		dwTime[0] = 0;
		dwTime[1] = 0;
		return;
	}

	for(num = 0; num < dwNum; num++){
		dwCurTime = dwTime[dwChn];
		dbLevel = pow(10.,lpdbDb[num]/20);
		dbPhase = dwPhase[num]*PI/180.;
		w = 2.*PI/dwRate*dwFreq[num];
		for(i=0;i<dwPointsInBuf;i++){
			lpFilterBuf[i] += dbLevel *sin(w*(dwCurTime++)+dbPhase);
		}
	}
	dwTime[dwChn] = dwTime[dwChn]+dwPointsInBuf;
}



/* obsolete
#ifdef USEWIN32API

//-------------------------------------
// thread to stop 'lockon'
void WINAPI StopRecordingThread(LPVOID lpVoid){

	HWND hWnd = (HWND)lpVoid;
	SendMessage(hWnd,WM_COMMAND,1003,0); 
}



//------------------------------------
// stop lockon
// create thread to avoid dead-lock
void StopRecording(HWND hWnd){
	
	DWORD dwID;

	if(hWnd == NULL) return;

	fprintf(stderr,"\nstopping lockon...\n");

	CreateThread(NULL,0,
		(LPTHREAD_START_ROUTINE)StopRecordingThread,
		(LPVOID)hWnd,
		0,(LPDWORD)&dwID);
	
}

#endif
*/



// EOF