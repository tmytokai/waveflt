// body of filter

#include "filter.h"
#include "config.h"


//--------------------------------------------------------
// copy byte data to double data
void CopyBufferBtoD(BYTE* lpBuffer,  // input, buffer (BYTE*)
						DWORD dwByte, // size of lpBuffer

						double* lpFilterBuf[2], // output, buffer of wave data (double*) (L-R)
						LPDWORD lpdwPointsInBuf, // points of data in lpFilterBuf
						WAVEFORMATEX waveFmt
						)
{
	DWORD i,dwChn,dwPos; 
	BYTE* lpBuffer2;
	LONG nVal;
	DWORD dwFilterBufSize; 

	dwFilterBufSize = dwByte / waveFmt.nBlockAlign;
	
	if(waveFmt.wBitsPerSample == 16)
	{  // 16 bit
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(short)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((short*)lpBuffer2));
				dwPos++;
				lpBuffer2 += waveFmt.nBlockAlign;
			}

		}
	}
	else if(waveFmt.wBitsPerSample == 8)
	{ // 8 bit
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				lpFilterBuf[dwChn][dwPos] = (double)((int)lpBuffer[i+dwChn]-0x80);
				dwPos++;
			}
		}

	}
	else if(waveFmt.wBitsPerSample == 24)
	{  // 24 bit
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+3*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				nVal = 0;
				memcpy((LPBYTE)(&nVal)+1,lpBuffer2,3);
				nVal /= 256;

				lpFilterBuf[dwChn][dwPos] = (double)nVal;
				dwPos ++;
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}
	else if(waveFmt.wBitsPerSample == 32 && waveFmt.wFormatTag == WAVE_FORMAT_PCM)
	{  // 32 bit long
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(long)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((long*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}
	else if(waveFmt.wBitsPerSample == 32 && waveFmt.wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
	{  // 32 bit float
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(float)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((float*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}
	else if(waveFmt.wBitsPerSample == 64)
	{  // 64 bit(double)
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(double)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=waveFmt.nBlockAlign)
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((double*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}

	*lpdwPointsInBuf = dwByte/waveFmt.nBlockAlign;
}


//----------------------------
// copy double data to byte buffer (Quantization)
void CopyBufferDtoB(BYTE* lpBuffer,  // output, buffer(BYTE*)
						   
						   double* lpFilterBuf[2], // input, buffer(double*)
						   DWORD dwPointsInBuf, // points in lpFilterBuf
						   WAVEFORMATEX waveFmt,
   						   BOOL bRound,
						   
						   DWORD* lpdwSaturate  // number of saturation
						   )
{
	
	DWORD i,dwChn,dwPos; 
	BYTE* lpBuffer2;
	double dFoo,dFoo2;
	double dMaxLevel;
	long nOut;
	float fOut;
	double dRound;

	if(bRound) dRound = 0.5; else dRound = 0;
	
	if(waveFmt.wBitsPerSample == 64)
	{  // 64 bit  , no check saturation
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + waveFmt.wBitsPerSample/8 * dwChn;

			for(i=0;i< dwPointsInBuf;i++,dwPos++)
			{
				memcpy(lpBuffer2,lpFilterBuf[dwChn]+dwPos,sizeof(double));
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}
	else if(waveFmt.wBitsPerSample == 32 && waveFmt.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) 
	{ 	// 32bit float , no check saturation

		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + waveFmt.wBitsPerSample/8 * dwChn;

			for(i=0;i< dwPointsInBuf;i++,dwPos++)
			{
				fOut = (float)(lpFilterBuf[dwChn][dwPos]);
				memcpy(lpBuffer2,&fOut,sizeof(float));
				lpBuffer2 += waveFmt.nBlockAlign;
			}
		}
	}
	else  // 8,16,24,32-long
	{
		dMaxLevel = GetMaxWaveLevel(waveFmt);
		
		for(dwChn = 0 ; dwChn < waveFmt.nChannels  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + waveFmt.wBitsPerSample/8 *dwChn;

			for(i=0;i< dwPointsInBuf;i++,dwPos++)
			{
				// check saturation
				dFoo = lpFilterBuf[dwChn][dwPos];
				dFoo2 = fabs(dFoo);
				if(dFoo2 > dMaxLevel)
				{
					if(lpdwSaturate) (*lpdwSaturate)++;
					dFoo = (dFoo > 0) ? (dMaxLevel-1) : -(dMaxLevel-1);
				}

				// Quantization
				nOut = QUANTIZATION(dFoo,dRound);

				// copy data to buffer
				if(waveFmt.wBitsPerSample == 16) *((short*)lpBuffer2) = (short)nOut;
				else if(waveFmt.wBitsPerSample == 8) *lpBuffer2 = (BYTE)(nOut+0x80);
				else if(waveFmt.wBitsPerSample == 24) memcpy(lpBuffer2,&nOut,3);
				else if(waveFmt.wBitsPerSample == 32 && waveFmt.wFormatTag == WAVE_FORMAT_PCM) 
					memcpy(lpBuffer2,&nOut,sizeof(long)); // 32 bit long
				
				lpBuffer2 += waveFmt.nBlockAlign;
			}

			
		}
		
	}
	
}



//--------------------------------------
// clear all filters
void ClearAllFilters(){
	
	ClearCOMP();
	ClearAutoDCOffset();
	ClearFirBuf();
	ClearIirBuf();
	ClearMixFile();
	ClearNormal();
	ClearRsmp();
	ClearNOSOUND();
	AddSinCurve(NULL,0,0,0,0,NULL,NULL,NULL);
	ClearNGATE();
	ClearNSFBuf();
}


//--------------------------------------
// unprepare all filters
void UnprepareAllFilters()
{
	unprepareCOMP();
	UnprepareCONV();
	unprepareFIR();
	CloseMixFile();
	unprepareRsmp();
	unprepareNGATE();
	unprepareFft();
}



//------------------------------------
// initialize filters
BOOL InitFilters(LPFILTER_DATA lpFDat,
				 DWORD dwFilterPoints, // points of data in filter buffer
				 WAVEFORMATEX inWaveFmt,
				 WAVEFORMATEX outWaveFmt,
				 char* lpszErr){

	DWORD dwFoo,i;
	double dT1,dT2,alpha,dDb,dCutFreq;

	// FIR before nosound
	if(lpFDat->dwNoSndFIRFilter != NO_FILTER){
		for(i=0;i<inWaveFmt.nChannels;i++){
			prepareFIRCoefficient(lpFDat->dwNoSndFIRFilter,lpFDat->dwNoSndFIRleng,
				inWaveFmt.nSamplesPerSec,lpFDat->dNoSndFIRDb,lpFDat->dwNoSndFIRCutLow,
				lpFDat->dwNoSndFIRCutHigh,dwFilterPoints,ID_FIR_NOSND,i);
		}
	}
	
	// nosound
	if(lpFDat->dwNoSndMode != NOSND_NOT)
		PrepareNOSOUND(lpFDat->dwNoSndMode,lpFDat->dwNoSndSuspend,inWaveFmt.nSamplesPerSec);
	
	// FIR 
	if(lpFDat->dwFIRFilter != NO_FILTER){
		for(i=0;i<outWaveFmt.nChannels;i++){
			prepareFIRCoefficient(lpFDat->dwFIRFilter,lpFDat->dwFIRleng,
				inWaveFmt.nSamplesPerSec,lpFDat->dFIRDb,lpFDat->dwFIRCutLow,
				lpFDat->dwFIRCutHigh,dwFilterPoints,ID_FIR_NORMAL,i);
		}
	}
	
	// FIR-EQ
	if(lpFDat->bFIREQ){ 
		for(i=0;i<outWaveFmt.nChannels;i++){
			CalcEQCoefficient(lpFDat->dwEQleng,lpFDat->dwEQband,inWaveFmt.nSamplesPerSec,
				lpFDat->dEQDb,lpFDat->dEQLevel,lpFDat->dEQ_Q,
				dwFilterPoints,ID_FIR_EQ,i);
		}
	}

	// IIR 
	if(lpFDat->dwIIRFilter != NO_FILTER)
	{
		for(i=0;i<outWaveFmt.nChannels;i++){
		prepareIIRCoefficient(lpFDat->dwIIRFilter,
			(double)lpFDat->dwIIRCutLow,(double)lpFDat->dwIIRCutHigh,inWaveFmt.nSamplesPerSec,
			2,ID_IIR_NORMAL,i);
		}
	}
	

	// de-emphasis
	if(lpFDat->bDemp)
	{
		// t1 = 50 m = 1./(2.*PI*3180.)   
		// t2 = 15 m = 1./(2.*PI*10600.)
		dT1 = 0.000050;
		dT2 = 0.000015;
		dDb = 20*log10(dT2/dT1) * lpFDat->dDempDb;
		dCutFreq = 1./(2.*PI*dT1);
		for(i=0;i<outWaveFmt.nChannels;i++){
			CalcIirShelvingEQ(SVEQH,dCutFreq,dDb,inWaveFmt.nSamplesPerSec,ID_IIR_DEMP,i);
		}
	}

	// shelving EQ low
	if(lpFDat->bSVEQL)
	{
		for(i=0;i<outWaveFmt.nChannels;i++){
			CalcIirShelvingEQ(SVEQL,lpFDat->dwSVEQLfreq,lpFDat->dSVEQLdb,
				inWaveFmt.nSamplesPerSec,ID_IIR_SVEQL,i);
		}
	}

	// shelving EQ high
	if(lpFDat->bSVEQH)
	{

		dT2 = 1./(2.*PI*(double)lpFDat->dwSVEQHfreq);
		alpha = pow(10.,-fabs(lpFDat->dSVEQHdb)/20);
		dT1 =dT2/alpha;
		dCutFreq = 1./(2.*PI*dT1);
		for(i=0;i<outWaveFmt.nChannels;i++){
			CalcIirShelvingEQ(SVEQH,dCutFreq,lpFDat->dSVEQHdb,inWaveFmt.nSamplesPerSec,ID_IIR_SVEQH,i);
		}
	}

	// peaking EQ 
	if(lpFDat->bPKEQ)
	{
		for(i=0;i<outWaveFmt.nChannels;i++){
			CalcIirPeakingEQ((double)lpFDat->dwPKfreq,lpFDat->dPKQ,lpFDat->dPKdb,inWaveFmt.nSamplesPerSec,ID_IIR_PKEQ,i);
		}
	}

	// noisegate
	if(lpFDat->bNgate)
		prepareNGATE(outWaveFmt,lpFDat->dNgateTh,lpFDat->dNgateRelease,lpFDat->dNgateAttack,lpFDat->dwNgateRMS);

	// re-sampling
	if(lpFDat->bRsmp)
	{
		prepareRsmp(lpFDat->dwRsmpInFreq,lpFDat->dwRsmpOutFreq,lpFDat->dwRsmpCut,lpFDat->dRsmpDB,lpFDat->dwRsmpLn,lpFDat->dwRsmpUp,lpFDat->dwRsmpDown,
			dwFilterPoints,outWaveFmt.nChannels);
	}

	// file mixing
	if(lpFDat->bMixFile){
		if(!OpenMixFile(lpFDat->szMixFile,inWaveFmt,dwFilterPoints,lpFDat->dMixStartTime[1],lpszErr)){
			return false;
		}
	}

	// compressor
	if(lpFDat->bComp) prepareCOMP(outWaveFmt.nChannels,lpFDat->dwCompRMS,0);

	// limiter after compressor
	prepareCOMP(outWaveFmt.nChannels,lpFDat->dwNormalCompRMS,1);

	// init convolution
	dwFoo = dwFilterPoints;
	if(lpFDat->bRsmp) dwFoo *= 2; // size *= 2 when re-sampling
	InitCONV(dwFoo);

	return true;
}




//----------------------------------------------
// set default parameters 
void SetDefaultOption(LPFILTER_DATA lpFDat){
	
	memset(lpFDat,0,sizeof(FILTER_DATA));

	// FIR  before nosound
	lpFDat->dwNoSndFIRFilter = NO_FILTER;
	lpFDat->dwNoSndFIRleng = 63; // length
	lpFDat->dNoSndFIRDb = -100; // db, loss of filter

	// split at no sound 
	lpFDat->dwNoSndMode = NOSND_NOT; // split mode
	lpFDat->dwNoSndSuspend = 500; // msec. when split mode enter NOSND_EXEC, don't split file in dwNoSndSuspend msec.
	lpFDat->dwNoSndM1toM2 = 14400000; // msec. If time of no sound execeed this, change mode from NOSND_SEARCH to NOSND_CUT.
	lpFDat->dwNoSndRecStop = 14400000; // msec. If time of no sound execeed this, have LockOn stop recording.

	// FIR 
	lpFDat->dwFIRFilter = NO_FILTER;
	lpFDat->dwFIRleng = 63; // length
	lpFDat->dFIRDb = -100; // db, loss of filter

	// FIR-EQ
	lpFDat->dwEQband = 10;
	lpFDat->dwEQleng = 1023;
	lpFDat->dEQDb = -60;
	lpFDat->dEQ_Q = 1.5;

	// IIR
	lpFDat->dwIIRFilter = NO_FILTER;
	
	// volume
	lpFDat->dVolume = 1.0; 
	
	// compressor atter normaizing
	lpFDat->dNormalRatio = 100; // ratio
	lpFDat->dNormalTh = -2; // threshold (db)
	lpFDat->dNormalAttack = 5; // attack time (msec)
	lpFDat->dNormalRelease = 100; // release time (msec)
	
	// re-sampling
	lpFDat->dwRsmpInFreq = 48000; // input freq
	lpFDat->dwRsmpOutFreq = 44100; // output freq
	lpFDat->dwRsmpLn = 32767; // length
	lpFDat->dwRsmpCut = 21200; // cut-off freq
	lpFDat->dRsmpDB = -150; // loss of filter
	
	// bit 
	lpFDat->dwBitChangeTag = WAVE_FORMAT_PCM; 
}



//-------------------------------------------------------
// body of filter
void WFLT_FILTER(LPFILTER_DATA lpFDat,  // parameter

				 double* lpFilterBuf[2],  // (input / output) buffer of wave data
				 LPDWORD lpdwPointsInBuf, // (input / output) data points in buffer
				 LPDWORD lpdwRealPointsInBuf, // (output) output data points
				 // note: if inWaveFmt.nSamplesPerSec = outWaveFmt.nSamplesPerSec,
				 // then *lpdwPointsInBuf = *lpdwRealPointsInBuf.

				 BOOL* lpbChangeFile, // if return value of *lpbChangeFile is true, change output file
				 DWORD dwCurrentNormalMode, // current mode of normalizer
				 double dNormalGain[2], // L-R, gain for normalizer
				 DWORD dwCurFileNo,  // current file number
				 LONGLONG n64OutSize, // byte, total output size
				 LONGLONG n64DataSize, // byte, total data size of wave
				 WAVEFORMATEX inWaveFmt, // format of input
				 WAVEFORMATEX outWaveFmt // format of output
				 ){
	
	DWORD i,i2,dwFoo;
	DWORD dwPointsInBufBeforeSplit; // for RewindBufFIR()
	BOOL bChangeFile = false;
	static double dNoiseShape[MAX_CHN]; // buffer of noise shaper

	// adjust DC offset 
	if(lpFDat->bOffset){
		for(i=0;i<inWaveFmt.nChannels;i++)
			DCOFFSET(lpFilterBuf[i],*lpdwPointsInBuf,lpFDat->dOffset[i]);
	}
	
	// pre-normalize data between -1 to 1 before filtering
	if( CONFIG::get().pre_normalization ){
		const double maxlevel = GetMaxWaveLevel(inWaveFmt);
		for(i=0;i<inWaveFmt.nChannels;i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) lpFilterBuf[i][i2] /= maxlevel;
		}
	}
	
	// auto adjust DC offset
	if(lpFDat->bAutoOffset)
		AUTODCOFFSET(lpFilterBuf,*lpdwPointsInBuf,inWaveFmt,lpFDat->dwAutoOffsetTime);
	
	// FIR before nosound
	if(lpFDat->dwNoSndFIRFilter != NO_FILTER){
		for(i=0;i<inWaveFmt.nChannels;i++)
			FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_NOSND,i);
	}	

	dwPointsInBufBeforeSplit = *lpdwPointsInBuf;

	// split at no sound part, or delete no sound part in head
	if(lpFDat->dwNoSndMode != NOSND_NOT)
		NOSOUND(
		lpFilterBuf,
		lpdwPointsInBuf,
		&bChangeFile,
		inWaveFmt.nSamplesPerSec,
		inWaveFmt.nChannels,
		
		lpFDat->dwNoSndTime,
		lpFDat->dwNoSndM1toM2,
		lpFDat->dwNoSndRecStop,
		
		lpFDat->dNoSndBoundHead,
		lpFDat->dNoSndBound
		);	

	// if the splitting has been occured in NOSOUND already, (i.e. bChangeFile = true), 
	// rewind buffer of FIR filter, or the noise appears in the head of next file.
	if(bChangeFile && lpFDat->dwNoSndFIRFilter != NO_FILTER){
			dwFoo = dwPointsInBufBeforeSplit - *lpdwPointsInBuf;//  size of rewind
			for(i=0;i<inWaveFmt.nChannels;i++) RewindBufFIR(dwFoo,ID_FIR_NOSND,i);
	}
	
	// split at specified time or size of output data
	// if splitting has been occured in NOSOUND, don't split again.
	if(lpFDat->bSplit && bChangeFile == false)  
		SPLIT(inWaveFmt,lpdwPointsInBuf,
		n64OutSize,dwCurFileNo,&bChangeFile,
		&lpFDat->n64SplitByte,
		
		lpFDat->dSplitTime,
		lpFDat->n64SplitByteMalti);

	if(lpbChangeFile) *lpbChangeFile = bChangeFile;
	
	// balance
	if(lpFDat->bBalance){
		for(i=0;i<inWaveFmt.nChannels;i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= lpFDat->dBalance[i];
		}
	}	


	// file mixing
	if(lpFDat->bMixFile){ 
		MixFile(inWaveFmt,lpFilterBuf,
			*lpdwPointsInBuf,n64OutSize,
			lpFDat->dMixLevel,lpFDat->dMixStartTime[0]);
	}
	
	
	// LR-mixing
	if(lpFDat->bMixLR){
		for(i=0;i<*lpdwPointsInBuf;i++){
			lpFilterBuf[0][i] = 
				lpFilterBuf[0][i]*lpFDat->dMixLRLevel+lpFilterBuf[1][i]*(1-lpFDat->dMixLRLevel);
		}
	}
	
	//---------------------------------------------------
	// notice: 
	// If LR-mixing has been executed, number of channels become 1/2 from here,
	// so use 'outWaveFmt.nChannels' instead of 'inWaveFmt.nChannels'

	// synthesize sine waves
	if(lpFDat->dwAddSinNum){
		for(i=0;i<outWaveFmt.nChannels;i++)
			AddSinCurve(lpFilterBuf[i],
			*lpdwPointsInBuf,inWaveFmt.nSamplesPerSec,i,
			lpFDat->dwAddSinNum,lpFDat->dwAddSinFreq,
			lpFDat->dAddSinDb,lpFDat->dwAddSinPhase);
	}
	
	// noise gate
	if(lpFDat->bNgate)
		NOISEGATE_FREQ(lpFilterBuf,*lpdwPointsInBuf,outWaveFmt.nChannels);	

	// FIR
	if(lpFDat->dwFIRFilter != NO_FILTER){
		for(i=0;i<outWaveFmt.nChannels;i++)
			FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_NORMAL,i);
	}

	// FIR-EQ
	if(lpFDat->bFIREQ){
		for(i=0;i<outWaveFmt.nChannels;i++) FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_EQ,i);
	}

	// IIR 
	if(lpFDat->dwIIRFilter != NO_FILTER) for(i=0;i<outWaveFmt.nChannels;i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_NORMAL,i);


	// (de)emphasis
	if(lpFDat->bDemp) for(i=0;i<outWaveFmt.nChannels;i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_DEMP,i);

	// shelving EQ low
	if(lpFDat->bSVEQL) for(i=0;i<outWaveFmt.nChannels;i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_SVEQL,i);

	// shelving EQ high
	if(lpFDat->bSVEQH) for(i=0;i<outWaveFmt.nChannels;i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_SVEQH,i);

	// peaking EQ high
	if(lpFDat->bPKEQ) for(i=0;i<outWaveFmt.nChannels;i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_PKEQ,i);

	// phase invert
	if(lpFDat->bPhaseInv){
		for(i=0;i<outWaveFmt.nChannels;i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] = - lpFilterBuf[i][i2];
		}
	}

	// compressor
	if(lpFDat->bComp){
		COMPRESS(inWaveFmt.nSamplesPerSec,lpFilterBuf,*lpdwPointsInBuf,outWaveFmt.nChannels,
			lpFDat->dCompTh,lpFDat->dCompRatio,
			lpFDat->dCompAttack,lpFDat->dCompRelease,0);
	}

	// volume
	if(lpFDat->dVolume != 1.0){
		for(i=0;i<outWaveFmt.nChannels;i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= lpFDat->dVolume;
		}
	}				

	// fade in/out
	for(i=0;i<outWaveFmt.nChannels;i++)
		FADEINOUT(lpFilterBuf[i],*lpdwPointsInBuf,

		inWaveFmt,lpFDat->dwFadeIn,lpFDat->dwFadeOut,
		n64OutSize,n64DataSize
		);

	*lpdwRealPointsInBuf = *lpdwPointsInBuf;

	// down sampling(48k -> 44.1k)
	if(lpFDat->bRsmp){
		for(i=0;i<outWaveFmt.nChannels;i++)
			RSAMP(lpFilterBuf[i],*lpdwPointsInBuf,i,lpdwRealPointsInBuf);
	}

	// notice:
	// sampling rate of output is changed when re-sampling has been executed, so
	// use '*lpdwRealPointsInBuf' instead of '*lpdwPointsInBuf', and
	// use 'outWaveFmt' instead of 'inWaveFmt' from here.

	// calclate RMS or average for normalizer
	if(dwCurrentNormalMode == NORMAL_AVG){
		for(i=0;i<outWaveFmt.nChannels;i++) 
			SET_AVG(lpFilterBuf[i],*lpdwRealPointsInBuf,i);			
	}

	if(dwCurrentNormalMode == NORMAL_RMS){
		for(i=0;i<outWaveFmt.nChannels;i++) 
			SET_RMS(lpFilterBuf[i],*lpdwRealPointsInBuf,i);			
	}

	// normalizer
	if(dwCurrentNormalMode == NORMAL_EXEC){

		for(i=0;i<outWaveFmt.nChannels;i++){
			for(i2=0;i2<*lpdwRealPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= dNormalGain[i];
		}

		// compressor(limiter)
		if(lpFDat->bNormalUseCompressor)
			COMPRESS(outWaveFmt.nSamplesPerSec,lpFilterBuf,*lpdwRealPointsInBuf,outWaveFmt.nChannels,
			lpFDat->dNormalTh,lpFDat->dNormalRatio,
			lpFDat->dNormalAttack,lpFDat->dNormalRelease,1);
	}

	// search peak,
	for(i=0;i<outWaveFmt.nChannels;i++) SET_PEAK(lpFilterBuf[i],*lpdwRealPointsInBuf,i);			

	// restore wave level
	if( CONFIG::get().pre_normalization ){
		if(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)
		{
			const double maxlevel = GetMaxWaveLevel(outWaveFmt);
			for(i=0;i<outWaveFmt.nChannels;i++){
				for(i2=0;i2<*lpdwRealPointsInBuf;i2++) lpFilterBuf[i][i2] *= maxlevel;
			}
		}
	}

	// dither
	if(lpFDat->bDither){
		if(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC)
		{		
			for(i=0;i<outWaveFmt.nChannels;i++)
			{
				DITHER(lpFilterBuf[i],*lpdwRealPointsInBuf,i,lpFDat->dDitherAmp);
			}
		}

	}
}
