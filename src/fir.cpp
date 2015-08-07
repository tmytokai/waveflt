#if 0

// FIR filter 

#ifdef WIN32
#include <windows.h>
#define _USE_MATH_DEFINES
#endif
#include <math.h>

//----------------------------
// conv.c
double CONV();
void InitCONV(unsigned int dwLength);
void UnprepareCONV();
void SetConvSize(unsigned int n);
double* GetConvBufA();
double* GetConvBufB();
bool  CheckSSE2(); // you need to call this function to use SSE2

//-------------------------------------------------
// filterfunc.c
double kaiserAlpha(double dDB);
double I0(double x);


#define MAX_CHN		6   // max number of channels
#define FIR_NUM		3

// type of filter
#define NO_FILTER	0
#define LPF 1
#define HPF 2
#define BPF 3
#define BSF 4
#define SVEQL 5  // shelving EQ low
#define SVEQH 6  // shelving EQ high

double* FIR_buffer[FIR_NUM][MAX_CHN];  // delay buffer
DWORD FIR_dwBufSize[FIR_NUM][MAX_CHN];  // buffer size
DWORD FIR_dwBufferPos[FIR_NUM][MAX_CHN]; // current buffer point

double* FIR_coef[FIR_NUM][MAX_CHN]; // FIR coefficients
DWORD FIR_dwLength[FIR_NUM][MAX_CHN]; // length of filter


//------------------------------------
// get coefficients
DWORD GetFIRCoef(double** coef,DWORD dwFirNum,DWORD dwChn)
{

	*coef = FIR_coef[dwFirNum][dwChn];

	return FIR_dwLength[dwFirNum][dwChn];
}



//-----------------------------
// clear buffer data
void ClearFirBuf(){
	
	DWORD i,i2;
	
	for(i=0;i<FIR_NUM;i++){
		for(i2=0;i2<MAX_CHN;i2++){
			if(FIR_buffer[i][i2]) 
				memset(FIR_buffer[i][i2],0,sizeof(double)*FIR_dwBufSize[i][i2]);
			FIR_dwBufferPos[i][i2] = 0;
		}
	}
	
}



//-----------------------------------
// create buffer
void CreateBufFIR(DWORD dwFIRleng, // points, length of FIR filter
				  DWORD dwBufSize, // buffer size
				  DWORD dwFirNum,
				  DWORD dwChn){

	FIR_dwBufSize[dwFirNum][dwChn] = dwBufSize;

	if(!FIR_buffer[dwFirNum][dwChn]) 
		FIR_buffer[dwFirNum][dwChn] = (double*)malloc(sizeof(double)*FIR_dwBufSize[dwFirNum][dwChn]+1024);

	FIR_dwLength[dwFirNum][dwChn] = dwFIRleng;

	if(!FIR_coef[dwFirNum][dwChn]) 
		FIR_coef[dwFirNum][dwChn] = (double*)malloc(sizeof(double)*FIR_dwLength[dwFirNum][dwChn]+1024); 

	ClearFirBuf();
}


//-----------------------------------
// rewind buffer position
void RewindBufFIR(DWORD dwRewSize,DWORD dwFirNum,DWORD dwChn){

	DWORD dwPos;

	dwPos = FIR_dwBufferPos[dwFirNum][dwChn];
	dwPos = (FIR_dwBufSize[dwFirNum][dwChn] + dwPos - dwRewSize) % FIR_dwBufSize[dwFirNum][dwChn];

	FIR_dwBufferPos[dwFirNum][dwChn] = dwPos;
}



//-----------------------------------
// free all buffers and reset all parameters
void unprepareFIR(){

	DWORD i,i2;
	
	for(i=0;i<FIR_NUM;i++){
		for(i2=0;i2<MAX_CHN;i2++){
			if(FIR_buffer[i][i2]) free(FIR_buffer[i][i2]);
			if(FIR_coef[i][i2]) free(FIR_coef[i][i2]);
			FIR_buffer[i][i2] = NULL;
			FIR_coef[i][i2] = NULL;
			FIR_dwBufSize[i][i2] = 0;
			FIR_dwBufferPos[i][i2] = 0;
			FIR_dwLength[i][i2]  = 0;
		}
	}
}



//--------------------------------------
// coefficients of linear phase FIR filter via window method (kaiser window)
void CalcFirCoefficient(DWORD dwFilter,  // type of filter
						double * coeff, 

						DWORD dwLength,
						double dbSmpRate,  // sampling rate
						double dDB,  // loss
						DWORD dwCutLow,  // cut-off freq (low)
						DWORD dwCutHigh  // cut-off freq (high)
						){
	DWORD i,dwCenter;
	double foo;
	double alpha,f_l,f_h;

	// normalize cut-off freq between 0 and 1.0
	f_l = (double)dwCutLow/(dbSmpRate/2.); 
	f_h = (double)dwCutHigh/(dbSmpRate/2.); 

	dwCenter = (dwLength-1)/2;

	// get impulse
	switch(dwFilter){
		
	case LPF:
		coeff[dwCenter] = f_l;
		for(i=1; i <= dwCenter ; i++) coeff[dwCenter+i] = sin(M_PI*i*f_l)/(M_PI*i);
		break;
		
	case HPF:
		coeff[dwCenter] = 1-f_l;
		for(i=1; i <= dwCenter ; i++) coeff[dwCenter+i] = -sin(M_PI*i*f_l)/(M_PI*i);
		break;
		
	case BPF:
		coeff[dwCenter] = (f_h-f_l);
		for(i=1; i <= dwCenter ; i++) coeff[dwCenter+i] = (sin(M_PI*i*f_h)-sin(M_PI*i*f_l))/(M_PI*i);
		break;
		
	case BSF:
		coeff[dwCenter] = 1-(f_h-f_l);
		for(i=1; i <= dwCenter ; i++) coeff[dwCenter+i] = -(sin(M_PI*i*f_h)-sin(M_PI*i*f_l))/(M_PI*i);
		break;
	}


	// shift
	alpha = kaiserAlpha(dDB);
	for(i=1;i<=dwCenter;i++) {

		foo = coeff[dwCenter+i]
			// x kaiser window
			*I0(alpha*sqrt(1-(2*i/(double)(dwLength-1)) 
			*(2*i/(double)(dwLength-1)) ))/I0(alpha);

		coeff[dwCenter+i] = foo;
		coeff[dwCenter-i] = foo;
	}
	
}


//--------------------------------------------------------
// FIR filter
void FIR(double* lpFilterBuf, // filter buffer
		 DWORD dwPointsInBuf, // points of data in the filter buffer
		 DWORD dwFirNum, // number of filter
		 DWORD dwChn // channel
		 ){
	
	DWORD i,i2,dwPos,dwBufferPos,dwConvSize; 
	double dFoo;
	double *buf1,*buf2;

	dwConvSize = (FIR_dwLength[dwFirNum][dwChn]-1)/2;
	SetConvSize(dwConvSize);
	buf1 = GetConvBufA();
	buf2 = GetConvBufB();

	dwPos = 0;
	dwBufferPos = FIR_dwBufferPos[dwFirNum][dwChn];

	// copy data to convolution buffer
	memcpy(buf2,FIR_coef[dwFirNum][dwChn],sizeof(double)*dwConvSize);

	for(i=0;i< dwPointsInBuf;i++){

		FIR_buffer[dwFirNum][dwChn][dwBufferPos] = lpFilterBuf[dwPos];

		// note that coefficients of linear phase FIR filter are symmetric.
		for(i2 = 0; i2 < dwConvSize; i2++) buf1[i2] = 
			(FIR_buffer[dwFirNum][dwChn][(dwBufferPos-i2) & (FIR_dwBufSize[dwFirNum][dwChn]-1)]
			+FIR_buffer[dwFirNum][dwChn][(dwBufferPos-(FIR_dwLength[dwFirNum][dwChn]-1)+i2) & (FIR_dwBufSize[dwFirNum][dwChn]-1)]);

		// convolution
		dFoo = CONV();
		// center
		dFoo += FIR_coef[dwFirNum][dwChn][dwConvSize] 
			* FIR_buffer[dwFirNum][dwChn][(dwBufferPos-dwConvSize) & (FIR_dwBufSize[dwFirNum][dwChn]-1)]; 

		lpFilterBuf[dwPos] = dFoo;
		
		dwBufferPos = (dwBufferPos+1) & (FIR_dwBufSize[dwFirNum][dwChn]-1);
		dwPos++;
	}

	FIR_dwBufferPos[dwFirNum][dwChn] = dwBufferPos;
}





//--------------------------------------
// set coefficients of linear phase FIR filter of each channel
void prepareFIRCoefficient(DWORD dwFilter,  // type of filter
						   DWORD dwFIRleng, // length of filter
						double dbSmpRate,  // sampling rate
						double dDB,  // dB, loss
						DWORD dwCutLow,  // cut-off freq (low)
						DWORD dwCutHigh,  // cut-off freq (high)
						DWORD dwBufSize, // buffer size
						DWORD dwFirNum, // number of filter
						DWORD dwChn // channel
						){

	CreateBufFIR(dwFIRleng,dwBufSize,dwFirNum,dwChn);

	CalcFirCoefficient(dwFilter,FIR_coef[dwFirNum][dwChn],
		FIR_dwLength[dwFirNum][dwChn],dbSmpRate,dDB,dwCutLow,dwCutHigh);

}




//------------------------------------------------
// set coefficients of linear phase FIR-EQ 
void CalcEQCoefficient(DWORD dwFIRleng,
					   DWORD dwBand,  // band
					   double dbSmpRate, // sampling rate
					   double dDB,  // dB, loss
					   double* dbEQlevel,  // dB, levels
					   double dQ, // Q
					   DWORD dwBufSize, // buffer size
					   DWORD dwFirNum, // number of filter
					   DWORD dwChn // channel
					   ){

	double* h;
	DWORD i,i2;
	double dCntFreq[10] = {31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000}; 
	double dLow,dHigh,dHalfFreq;
	double dGain;
	DWORD dwFilter;

	CreateBufFIR(dwFIRleng,dwBufSize,dwFirNum,dwChn);

	if(dwBand == 3){
		dCntFreq[0] = 125;
		dCntFreq[1] = 1000;
		dCntFreq[2] = 8000;
	}

	h = (double*)malloc(sizeof(double)*FIR_dwLength[dwFirNum][dwChn] + 1024);
	
	dHalfFreq = dCntFreq[1]/2./dQ;
	dLow = dHigh = dCntFreq[1] - dHalfFreq;
	dGain = pow(10.,dbEQlevel[0]/20);
	CalcFirCoefficient(LPF,FIR_coef[dwFirNum][dwChn],
		FIR_dwLength[dwFirNum][dwChn],dbSmpRate,dDB,(DWORD)dLow,(DWORD)dHigh);
	for(i2=0;i2<FIR_dwLength[dwFirNum][dwChn];i2++) FIR_coef[dwFirNum][dwChn][i2] *= dGain;

	for(i=1;i<dwBand;i++){
		dHalfFreq = dCntFreq[i]/2./dQ;
		dLow = dCntFreq[i] - dHalfFreq;
		dHigh = dCntFreq[i] + dHalfFreq;
		dGain = pow(10.,dbEQlevel[i]/20);
		if(i==dwBand-1 || dHigh > dbSmpRate/2) dwFilter = HPF;
		else dwFilter = BPF;
		CalcFirCoefficient(dwFilter,h,FIR_dwLength[dwFirNum][dwChn],
			dbSmpRate,dDB,(DWORD)dLow,(DWORD)dHigh);
		for(i2=0;i2<FIR_dwLength[dwFirNum][dwChn];i2++) 
			FIR_coef[dwFirNum][dwChn][i2] += h[i2]*dGain;

		if(dHigh > dbSmpRate/2) break;
	}

	free(h);

}



//------------------------------------------------------------------------



// EOF

#endif