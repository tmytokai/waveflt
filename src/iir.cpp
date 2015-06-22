// IIR filter

#include "filter.h"

#define IIR_NUM		5
#define MAX_IIRDIM	5

double IIR_InputBuffer[IIR_NUM][MAX_CHN][MAX_IIRDIM];
double IIR_OutBuffer[IIR_NUM][MAX_CHN][MAX_IIRDIM];
DWORD IIR_DwBufferPos[IIR_NUM][MAX_CHN];

double IIR_A[IIR_NUM][MAX_CHN][MAX_IIRDIM]; 
double IIR_B[IIR_NUM][MAX_CHN][MAX_IIRDIM];
DWORD IIR_dwDim[IIR_NUM][MAX_CHN];


//-------------------------------------------
// get impulse of IIR
DWORD GetIIRimpulse(double* imp,DWORD N,DWORD dwIirNum,DWORD dwChn){
	
	memset(imp,0,sizeof(double)*N);
	imp[0] = 1;

	// get impulse
	IIR(imp,N,dwIirNum,dwChn);

	return N;
}


//------------------------------------
// get coefficients
DWORD GetIIRCoef(double* dCoef[2],DWORD dwIirNum,DWORD dwChn)
{

	dCoef[0] = IIR_A[dwIirNum][dwChn];
	dCoef[1] = IIR_B[dwIirNum][dwChn];

	return IIR_dwDim[dwIirNum][dwChn];
}



//--------------------------------------------------------
// IIR filter
void IIR(double* lpFilterBuf, // buffer
		 DWORD dwPointsInBuf, // points

		 DWORD dwIirNum,
		 DWORD dwChn
		 ){
	
	DWORD i,n,dwPos,dwCurBufPos; 
	double dInput,dOutput;

	dwPos = 0;

	for(i=0;i< dwPointsInBuf;i++){
		dInput = lpFilterBuf[dwPos];

		dOutput = IIR_B[dwIirNum][dwChn][0]*dInput;
		for(n=1;n<=IIR_dwDim[dwIirNum][dwChn];n++){
			dwCurBufPos = (IIR_DwBufferPos[dwIirNum][dwChn]+(n-1))%IIR_dwDim[dwIirNum][dwChn];
			dOutput += IIR_B[dwIirNum][dwChn][n]*IIR_InputBuffer[dwIirNum][dwChn][dwCurBufPos];
			dOutput += IIR_A[dwIirNum][dwChn][n]*IIR_OutBuffer[dwIirNum][dwChn][dwCurBufPos]; 
		}

		if(IIR_DwBufferPos[dwIirNum][dwChn] ==0) IIR_DwBufferPos[dwIirNum][dwChn] = IIR_dwDim[dwIirNum][dwChn]-1; 
		else IIR_DwBufferPos[dwIirNum][dwChn]--;
		dwCurBufPos = IIR_DwBufferPos[dwIirNum][dwChn];
		IIR_InputBuffer[dwIirNum][dwChn][dwCurBufPos] = dInput;
		IIR_OutBuffer[dwIirNum][dwChn][dwCurBufPos] = dOutput;

		lpFilterBuf[dwPos] = dOutput;
	
		dwPos++;
	}
}




//-----------------------------
// clear buffer
void ClearIirBuf(){

	DWORD i,i2;
	
	for(i=0;i<IIR_NUM;i++){
		for(i2=0;i2<MAX_CHN;i2++){
			memset(IIR_InputBuffer[i][i2],0,sizeof(double)*MAX_IIRDIM);
			memset(IIR_OutBuffer[i][i2],0,sizeof(double)*MAX_IIRDIM);
			IIR_DwBufferPos[i][i2] = 0;
		}
	}
	
}



//-----------------------------------
// set IIR filter
void prepareIIRCoefficient(DWORD dwFilter,// type of filter
						   double dCutFreqL, // low
						   double dCutFreqH, // hish
						   double dSmpRate,  // sampling rate
						   DWORD N,  // dimension of proto-filter

						   DWORD dwIirNum, // ID of filter
						   DWORD dwChn // channel
						   ){
	
	if(dwFilter == BPF){ // 4dim-IIR for BPF
		IIR4_BPF_Butterworth(IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],1,N,dCutFreqL,dCutFreqH,dSmpRate);
		IIR_dwDim[dwIirNum][dwChn] = N*2;
	}
	else // 2dim
	{
		IIR2_Butterworth(dwFilter,IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],1,N,dCutFreqL,dCutFreqH,dSmpRate);
		IIR_dwDim[dwIirNum][dwChn] = N;
	}
	
	ClearIirBuf();
}




//-----------------------------------
// shelving EQ
void CalcIirShelvingEQ(DWORD dwFilter,
					   double dCutFreq,
					 double dDb,
					 double dSmpRate,  // sampling rate
					 DWORD dwIirNum, // ID of filter
					 DWORD dwChn // channel
					 ){

	IIR1_ShelvingEQ(dwFilter,IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],dCutFreq,dDb,dSmpRate);
	IIR_dwDim[dwIirNum][dwChn] = 1;
	ClearIirBuf();
}





//-----------------------------------
// peaking EQ
void CalcIirPeakingEQ(double dCntFreq,  // center
					  double dQ,  // Q
					  double dDb,
					  double dSmpRate,  // sampling rate
					  DWORD dwIirNum, // ID of filter
					  DWORD dwChn // channel
					  ){
	double dCutFreqL,dCutFreqH,dHalfFreq; 
	double dT1,dT2,alpha;
	
	dHalfFreq = dCntFreq/2./dQ;
	dCutFreqL = dCntFreq - dHalfFreq;
	dCutFreqH = dCntFreq + dHalfFreq;

	if(dCutFreqL < 0){
		IIR1_ShelvingEQ(SVEQL,IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],dCutFreqH,dDb,dSmpRate);
		IIR_dwDim[dwIirNum][dwChn] = 1;
	}
	else if(dCutFreqH > dSmpRate/2){

		dT2 = 1./(2.*PI*dCutFreqL);
		alpha = pow(10.,-fabs(dDb)/20);
		dT1 =dT2/alpha;
		dCutFreqL = 1./(2.*PI*dT1);

		IIR1_ShelvingEQ(SVEQH,IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],dCutFreqL,dDb,dSmpRate);
		IIR_dwDim[dwIirNum][dwChn] = 1;
	}
	else{
		IIR2_PeakingEQ(IIR_A[dwIirNum][dwChn],IIR_B[dwIirNum][dwChn],dCutFreqL,dCutFreqH,dDb,dSmpRate);
		IIR_dwDim[dwIirNum][dwChn] = 2;
	}
	
	ClearIirBuf();
}




//--------------------------------------------------------------
//--------------------------------------------------------------



//-----------------------------------
// basic 2-dim Butterworth IIR filter
void IIR2_Butterworth(DWORD dwFilter,// type of filter
					double a[3],
					double b[3],
					double k,
					double N,

					double dCutFreqL, // low
					double dCutFreqH, // hish
						
					double dSmpRate  // sampling rate
					){

	double dOmg,dOmgL,dOmgH,theta,a0;
	DWORD i;
	double alpha;
	double beta;

	switch(dwFilter){
		
	case LPF:
		// LPF (Butterworth 2)

		dOmg = tan(2*PI*dCutFreqL/(2*dSmpRate)); // pre-warping
		theta = (2*k-1)*PI/(2*N);

		a[0] = 1. + 2.*dOmg*cos(theta) +dOmg*dOmg;
		a[1] = 2.*(1.-dOmg*dOmg);
		a[2] = (2.*dOmg*cos(theta)-1.-dOmg*dOmg);

		b[0] = dOmg*dOmg;
		b[1] = 2*dOmg*dOmg;
		b[2] = dOmg*dOmg;
		
		break;
		
	case HPF:
		//HPF (Butterworth 2)

		dOmg = tan(2*PI*dCutFreqL/(2*dSmpRate)); // pre-warping
		theta = (2*k-1)*PI/(2*N);

		a[0] = 1. + 2.*dOmg*cos(theta) +dOmg*dOmg;
		a[1] = 2.*(1.-dOmg*dOmg);
		a[2] = (2.*dOmg*cos(theta)-1.-dOmg*dOmg);

		b[0] = 1;
		b[1] = -2;	
		b[2] = 1;

		break;
		
	case BPF: 
		// BPF (Butterworth 1)
		
		dOmgL = tan(2.*PI*dCutFreqL/(2*dSmpRate)); 
		dOmgH = tan(2.*PI*dCutFreqH/(2*dSmpRate)); 

		alpha = (dOmgH*dOmgL);
		beta = (dOmgH-dOmgL); 

		a[0] = 1. + alpha + beta;
		a[1] = 2*(1.-alpha);
		a[2] = (beta-alpha-1.);
		b[0] = beta;
		b[1] = 0;
		b[2] = -b[0];
	}
	
	a0 = a[0];
	for(i=0;i<3;i++){
		a[i] /= a0;
		b[i] /= a0;
	}	
}



//-----------------------------------
// basic 4-dim Butterworth IIR BPF filter
void IIR4_BPF_Butterworth(double a[5],
					double b[5],
					double k,
					double N,

					double dCutFreqL, // low
					double dCutFreqH, // hish
						
					double dSmpRate  // sampling rate
					){

	double dOmgL,dOmgH,theta,a0;
	DWORD i;
	double alpha;
	double beta;

	theta = (2*k-1)*PI/(2*N);
		
	dOmgL = tan(2.*PI*dCutFreqL/(2*dSmpRate)); 
	dOmgH = tan(2.*PI*dCutFreqH/(2*dSmpRate)); 
	alpha = (dOmgH*dOmgL);
	beta = (dOmgH-dOmgL); 

	a[0] = (1+alpha)*(1+alpha) + 2*(1+alpha)*cos(theta)*beta+beta*beta;
	a[1] = -4*(1+alpha)*(alpha-1)-4*(alpha-1)*cos(theta)*beta;
	a[2] = -4*(alpha-1)*(alpha-1)-2*(1+alpha)*(1+alpha)
		+2*(1+alpha)*cos(theta)*beta-2*(1+alpha)*cos(theta)*beta+2*beta*beta;
	a[3] = -4*(1+alpha)*(alpha-1)+4*(alpha-1)*cos(theta)*beta;
	a[4] = -(1+alpha)*(1+alpha) +2*(1+alpha)*cos(theta)*beta-beta*beta;
	b[0] = beta*beta;
	b[1] = 0;
	b[2] = -2*beta*beta;
	b[3] = 0;
	b[4] = beta*beta;
	
	a0 = a[0];
	for(i=0;i<5;i++){
		a[i] /= a0;
		b[i] /= a0;
	}	
}


//--------------------------------------------------
// shelving EQ IIR-1
//
// H(s) = (1+gamma*s)/(1+s)
//
void IIR1_ShelvingEQ(DWORD dwFilter,
				   double a[2],
				   double b[2],
				   double dCutFreq,
				   double dDb,
				   double dSmpRate  // sampling rate
						   ){

	DWORD i;
	double a0;

	double dOmg = tan(2.*PI*dCutFreq/(2*dSmpRate)); 
	double gamma = pow(10.,-fabs(dDb)/20);
	
	switch(dwFilter){
		
	case SVEQH:
		
		if(dDb < 0){
			a[0] = (1+dOmg);
			a[1] = (1-dOmg);
			b[0] = dOmg+gamma;
			b[1] = dOmg-gamma;
		}
		else
		{
			a[0] = dOmg+gamma;
			a[1] = -(dOmg-gamma);
			b[0] = (1+dOmg);
			b[1] = -(1-dOmg);
		}
		
		break;

	case SVEQL:
		
		if(dDb >= 0){
			a[0] = (1+dOmg);
			a[1] = (1-dOmg);
			b[0] = (dOmg+gamma)/gamma;
			b[1] = (dOmg-gamma)/gamma;
		}
		else
		{
			a[0] = dOmg+gamma;
			a[1] = -(dOmg-gamma);
			b[0] = (1+dOmg)*gamma;
			b[1] = -(1-dOmg)*gamma;
		}
		
		break;

	}

	a0 = a[0];
	for(i=0;i<2;i++){
		a[i] /= a0;
		b[i] /= a0;
	}

}



//----------------------------------------
// peaking EQ
// H(s) = (1+gamma*s)/(1+s)  
void IIR2_PeakingEQ(double a[3],
				   double b[3],

					double dCutFreqL, // low
					double dCutFreqH, // hish
					double dDb,

					double dSmpRate  // sampling rate
					){

	double dOmgL,dOmgH,a0;
	DWORD i;
	double alpha;
	double beta;
	double gamma = pow(10.,-fabs(dDb)/20);

	dOmgL = tan(2.*PI*dCutFreqL/(2*dSmpRate)); 
	dOmgH = tan(2.*PI*dCutFreqH/(2*dSmpRate)); 
	alpha = (dOmgH*dOmgL);
	beta = (dOmgH-dOmgL); 
	
	if(dDb > 0){
		a[0] = 1. + alpha + beta;
		a[1] = 2*(1.-alpha);
		a[2] = (beta-alpha-1.);
		
		b[0] = (gamma + gamma*alpha +beta)/gamma;
		b[1] = (2*gamma*(alpha-1))/gamma;
		b[2] = (gamma + gamma*alpha -beta)/gamma;
	}
	else
	{
		a[0] = (gamma + gamma*alpha +beta);
		a[1] = -(2*gamma*(alpha-1));
		a[2] = -(gamma + gamma*alpha -beta);

		b[0] = (1. + alpha + beta)*gamma;
		b[1] = -2*(1.-alpha)*gamma;
		b[2] = -(beta-alpha-1.)*gamma;
	}

	a0 = a[0];
	for(i=0;i<3;i++){
		a[i] /= a0;
		b[i] /= a0;
	}

}


// EOF