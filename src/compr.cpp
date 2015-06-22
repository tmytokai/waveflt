// compressor

#include "filter.h"

#define COMPMODE_ATTACK 0
#define COMPMODE_RELEASE 1
#define COMP_NUM	4

double COMP_dRatio[COMP_NUM];  // current ratio
double COMP_dPeak[COMP_NUM]; // peak
double* COMP_lpRMSbuf[COMP_NUM][2]; // buffer to calculate RMS (L-R)
DWORD COMP_dwRMSsize[COMP_NUM]; // buffer size
DWORD COMP_dwRMSpos[COMP_NUM];
double COMP_dRMS[COMP_NUM][2];  // RMS



//----------------------------
// clear
void ClearCOMP(){

	DWORD i,i2;
	for(i=0;i<COMP_NUM;i++){
		for(i2=0;i2<2;i2++){
			if(COMP_lpRMSbuf[i][i2]) memset(COMP_lpRMSbuf[i][i2],0,sizeof(double)*COMP_dwRMSsize[i]);
			COMP_dRMS[i][i2] = 0;
		}
		COMP_dRatio[i] = 1.0;
		COMP_dPeak[i] = -1000;
		COMP_dwRMSpos[i] = 0;
	}
}


//-------------------------------------
// init
void prepareCOMP(DWORD dwChn,
				 DWORD dwRMSsize, // frame size of RMS
				 DWORD dwCompNum){

	DWORD i;

	if(dwRMSsize == 0) COMP_dwRMSsize[dwCompNum] = 1;
	else COMP_dwRMSsize[dwCompNum] = dwRMSsize;

	for(i=0;i<dwChn;i++){
		if(!COMP_lpRMSbuf[dwCompNum][i]) 
			COMP_lpRMSbuf[dwCompNum][i] = (double*)malloc(sizeof(double)*COMP_dwRMSsize[dwCompNum]);
	}

	ClearCOMP();
}


//----------------------------------
// close
void unprepareCOMP(){

	DWORD i,i2;

	for(i=0;i<COMP_NUM;i++){
		for(i2=0;i2<2;i2++){
			if(COMP_lpRMSbuf[i][i2]) free(COMP_lpRMSbuf[i][i2]);
			COMP_lpRMSbuf[i][i2] = NULL;
		}
	}

}



//------------------------------
// get current gain
double COMP_getCurrentGain(double dCurrentPeak,  // dB

							double dTh,  // dB, threshold
							double dRatio,
							double dAttackSpeed,
							double dReleaseSpeed,
							DWORD dwCompNum
			   ){

	double dGain;
	DWORD dwMode;

	if(dCurrentPeak > dTh) dwMode = COMPMODE_ATTACK;
	else dwMode = COMPMODE_RELEASE;

	switch(dwMode){

	case COMPMODE_ATTACK: 

		COMP_dRatio[dwCompNum] += dAttackSpeed;
		COMP_dRatio[dwCompNum] = min(dRatio , COMP_dRatio[dwCompNum]);
		COMP_dPeak[dwCompNum] = max(COMP_dPeak[dwCompNum] , dCurrentPeak);

		break;
		
	case COMPMODE_RELEASE: 

		COMP_dRatio[dwCompNum] -= dReleaseSpeed;
		COMP_dRatio[dwCompNum] = max(1.0,COMP_dRatio[dwCompNum]);

		break;
	}

	if(COMP_dRatio[dwCompNum] != 1.0){
		dGain = pow(10.,(dTh+(COMP_dPeak[dwCompNum] -dTh)/COMP_dRatio[dwCompNum] )/20)
			/pow(10.,COMP_dPeak[dwCompNum]/20);
		if(dGain >= 1.0) dGain = 1.0;
		else if(dGain <= 0) dGain = 0;
	}
	else{
		COMP_dPeak[dwCompNum] = -1000;
		dGain = 1.0;
	}

	return dGain;
}




//-----------------------------------
// compressor
void COMPRESS(long nSamplesPerSec,
			 double* lpFilterBuf[2],
			 DWORD dwPointsInBuf, 
			 DWORD dwChn,

			 double dTh,  // threshold
			 double dRatio,
			 double dAttackTime,  // msec
			 double dReleaseTime,  // msec
			 DWORD dwCompNum
			 ){
	
	DWORD i,i2,i3;
	double dPeak,dGain,dMax;
	double dAttackSpeed;
	double dReleaseSpeed;
	double dFoo;
	DWORD dwPos;
	

	if(dAttackTime == 0) dAttackSpeed = 1000;
	else dAttackSpeed = (dRatio-1.)/((double)nSamplesPerSec*dAttackTime/1000);
	if(dReleaseTime == 0) dReleaseSpeed = 1000;
	dReleaseSpeed = (dRatio-1.)/((double)nSamplesPerSec*dReleaseTime/1000);

	for(i=0;i< dwPointsInBuf;i++){
		
		// calulate RMS
		COMP_dwRMSpos[dwCompNum] = (COMP_dwRMSpos[dwCompNum]+1) % COMP_dwRMSsize[dwCompNum];
		dwPos = COMP_dwRMSpos[dwCompNum];
		if(COMP_dwRMSpos[dwCompNum]){
			for(i2=0;i2<dwChn;i2++){
				dFoo = COMP_lpRMSbuf[dwCompNum][i2][dwPos];
				COMP_dRMS[dwCompNum][i2] -= dFoo * dFoo;
				COMP_dRMS[dwCompNum][i2] += lpFilterBuf[i2][i]*lpFilterBuf[i2][i];
				COMP_lpRMSbuf[dwCompNum][i2][dwPos] = lpFilterBuf[i2][i];
			}
		}
		else{ // renew RMS per COMP_dwRMSsize
			for(i2=0;i2<dwChn;i2++){
				COMP_dRMS[dwCompNum][i2] = 0;
				COMP_lpRMSbuf[dwCompNum][i2][dwPos] = lpFilterBuf[i2][i];
				for(i3=0;i3<COMP_dwRMSsize[dwCompNum];i3++){
					COMP_dRMS[dwCompNum][i2] 
					+= COMP_lpRMSbuf[dwCompNum][i2][i3] * COMP_lpRMSbuf[dwCompNum][i2][i3];
				}
			}
		}
		
		// stereo link
		dMax = max(
			sqrt(COMP_dRMS[dwCompNum][0]/COMP_dwRMSsize[dwCompNum]),
			sqrt(COMP_dRMS[dwCompNum][1]/COMP_dwRMSsize[dwCompNum])
			);
		dPeak = 20*log10(dMax);
		dGain = COMP_getCurrentGain(dPeak,dTh,dRatio,dAttackSpeed,dReleaseSpeed,dwCompNum);
	
		for(i2=0;i2<dwChn;i2++) lpFilterBuf[i2][i] *= dGain;
	}
	
}
	





// EOF