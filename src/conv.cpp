// convolution


#include <intrin.h>
#include <emmintrin.h>
#include <malloc.h>
#include <stdio.h>

double* CONV_A = NULL;
double* CONV_B = NULL;
double* CONV_C = NULL;
unsigned int CONV_dwLength;
bool CONV_SSE2 = false;  // SSE2 is supported



//----------------------------------------------
// convolution via SSE2
void conv_SSE2(const double* a,const double* b,double* c, unsigned int num)
{
	__m128d xmm0={0,0},xmm1={0,0},xmm2,xmm3,xmm4,xmm5;
	for( unsigned int i = 0; i < num; i+=4 ){

		xmm2 = _mm_load_pd(a+i);
		xmm3 = _mm_load_pd(b+i);
		xmm4 = _mm_load_pd(a+i+2);
		xmm5 = _mm_load_pd(b+i+2);
		xmm2 = _mm_mul_pd(xmm2,xmm3);
		xmm4 = _mm_mul_pd(xmm4,xmm5);
		xmm0 = _mm_add_pd(xmm0,xmm2);
		xmm1 = _mm_add_pd(xmm1,xmm4);
	}
	xmm0 = _mm_add_pd(xmm0,xmm1);
	_mm_store_pd(c,xmm0);
	
}


//----------------------------------
// check whether SSE2 is supported
// call this function to use SSE2
bool CheckSSE2(){

	int info[4];
	__cpuid(info,1);
	if( info[3] & (1<<26) ) CONV_SSE2 = true;
	else CONV_SSE2 = false;

	return CONV_SSE2;
}



//----------------------------------------------
// init
void InitCONV(unsigned int dwLength){

	if(CONV_SSE2){
		if(!CONV_A) CONV_A = (double*)_aligned_malloc(sizeof(double)*dwLength,16);
		if(!CONV_B) CONV_B = (double*)_aligned_malloc(sizeof(double)*dwLength,16);
		if(!CONV_C) CONV_C = (double*)_aligned_malloc(sizeof(double)*64,16);
	}
	else
	{
		if(!CONV_A) CONV_A = (double*)malloc(sizeof(double)*dwLength);
		if(!CONV_B) CONV_B = (double*)malloc(sizeof(double)*dwLength);
		if(!CONV_C) CONV_C = (double*)malloc(sizeof(double)*64);
		
	}
}


//----------------------------------------------
// unprepare
void UnprepareCONV(){
	
	if(CONV_SSE2){
		if(CONV_A) _aligned_free(CONV_A);
		if(CONV_B) _aligned_free(CONV_B);
		if(CONV_C) _aligned_free(CONV_C);
		CONV_A = NULL;
		CONV_B = NULL;
		CONV_C = NULL;
	}
	else
	{
		if(CONV_A) free(CONV_A);
		if(CONV_B) free(CONV_B);
		if(CONV_C) free(CONV_C);
		CONV_A = NULL;
		CONV_B = NULL;
		CONV_C = NULL;
	}
}


//----------------------------------------------
// set convolution size
void SetConvSize(unsigned int n){
	if(n % 2) n++;
	CONV_dwLength = n;
}


//----------------------------------------------
// get adr of buffer A
double* GetConvBufA(){
	if(CONV_A) memset(CONV_A,0,sizeof(double)*CONV_dwLength);
	return CONV_A;
}

//----------------------------------------------
// get adr of buffer B
double* GetConvBufB(){
	if(CONV_B) memset(CONV_B,0,sizeof(double)*CONV_dwLength);
	return CONV_B;
}



//----------------------------------------------
// convolution   axb
double CONV(){

	double out = 0;

	if(CONV_SSE2){ 
		conv_SSE2(CONV_A,CONV_B,CONV_C,CONV_dwLength);
		out = CONV_C[0] + CONV_C[1];
	}
	else
	{
		unsigned int i;

		out = 0;
		for(i=0;i<CONV_dwLength;i++) out += CONV_A[i]*CONV_B[i];
	}

	return out;
}
