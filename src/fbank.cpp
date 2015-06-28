// filter bank

#ifdef WIN32
#include <windows.h>
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#define MAX_CHN		6   // max number of channels

#include "fbank.h"

#define SUB_BAND 32
#define FIR_TAP	8

// buffer for analysis, 2 * SUB_BAND (number of phaise) * FIR_TAP (FIR taps of each phase)
double FBK_anaBuf[MAX_CHN][(2*SUB_BAND*FIR_TAP)];  
DWORD  FBK_dwAnaPos[MAX_CHN];

// buffer for synthesis
double FBK_synthBuf[MAX_CHN][2*(2*SUB_BAND*FIR_TAP)];  
DWORD  FBK_dwSynthPos[MAX_CHN];

// DCT coefficients
double FBK_DCT[SUB_BAND][SUB_BAND*2];
double FBK_IDCT[SUB_BAND][SUB_BAND*2];


//-------------------------------------------------
// clear filter bank
void ClearFltBank(){

	int i,k;
	
	for(i=0;i<MAX_CHN;i++){
		memset(FBK_anaBuf[i],0,sizeof(double)*(2*SUB_BAND*FIR_TAP));
		memset(FBK_synthBuf[i],0,sizeof(double)*(2*SUB_BAND*FIR_TAP));
		FBK_dwAnaPos[i] = 0;
		FBK_dwSynthPos[i] = 0;
	}

	// set DCT coefficients
	for(i=0;i<SUB_BAND;i++){
		for(k=0;k<SUB_BAND*2;k++){
		FBK_DCT[i][k] = cos((2*i+1)*(k-16)*M_PI/(SUB_BAND*2));
		FBK_IDCT[i][k] = cos((2*i+1)*(k+16)*M_PI/(SUB_BAND*2));
		} 
	}
}



//--------------------------------------------------
// analysis
VOID FltBank_analysis(double *dInput, // SUB_BAND points, input
				double *dOutput,   // SUB_BAND points, output
				DWORD dwCh){

	DWORD i,j,dwPhase,winpos,bufpos,M,K;
	double dFirOut[2*SUB_BAND];
	double dFoo,alpha;

	M = SUB_BAND;
	K = FIR_TAP;

	// delay and down-sampling 
	for(i=0;i< M; i++) FBK_anaBuf[dwCh][(FBK_dwAnaPos[dwCh] +i)&((2*K*M)-1)] = dInput[i];
	FBK_dwAnaPos[dwCh]=(FBK_dwAnaPos[dwCh]+M) & ((2*K*M)-1);

	// polyphase FIR filter (2 * M phases, K taps)
	for(dwPhase = 0;dwPhase < 2*M; dwPhase++){  
		alpha = 1;
		dFirOut[dwPhase]=0;
		for(j=0;j< K ;j++){
			winpos = dwPhase + (2*M)*j;		
			bufpos=(FBK_dwAnaPos[dwCh] - dwPhase - (2*M)*j) & ((2*K*M)-1);
			dFirOut[dwPhase] += alpha*FBK_anawin[winpos]*FBK_anaBuf[dwCh][bufpos];
			alpha *= -1;  // if j is odd number , alpha =  -1 because of G_k(-z^2)
		}
	}

	// DCT (2*M -> M)
	for(i=0;i<M;i++){
		dFoo = 0;
		for(j=0 ;j < 2*M; j++) dFoo += FBK_DCT[i][j]*dFirOut[j];
		dOutput[i] = dFoo;
	}
}




//--------------------------------------------------
// synthesis
VOID FltBank_synthesis(double *dOutput, // SUB_BAND points, output
				double *dInput,   // SUB_BAND points, input
				DWORD  dwCh // channel
				){

	DWORD i,j,k,dwPhase,winpos,bufpos,M,K;
	double dFirOut[SUB_BAND*2];
	double dFoo,alpha;

	M = SUB_BAND;
	K = FIR_TAP;

	// IDCT (M -> 2*M)
	for(i=0;i<2*M;i++){
		dFoo = 0;
		for(k=0;k<M;k++) dFoo += FBK_IDCT[k][i] * dInput[k];
		FBK_synthBuf[dwCh][(FBK_dwSynthPos[dwCh]+i)& (2*(2*K*M)-1)] = dFoo;
	}

	// polyphase FIR filter (2*M phases, K taps)
	for(dwPhase = 0;dwPhase < 2*M; dwPhase++){
		dFirOut[dwPhase] = 0;
		alpha = 1.0;
		for(j=0 ;j<K ;j++){
			winpos = dwPhase +(2*M)*j;		
			bufpos= (FBK_dwSynthPos[dwCh]
				+dwPhase
				-2*(2*M)*j  // z^(-2*j)
				-(2*M)*(dwPhase >= M) // z^(-1)
				) & (2*(2*K*M)-1);
			dFirOut[dwPhase] += alpha * FBK_synwin[winpos] * FBK_synthBuf[dwCh][bufpos];
			alpha *= -1;
		}
	}

	// up-sampling and delay
	for(i=0; i<M ;i++) dOutput[i] = dFirOut[i] + dFirOut[i+M];

	FBK_dwSynthPos[dwCh] = (FBK_dwSynthPos[dwCh]+2*M) & (2*(2*K*M)-1);

}

// EOF