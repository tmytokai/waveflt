// re-sampling

#ifdef WIN32
#include <windows.h>
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <assert.h>
#include <stdio.h>

//--------------------------------------------------
// fir.c
void CalcFirCoefficient(DWORD dwFilter,  // type of filter
						double * coeff, 

						DWORD dwLength,
						double dbSmpRate,  // sampling rate
						double dDB,  // loss
						DWORD dwCutLow,  // cut-off freq (low)
						DWORD dwCutHigh  // cut-off freq (high)
						);


//----------------------------
// conv.c
double CONV();
void InitCONV(DWORD dwLength);
void UnprepareCONV();
void SetConvSize(DWORD n);
double* GetConvBufA();
double* GetConvBufB();
BOOL  CheckSSE2(); // you need to call this function to use SSE2

// type of filter
#define NO_FILTER	0
#define LPF 1
#define HPF 2
#define BPF 3
#define BSF 4
#define SVEQL 5  // shelving EQ low
#define SVEQH 6  // shelving EQ high


double **RSM_dFir;  // RSM_dFir[phase][index], polyphase FIR filter coefficients
DWORD RSM_dwFirLn; // length of filter
DWORD RSM_dwFirPhase[2] = {0,0}; // current number of phase

double* RSM_buffer[2] = {NULL,NULL}; // buffer
DWORD RSM_dwMaxBufferSize; // buffer size
DWORD RSM_dwBufSize[2] = {0,0}; //size of data filled in buffer
DWORD RSM_dwCurPos[2] = {0,0}; // current reading point 
DWORD RSM_dwCounter[2] = {0,0};

DWORD RSM_dwUp = 0;  // up-sampling  points
DWORD RSM_dwDown = 0; // down-sampling points after up-sampling
DWORD RSM_dwConvSize = 0;  // = (RSM_dwFirLn / RSM_dwUp) , FIR convolution size in each phase
DWORD RSM_dwCutoff = 0; // cut-off freq
double RSM_dLoss = 0; // db, loss



//------------------------------------
// get coefficients
// note: *h have to be free later
DWORD GetRSMPCoef(double** h,DWORD dwInputFreq,LPDWORD lpdwUp)
{

	*h = (double*)malloc(sizeof(double)*RSM_dwFirLn + 1024);
	*lpdwUp = RSM_dwUp;
	CalcFirCoefficient(LPF,*h,RSM_dwFirLn,(double)(dwInputFreq*RSM_dwUp),RSM_dLoss,RSM_dwCutoff,RSM_dwCutoff);

	return RSM_dwFirLn;
}


//-------------------------------------------------------
// prepare re-sampling
void prepareRsmp(DWORD dwInputFreq, DWORD dwOutputFreq, 
				 DWORD dwCutFreq, double dDB, 
				 DWORD dwLength, 
				 DWORD dwUp,DWORD dwDown,
				 DWORD dwFilterBufSize,DWORD dwChn){

	DWORD i,i2;
	double *h;
	
	RSM_dwUp = dwUp;   
	RSM_dwDown = dwDown;
	RSM_dwFirLn = dwLength;
	RSM_dwConvSize = RSM_dwFirLn/RSM_dwUp;
	RSM_dwFirPhase[0] = RSM_dwFirPhase[1] = 0;
	RSM_dwCurPos[0] = RSM_dwCurPos[1] = RSM_dwConvSize;
	RSM_dwCutoff = dwCutFreq;
	RSM_dLoss = dDB;

	RSM_dwMaxBufferSize = dwFilterBufSize + RSM_dwConvSize;
	for(i=0;i<dwChn;i++){
		if(RSM_buffer[i] == NULL) RSM_buffer[i] = (double*)malloc(sizeof(double)*RSM_dwMaxBufferSize +1024);
		memset(RSM_buffer[i],0,sizeof(double)*RSM_dwMaxBufferSize);
		RSM_dwBufSize[i] = 0;
	}

	// get LPF filter coefficients
	if(RSM_dFir == NULL){

		RSM_dFir = (double**)malloc(sizeof(double)*RSM_dwUp + 1024);
		RSM_dFir[0] = (double*)malloc(sizeof(double)*RSM_dwFirLn + 1024);
		for(i=1;i<RSM_dwUp;i++) RSM_dFir[i] = RSM_dFir[i-1] + RSM_dwConvSize;  

		h = (double*)malloc(sizeof(double)*RSM_dwFirLn + 1024);
		CalcFirCoefficient(LPF,h,RSM_dwFirLn,(double)(dwInputFreq*RSM_dwUp),RSM_dLoss,RSM_dwCutoff,RSM_dwCutoff);

		// polyphase decomposition into RSM_dwUp-phases
		for(i=0;i<RSM_dwUp;i++){
			for(i2=0;i2<RSM_dwConvSize;i2++){
				RSM_dFir[i][(RSM_dwConvSize-1)-i2] 
					= h[i + i2*RSM_dwUp] 
					* RSM_dwUp; // normalizing term
			}
		}

		free(h);
	}

}




//---------------------------------------------------------------
// unprepare
void unprepareRsmp(){

	int i;

	for(i=0;i<2;i++) {
		if(RSM_buffer[i]) free(RSM_buffer[i]);
		RSM_buffer[i] = NULL;
	}

	if(RSM_dFir){
		free(RSM_dFir[0]);
		free(RSM_dFir);
	}
	RSM_dFir = NULL;


}



//-------------------------------------------------------------
// clear
void ClearRsmp(){

	DWORD i;
	
	for(i=0;i<2;i++){
		if(RSM_buffer[i]) memset(RSM_buffer[i],0,sizeof(double)*RSM_dwMaxBufferSize);
		RSM_dwFirPhase[i] = 0;
		RSM_dwCurPos[i] = RSM_dwConvSize-1;
		RSM_dwBufSize[i] = 0;
		RSM_dwCounter[i] = RSM_dwDown-1;
	}

}



//--------------------------------------------------------------------
// re-sampling
void RSAMP( double* lpFilterBuf, // filter buffer
		   DWORD dwPointsInBuf,
			DWORD dwCh,

			LPDWORD lpdwAfterPoints
			)
{
	if( !dwPointsInBuf ) return;

	DWORD dwPos = 0;
	double *buf1,*buf2;

	SetConvSize(RSM_dwConvSize);
	buf1 = GetConvBufA();
	buf2 = GetConvBufB();

	// copy data to re-sampling buffer
	memcpy(RSM_buffer[dwCh]+RSM_dwBufSize[dwCh],lpFilterBuf,sizeof(double)*dwPointsInBuf);
	RSM_dwBufSize[dwCh] += dwPointsInBuf;
	assert( RSM_dwBufSize[dwCh] <= RSM_dwMaxBufferSize );

	while(RSM_dwFirPhase[dwCh] >= RSM_dwUp){
		RSM_dwFirPhase[dwCh] -= RSM_dwUp;
		RSM_dwCurPos[dwCh]++;
		assert(RSM_dwCurPos[dwCh] < RSM_dwBufSize[dwCh]);
		printf("\n%d - %d\n",RSM_dwFirPhase[dwCh],  RSM_dwUp );
	}
	memcpy(buf2,(RSM_buffer[dwCh]+RSM_dwCurPos[dwCh] -(RSM_dwConvSize-1)),sizeof(double)*RSM_dwConvSize);

	while(1){

		// convolution
		memcpy(buf1,RSM_dFir[RSM_dwFirPhase[dwCh]],sizeof(double)*RSM_dwConvSize);
		lpFilterBuf[dwPos++] = CONV();
		/*
		double out = CONV();
		RSM_dwCounter[dwCh]++;
		if( RSM_dwCounter[dwCh] == RSM_dwDown ){
			RSM_dwCounter[dwCh] = 0;
			lpFilterBuf[dwPos++] = out;
		}
		*/

		// renew phase and reading point of buffer
		RSM_dwFirPhase[dwCh] += RSM_dwDown;
//		RSM_dwFirPhase[dwCh]++;
		bool cpy = false;
		while(RSM_dwFirPhase[dwCh] >= RSM_dwUp){
			RSM_dwFirPhase[dwCh] -= RSM_dwUp;
			RSM_dwCurPos[dwCh]++;
			if(RSM_dwCurPos[dwCh] == RSM_dwBufSize[dwCh]) break;
			cpy = true;			
		}
		if(RSM_dwCurPos[dwCh] == RSM_dwBufSize[dwCh]) break;
		if( cpy ) memcpy(buf2,(RSM_buffer[dwCh]+RSM_dwCurPos[dwCh] -(RSM_dwConvSize-1)),sizeof(double)*RSM_dwConvSize);
	}

	(*lpdwAfterPoints) = dwPos;

	// move RSM_dwConvSize byte data from tail to front for next convolution
	memcpy(RSM_buffer[dwCh],RSM_buffer[dwCh] + (RSM_dwBufSize[dwCh]-(RSM_dwConvSize-1)),sizeof(double)*(RSM_dwConvSize-1));
	RSM_dwCurPos[dwCh] = RSM_dwConvSize-1;
	RSM_dwBufSize[dwCh] = RSM_dwConvSize-1;

};




