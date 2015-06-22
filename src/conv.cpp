// convolution
//
// note: I used here inline assembler of VC++.

#include <malloc.h>

#include "filter.h"

double* CONV_A = NULL;
double* CONV_B = NULL;
double* CONV_C = NULL;
DWORD CONV_dwLength;
DWORD CONV_SSE2 = 0;  // SSE2 is supported



//----------------------------------------------
// convolution via SSE2
void conv_SSE2(const double* a,const double* b,const double* c,DWORD num){

	_asm{
		mov edi,num
		mov ecx,a // ecx = a
		mov ebx,b // ebx = b
		shr edi,1 // edi = num / 2

		xorpd xmm0,xmm0 // xmm0 = 0
L1:
		movapd	xmm1,[ecx]  // xmm1 = ecx
		movapd	xmm2,[ebx]  // xmm2 = ebx
		mulpd	xmm1,xmm2  // xmm1 *= xmm2
		addpd	xmm0,xmm1  // xmm0 += xmm1
		add ecx,16		   // ecx += sizeof(double)*2
		add ebx,16			// ebx += sizeof(double)*2

		dec edi // edi--
		jne	L1

		mov eax,c 
		movapd [eax],xmm0  // c = xmm0
	}
}



//--------------------------------------------------
// check whether SSE2 is supported from CPU ID
void CPUID_SSE2(LPDWORD lpdwSSE2){

	__asm{

		pushfd
		pop eax
		mov ecx,eax
		xor eax,(1<<21) 
		push eax
		popfd
		pushfd
		pop eax
		cmp eax,ecx 
		je EXIT	

		// get CPUID 
		xor eax,eax
		cpuid
		cmp eax,1 
		jb EXIT 
		mov eax,1
		cpuid

		// SSE2 = 26
		and edx,(1<<26) 
		mov ebx,lpdwSSE2
		mov dword ptr [ebx], edx
EXIT:
	}
}


//-------------------------------------------------------------------------


//----------------------------------
// check whether SSE2 is supported
// call this function to use SSE2
BOOL CheckSSE2(){

	CPUID_SSE2(&CONV_SSE2);

	return CONV_SSE2;
}



//----------------------------------------------
// init
void InitCONV(DWORD dwLength){

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
void SetConvSize(DWORD n){
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

	DWORD i;
	double out;

	if(CONV_SSE2){ 
		conv_SSE2(CONV_A,CONV_B,CONV_C,CONV_dwLength);
		out = CONV_C[0] + CONV_C[1];
	}
	else
	{
		out = 0;
		for(i=0;i<CONV_dwLength;i++) out += CONV_A[i]*CONV_B[i];
	}

	return out;
}

//EOF
