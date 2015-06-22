// FFT , IFFT

#include "filter.h"


#define FFT_NUM 4

double *FFT_cos[FFT_NUM]; // table of sine and cosine
double *FFT_sin[FFT_NUM]; 
DWORD FFT_dwMaxBit[FFT_NUM]; // max number of bit
DWORD FFT_dwLn[FFT_NUM];  // length


//------------------------------------------------------
// the table of sine and cosine
void calcFftCosTable(double* lpdCos,  
				double* lpdSin,  
				DWORD* lpdwMaxBit, // max number of bit
				DWORD dwFftPoint  
){
	 
	 DWORD i,i2;
	 double dRad;

	 // max of bit
	 i=dwFftPoint-1; 
	 *lpdwMaxBit = 0;  
	 while(i>0){
		 i>>=1;
		 (*lpdwMaxBit)++;
	 }

	 // table of sin and cos
	 for(i=1;i<=(*lpdwMaxBit);i++){
		 for(i2=0;i2<dwFftPoint/2;i2++){ 
			 dRad = 2.*PI*(double)(i2)/(double)(1<<i); 
			 lpdCos[i*dwFftPoint/2+i2] = cos(dRad);				 
			 lpdSin[i*dwFftPoint/2+i2] = sin(dRad);		
		 }	 
	 }
	 

}



//------------------------------------------------------
// FFT
void CalcFft(COMPLEX* data, // input and output
	 DWORD dwFftPoint, // length
	 DWORD dwMaxBit,  // max number of bit
	 double* dCosTable,// table of cosine
	 double* dSinTable // table of sine
	 ){
	 
	 DWORD dwStage,dwMaxStage;
	 DWORD dwPart;
	 DWORD dwDataHeight,dwDataHeight2;
	 
	 DWORD dwDataPos1,dwDataPos2,dwPoint;
	 
	 COMPLEX X1,X2;
	 double dReal3,dImag3;
	 
	 DWORD i,j,k;
	 double dSinn,dCoss;
	 COMPLEX swap;


	 dwPart = 1;
	 dwMaxStage = dwMaxBit;
	 for(dwStage = dwMaxStage ;dwStage>0; dwStage--)
	 {
		 dwDataHeight = (DWORD)1 << dwStage; // 2^dwStage 
		 dwDataHeight2 = dwDataHeight/2;

		 // butterfly
		 for(j = 0; j < dwPart ;j++){	
			 
			 dwDataPos1 = dwDataHeight*j;
			 dwDataPos2 = dwDataPos1+dwDataHeight2;
			 dwPoint = dwStage*dwFftPoint/2;
			 
			 for(k = 0 ; k <  dwDataHeight2 ; k++){
				 
				 X1 = data[dwDataPos1];	
				 X2 = data[dwDataPos2];	
				 
				 data[dwDataPos1].r = X1.r + X2.r; 
				 data[dwDataPos1].i = X1.i + X2.i; 
				 
				 dReal3 = X1.r - X2.r;	  
				 dImag3 = X1.i - X2.i;  	   
				 dCoss = dCosTable[dwPoint];
				 dSinn = dSinTable[dwPoint];
				 data[dwDataPos2].r = dReal3 * dCoss + dImag3 * dSinn;	  
				 data[dwDataPos2].i = dImag3 * dCoss - dReal3 * dSinn;
				 
				 dwDataPos1++;
				 dwDataPos2++;
				 dwPoint++;
			 }
		 }
		 
		 dwPart *=2;
	 }

	 // exchange elements
	 for(i = 0; i < dwFftPoint ;i++){
		 
		 dwDataPos1 = 0;
		 j = i;
		 for(k=0;k<dwMaxBit;k++){
			 dwDataPos1 <<= 1;
			 dwDataPos1 |= (j & 1);
			 j >>= 1;
		 }

		 // swap
		 if(dwDataPos1 > i){
			 swap = data[dwDataPos1];
			 data[dwDataPos1] = data[i];
			 data[i] = swap;
		 }
	 }
}



//------------------------------------------------------
// IFFT 
void CalcIfft(COMPLEX* data,// input and output
	 DWORD dwFftPoint, 
	 DWORD dwMaxBit,  
	 double* dCosTable,
	 double* dSinTable
	 ){
	 
	 DWORD dwStage,dwMaxStage;
	 DWORD dwPart;
	 DWORD dwDataHeight,dwDataHeight2;
	 
	 DWORD dwDataPos1,dwDataPos2,dwPoint;
	 
	 COMPLEX X1,X2;
	 double dReal3,dImag3;
	 
	 DWORD i,j,k;
	 double dSinn,dCoss;
	 COMPLEX swap;

	 dwPart = 1;
	 dwMaxStage = dwMaxBit;
	 for(dwStage = dwMaxStage ;dwStage>0; dwStage--)
	 {
		 dwDataHeight = (DWORD)1 << dwStage; // 2^dwStage 
		 dwDataHeight2 = dwDataHeight/2;

		 // butterfly
		 for(j = 0; j < dwPart ;j++){	
			 
			 dwDataPos1 = dwDataHeight*j;
			 dwDataPos2 = dwDataPos1+dwDataHeight2;
			 dwPoint = dwStage*dwFftPoint/2;
			 
			 for(k = 0 ; k <  dwDataHeight2 ; k++){
				 
				 X1 = data[dwDataPos1];	
				 X2 = data[dwDataPos2];	
				 
				 data[dwDataPos1].r = X1.r + X2.r; 
				 data[dwDataPos1].i = X1.i + X2.i; 
				 
				 dReal3 = X1.r - X2.r;	  
				 dImag3 = X1.i - X2.i;  	   
				 dCoss = dCosTable[dwPoint];
				 dSinn = dSinTable[dwPoint];
				 data[dwDataPos2].r = dReal3 * dCoss - dImag3 * dSinn;	  
				 data[dwDataPos2].i = dImag3 * dCoss + dReal3 * dSinn;
				 
				 dwDataPos1++;
				 dwDataPos2++;
				 dwPoint++;
			 }
		 }
		 
		 dwPart *=2; 
	 }

	 // exchange elements
	 for(i = 0; i < dwFftPoint ;i++){
		 
		 dwDataPos1 = 0;
		 j = i;
		 for(k=0;k<dwMaxBit;k++){
			 dwDataPos1 <<= 1;
			 dwDataPos1 |= (j & 1);
			 j >>= 1;
		 }

		 // swap
		 if(dwDataPos1 > i){
			 swap = data[dwDataPos1];
			 data[dwDataPos1] = data[i];
			 data[i] = swap;
		 }
	 }
}


//------------------------------------------------------
// prepare
void prepareFft(DWORD dwFftPoint,
				DWORD dwFFTnum
				){

	if(!FFT_cos[dwFFTnum]) FFT_cos[dwFFTnum] = (double*)malloc(sizeof(double)*(DWORD)((int)(log((double)dwFftPoint)/log(2.0)+1)*dwFftPoint/2) + 1024);
	if(!FFT_sin[dwFFTnum]) FFT_sin[dwFFTnum] = (double*)malloc(sizeof(double)*(DWORD)((int)(log((double)dwFftPoint)/log(2.0)+1)*dwFftPoint/2) + 1024);
	
	FFT_dwLn[dwFFTnum] = dwFftPoint;
	calcFftCosTable(FFT_cos[dwFFTnum],FFT_sin[dwFFTnum],&FFT_dwMaxBit[dwFFTnum],dwFftPoint);
}



//------------------------------------------------------
// unprepare
void unprepareFft(){

	DWORD i;
	
	for(i=0;i<FFT_NUM;i++){
		if(FFT_cos[i]) free(FFT_cos[i]);
		if(FFT_sin[i]) free(FFT_sin[i]);
		FFT_cos[i] = NULL;
		FFT_sin[i] = NULL;
	}
}



//------------------------------------------------------
// FFT (without normalize)
void calcFftFast(COMPLEX* data,
			 DWORD dwFFTnum
			 ){

	CalcFft(data,FFT_dwLn[dwFFTnum],FFT_dwMaxBit[dwFFTnum],FFT_cos[dwFFTnum],FFT_sin[dwFFTnum]);
}



//------------------------------------------------------
// IFFT 
void calcIfftFast(COMPLEX* data,
			 DWORD dwFFTnum
			 ){

	CalcIfft(data,FFT_dwLn[dwFFTnum],FFT_dwMaxBit[dwFFTnum],FFT_cos[dwFFTnum],FFT_sin[dwFFTnum]);
}

// EOF
