// body of filter

#include <vector>
#include <string>

#include "waveflt.h"

#include "config.h"
#include "wave.h"
#include "io.h"

#include "filter.h"
#include "trackmanager.h"
#include "dcoffset.h"

std::vector<Filter*> filters;


//----------------------------------------------------
// global 

char argv[MAX_ARGC][CHR_BUF],szCmdLine[MAX_WAVFLTOPTBUF];
WaveFormat InputWaveFmt; // format of input
WaveFormat WriteWaveFmt; // format of output
char SzReadFile[MAX_PATH]; // input file name
char SzOrgWriteFile[MAX_PATH]; // original output file name specified in command line
char SzWriteFile[MAX_PATH]; // (virtual) output file name
char SzRealWriteFile[MAX_PATH]; // (actual) output file name 
char SzCurDir[MAX_PATH]; // current directry
unsigned long long N64FileOffset; // byte, header offset of input file
unsigned long long N64FileDataSize; // byte, data size of input file
DWORD DwBufSize = 10; // buffer size
BOOL BlHeadOffset = false; // specify header offset of input file 
BOOL BlNoChkHdd = false; // no check space of HDD
BOOL BlWaveHdrOut = true; // output wave header
//bool BlExtChunkOfHdr = true; // use extra wave header
BOOL BlUseSSE2 = true; // use sse2

LONGLONG N64OutOffset; // byte, offset of input file

BOOL BlStdin = false;  // read from stdin
BOOL BlStdout = false;  // write to stdout
bool BlVerbose = true; // verbose
BOOL BlEndless = false; // endless mode
char SzUserDef[3][CHR_BUF]; // user defined strings
SYSTEMTIME SystemTime; // system time

// config of blocks
unsigned long long N64TotalDataSize; // total data size of output
unsigned long long N64RealTotalDataSize; // total data size of 'real' output file
double DbStartTime[MAXCOPYBLOCK]; // sec, start time of each block
double DbEndTime[MAXCOPYBLOCK]; // sec, end time of each bloc
BOOL BlCutTail = false;  // specified -cuttail option
BOOL BlCutFile = false;  // execute cutting file


DWORD DwCopyBlock = 1; // number of blocks
unsigned long long N64OffsetBlk[MAXCOPYBLOCK]; // byte, offset of each block
unsigned long long N64DataSizeBlk[MAXCOPYBLOCK]; //  byte, copy size of each block


std::vector<BlockData> blockdata;



// shift output file
double DbShiftTime = 0; // msec


// saturation
DWORD DwSaturate = 0; // number of saturation

// normalize
DWORD DwNormalMode = NORMAL_NOT; // mode
double DbNormalLevel = 0; // db, level
BOOL BlNormalStereo = true; // stereo link

// play sound (waveout)
BOOL BlWaveOut = false;
UINT UnWaveDevID = -1;  // WAVE_MAPPER

// text output
BOOL BlTextOut = false;

// print information of input file
BOOL BlFileInfo = false;

// delete input file
BOOL BlDelFile = false;

// add no sound part
double DbAddSp[2] = {0,0};
DWORD DwAddSp[2] = {0,0};

// input is 'nosound'
BOOL BlNoSignal = false;

// print characteristics of resampling
BOOL BlRsmpOutFilter; 

// print characteristics of FIR filter
BOOL BlFIROutFilter;  

// print characteristics of IIR filter
BOOL BlIIROutFilter;  


// time of fade in/out
double DbFadeInTime; // sec
double DbFadeOutTime; // sec


BOOL BlOutFile;  // use -outfile option
char SzOutFile[MAX_PATH];
#ifdef WIN32
HANDLE HdOutFileMap;  // use filemapping for config of the output file name
#endif

// current file number
DWORD DwCurSplitNo = 0; 

/* obsolete
// set filestamp to creation time
BOOL BlFileStamp = false;
#ifdef USEWIN32API
FILETIME FtCreationTime,FtLastAccessTime,FtLastWriteTime;
#endif
*/

FILTER_DATA FDAT;

/*
//--------------------------------------------------------
// copy byte data to double data
void CopyBufferBtoD(BYTE* lpBuffer,  // input, buffer (BYTE*)
						DWORD dwByte, // size of lpBuffer

						double* lpFilterBuf[2], // output, buffer of wave data (double*) (L-R)
						unsigned int* lpdwPointsInBuf, // points of data in lpFilterBuf
						WaveFormat format
						)
{
	DWORD i,dwChn,dwPos; 
	BYTE* lpBuffer2;
	LONG nVal;
	DWORD dwFilterBufSize; 
	
	dwFilterBufSize = dwByte / format.block();
	
	if(format.bits() == 16)
	{  // 16 bit
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(short)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((short*)lpBuffer2));
				dwPos++;
				lpBuffer2 += format.block();
			}

		}
	}
	else if(format.bits() == 8)
	{ // 8 bit
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				lpFilterBuf[dwChn][dwPos] = (double)((int)lpBuffer[i+dwChn]-0x80);
				dwPos++;
			}
		}

	}
	else if(format.bits() == 24)
	{  // 24 bit
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+3*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				nVal = 0;
				memcpy((LPBYTE)(&nVal)+1,lpBuffer2,3);
				nVal /= 256;

				lpFilterBuf[dwChn][dwPos] = (double)nVal;
				dwPos ++;
				lpBuffer2 += format.block();
			}
		}
	}
	else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_PCM)
	{  // 32 bit long
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(long)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((long*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += format.block();
			}
		}
	}
	else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_IEEE_FLOAT)
	{  // 32 bit float
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(float)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((float*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += format.block();
			}
		}
	}
	else if(format.bits() == 64)
	{  // 64 bit(double)
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer+sizeof(double)*dwChn;
			memset(lpFilterBuf[dwChn],0,sizeof(double)*dwFilterBufSize);
			
			for(i=0;i< dwByte;i+=format.block())
			{
				lpFilterBuf[dwChn][dwPos] = (double)(*((double*)lpBuffer2));
				dwPos ++;
				lpBuffer2 += format.block();
			}
		}
	}

	*lpdwPointsInBuf = dwByte/format.block();
}
*/


//----------------------------
// copy double data to byte buffer (Quantization)
void CopyBufferDtoB(BYTE* lpBuffer,  // output, buffer(BYTE*)
						   
						   double* lpFilterBuf[2], // input, buffer(double*)
						   DWORD dwPointsInBuf, // points in lpFilterBuf
						   WaveFormat format,
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
	
	if(format.bits() == 64)
	{  // 64 bit  , no check saturation
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + format.bits()/8 * dwChn;

			for(i=0;i< dwPointsInBuf;i++,dwPos++)
			{
				memcpy(lpBuffer2,lpFilterBuf[dwChn]+dwPos,sizeof(double));
				lpBuffer2 += format.block();
			}
		}
	}
	else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_IEEE_FLOAT) 
	{ 	// 32bit float , no check saturation

		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + format.bits()/8 * dwChn;

			for(i=0;i< dwPointsInBuf;i++,dwPos++)
			{
				fOut = (float)(lpFilterBuf[dwChn][dwPos]);
				memcpy(lpBuffer2,&fOut,sizeof(float));
				lpBuffer2 += format.block();
			}
		}
	}
	else  // 8,16,24,32-long
	{
		dMaxLevel = format.GetMaxWaveLevel();
		
		for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
		{
			dwPos = 0;
			lpBuffer2 = lpBuffer + format.bits()/8 *dwChn;

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
				if(format.bits() == 16) *((short*)lpBuffer2) = (short)nOut;
				else if(format.bits() == 8) *lpBuffer2 = (BYTE)(nOut+0x80);
				else if(format.bits() == 24) memcpy(lpBuffer2,&nOut,3);
				else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_PCM) 
					memcpy(lpBuffer2,&nOut,sizeof(long)); // 32 bit long
				
				lpBuffer2 += format.block();
			}

			
		}
		
	}
	
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

	std::vector<Filter*>::iterator it = filters.begin();
	for( ; it != filters.end(); ++it ){
		delete (*it);
	}
}



//------------------------------------
// initialize filters
BOOL InitFilters(LPFILTER_DATA lpFDat,
				 DWORD dwFilterPoints, // points of data in filter buffer
				 WaveFormat informat,
				 WaveFormat outformat,
				 char* lpszErr){

	DWORD dwFoo,i;
	double dT1,dT2,alpha,dDb,dCutFreq;

	// FIR before nosound
	if(lpFDat->dwNoSndFIRFilter != NO_FILTER){
		for(i=0;i<informat.channels();i++){
			prepareFIRCoefficient(lpFDat->dwNoSndFIRFilter,lpFDat->dwNoSndFIRleng,
				informat.rate(),lpFDat->dNoSndFIRDb,lpFDat->dwNoSndFIRCutLow,
				lpFDat->dwNoSndFIRCutHigh,dwFilterPoints,ID_FIR_NOSND,i);
		}
	}
	
	// nosound
	if(lpFDat->dwNoSndMode != NOSND_NOT)
		PrepareNOSOUND(lpFDat->dwNoSndMode,lpFDat->dwNoSndSuspend,informat.rate());
	
	// FIR 
	if(lpFDat->dwFIRFilter != NO_FILTER){
		for(i=0;i<outformat.channels();i++){
			prepareFIRCoefficient(lpFDat->dwFIRFilter,lpFDat->dwFIRleng,
				informat.rate(),lpFDat->dFIRDb,lpFDat->dwFIRCutLow,
				lpFDat->dwFIRCutHigh,dwFilterPoints,ID_FIR_NORMAL,i);
		}
	}
	
	// FIR-EQ
	if(lpFDat->bFIREQ){ 
		for(i=0;i<outformat.channels();i++){
			CalcEQCoefficient(lpFDat->dwEQleng,lpFDat->dwEQband,informat.rate(),
				lpFDat->dEQDb,lpFDat->dEQLevel,lpFDat->dEQ_Q,
				dwFilterPoints,ID_FIR_EQ,i);
		}
	}

	// IIR 
	if(lpFDat->dwIIRFilter != NO_FILTER)
	{
		for(i=0;i<outformat.channels();i++){
		prepareIIRCoefficient(lpFDat->dwIIRFilter,
			(double)lpFDat->dwIIRCutLow,(double)lpFDat->dwIIRCutHigh,informat.rate(),
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
		dCutFreq = 1./(2.*M_PI*dT1);
		for(i=0;i<outformat.channels();i++){
			CalcIirShelvingEQ(SVEQH,dCutFreq,dDb,informat.rate(),ID_IIR_DEMP,i);
		}
	}

	// shelving EQ low
	if(lpFDat->bSVEQL)
	{
		for(i=0;i<outformat.channels();i++){
			CalcIirShelvingEQ(SVEQL,lpFDat->dwSVEQLfreq,lpFDat->dSVEQLdb,
				informat.rate(),ID_IIR_SVEQL,i);
		}
	}

	// shelving EQ high
	if(lpFDat->bSVEQH)
	{

		dT2 = 1./(2.*M_PI*(double)lpFDat->dwSVEQHfreq);
		alpha = pow(10.,-fabs(lpFDat->dSVEQHdb)/20);
		dT1 =dT2/alpha;
		dCutFreq = 1./(2.*M_PI*dT1);
		for(i=0;i<outformat.channels();i++){
			CalcIirShelvingEQ(SVEQH,dCutFreq,lpFDat->dSVEQHdb,informat.rate(),ID_IIR_SVEQH,i);
		}
	}

	// peaking EQ 
	if(lpFDat->bPKEQ)
	{
		for(i=0;i<outformat.channels();i++){
			CalcIirPeakingEQ((double)lpFDat->dwPKfreq,lpFDat->dPKQ,lpFDat->dPKdb,informat.rate(),ID_IIR_PKEQ,i);
		}
	}

	// noisegate
	if(lpFDat->bNgate)
		prepareNGATE(outformat,lpFDat->dNgateTh,lpFDat->dNgateRelease,lpFDat->dNgateAttack,lpFDat->dwNgateRMS);

	// re-sampling
	if(lpFDat->bRsmp)
	{
		prepareRsmp(lpFDat->dwRsmpInFreq,lpFDat->dwRsmpOutFreq,lpFDat->dwRsmpCut,lpFDat->dRsmpDB,lpFDat->dwRsmpLn,lpFDat->dwRsmpUp,lpFDat->dwRsmpDown,
			dwFilterPoints,outformat.channels());
	}

	// file mixing
	if(lpFDat->bMixFile){
		if(!OpenMixFile(lpFDat->szMixFile,informat,dwFilterPoints,lpFDat->dMixStartTime[1],lpszErr)){
			return false;
		}
	}

	// compressor
	if(lpFDat->bComp) prepareCOMP(outformat.channels(),lpFDat->dwCompRMS,0);

	// limiter after compressor
	prepareCOMP(outformat.channels(),lpFDat->dwNormalCompRMS,1);

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
// in case of Ctrl+C 
#ifdef USEWIN32API
BOOL WINAPI ExitCtrlC(DWORD dwCtrlType){
#else
void ExitCtrlC(int nRet){
#endif
	
#ifdef USEWIN32API
	// close wave device
	CloseWaveDevice(true);
	/* obsolete
	StopChildStop(hProcessInfo,BlCreatePipe);
	*/
#endif	
	
	
#ifdef USEWIN32API
	fprintf(stderr,"\nstop (waveflt)\n");
	return false;
#else
	fprintf(stderr,"\n\nstop.(%d)(waveflt)\n",nRet);
	exit(nRet);
#endif
}
	

//-------------------------------------------------------
// body of filter
void WFLT_FILTER(LPFILTER_DATA lpFDat,  // parameter

				 double* lpFilterBuf[2],  // (input / output) buffer of wave data
				 unsigned int* lpdwPointsInBuf, // (input / output) data points in buffer
				 unsigned int& points_before_resampling, 

				 BOOL* lpbChangeFile, // if return value of *lpbChangeFile is true, change output file
				 DWORD dwCurrentNormalMode, // current mode of normalizer
				 double dNormalGain[2], // L-R, gain for normalizer
				 DWORD dwCurFileNo,  // current file number
				 LONGLONG total_out_size, // byte, total output size
				 LONGLONG n64TotalDataSize, // byte, total data size of wave
				 WaveFormat informat, // format of input
				 WaveFormat outformat // format of output
				 ){
	
	DWORD i,i2,dwFoo;
	DWORD dwPointsInBufBeforeSplit; // for RewindBufFIR()
	BOOL bChangeFile = false;
	static double dNoiseShape[MAX_CHN]; // buffer of noise shaper

	// pre-normalize data between -1 to 1 before filtering
	if( CONFIG::get().pre_normalization ){
		const double maxlevel = informat.GetMaxWaveLevel();
		for(i=0;i<informat.channels();i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) lpFilterBuf[i][i2] /= maxlevel;
		}
	}
	
	// auto adjust DC offset
	if(lpFDat->bAutoOffset)
		AUTODCOFFSET(lpFilterBuf,*lpdwPointsInBuf,informat,lpFDat->dwAutoOffsetTime);
	
	// FIR before nosound
	if(lpFDat->dwNoSndFIRFilter != NO_FILTER){
		for(i=0;i<informat.channels();i++)
			FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_NOSND,i);
	}	

	dwPointsInBufBeforeSplit = *lpdwPointsInBuf;

	// split at no sound part, or delete no sound part in head
	if(lpFDat->dwNoSndMode != NOSND_NOT)
		NOSOUND(
		lpFilterBuf,
		lpdwPointsInBuf,
		&bChangeFile,
		informat.rate(),
		informat.channels(),
		
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
			for(i=0;i<informat.channels();i++) RewindBufFIR(dwFoo,ID_FIR_NOSND,i);
	}
/*	
	// split at specified time or size of output data
	// if splitting has been occured in NOSOUND, don't split again.
	if(lpFDat->bSplit && bChangeFile == false)  
		SPLIT(informat,lpdwPointsInBuf,
		total_out_size,dwCurFileNo,&bChangeFile,
		&lpFDat->n64SplitByte,
		
		lpFDat->dSplitTime,
		lpFDat->n64SplitByteMalti);

	if(lpbChangeFile) *lpbChangeFile = bChangeFile;
*/	
	// balance
	if(lpFDat->bBalance){
		for(i=0;i<informat.channels();i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= lpFDat->dBalance[i];
		}
	}	


	// file mixing
	if(lpFDat->bMixFile){ 
		MixFile(informat,lpFilterBuf,
			*lpdwPointsInBuf,total_out_size,
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
	// so use 'outformat.channels' instead of 'informat.channels'

	// synthesize sine waves
	if(lpFDat->dwAddSinNum){
		for(i=0;i<outformat.channels();i++)
			AddSinCurve(lpFilterBuf[i],
			*lpdwPointsInBuf,informat.rate(),i,
			lpFDat->dwAddSinNum,lpFDat->dwAddSinFreq,
			lpFDat->dAddSinDb,lpFDat->dwAddSinPhase);
	}
	
	// noise gate
	if(lpFDat->bNgate)
		NOISEGATE_FREQ(lpFilterBuf,*lpdwPointsInBuf,outformat.channels());	

	// FIR
	if(lpFDat->dwFIRFilter != NO_FILTER){
		for(i=0;i<outformat.channels();i++)
			FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_NORMAL,i);
	}

	// FIR-EQ
	if(lpFDat->bFIREQ){
		for(i=0;i<outformat.channels();i++) FIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_FIR_EQ,i);
	}

	// IIR 
	if(lpFDat->dwIIRFilter != NO_FILTER) for(i=0;i<outformat.channels();i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_NORMAL,i);


	// (de)emphasis
	if(lpFDat->bDemp) for(i=0;i<outformat.channels();i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_DEMP,i);

	// shelving EQ low
	if(lpFDat->bSVEQL) for(i=0;i<outformat.channels();i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_SVEQL,i);

	// shelving EQ high
	if(lpFDat->bSVEQH) for(i=0;i<outformat.channels();i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_SVEQH,i);

	// peaking EQ high
	if(lpFDat->bPKEQ) for(i=0;i<outformat.channels();i++)
		IIR(lpFilterBuf[i],*lpdwPointsInBuf,ID_IIR_PKEQ,i);

	// phase invert
	if(lpFDat->bPhaseInv){
		for(i=0;i<outformat.channels();i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] = - lpFilterBuf[i][i2];
		}
	}

	// compressor
	if(lpFDat->bComp){
		COMPRESS(informat.rate(),lpFilterBuf,*lpdwPointsInBuf,outformat.channels(),
			lpFDat->dCompTh,lpFDat->dCompRatio,
			lpFDat->dCompAttack,lpFDat->dCompRelease,0);
	}

	// volume
	if(lpFDat->dVolume != 1.0){
		for(i=0;i<outformat.channels();i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= lpFDat->dVolume;
		}
	}				

	// fade in/out
	for(i=0;i<outformat.channels();i++)
		FADEINOUT(lpFilterBuf[i],*lpdwPointsInBuf,

		informat,lpFDat->dwFadeIn,lpFDat->dwFadeOut,
		total_out_size,n64TotalDataSize
		);

	// down sampling(48k -> 44.1k)
	points_before_resampling = *lpdwPointsInBuf;
	if(lpFDat->bRsmp){
		unsigned int points_tmp;
		for(i=0;i<outformat.channels();i++)
			RSAMP(lpFilterBuf[i],*lpdwPointsInBuf,i,&points_tmp);
		*lpdwPointsInBuf = points_tmp;
	}

	// calclate RMS or average for normalizer
	if(dwCurrentNormalMode == NORMAL_AVG){
		for(i=0;i<outformat.channels();i++) 
			SET_AVG(lpFilterBuf[i],*lpdwPointsInBuf,i);			
	}

	if(dwCurrentNormalMode == NORMAL_RMS){
		for(i=0;i<outformat.channels();i++) 
			SET_RMS(lpFilterBuf[i],*lpdwPointsInBuf,i);			
	}

	// normalizer
	if(dwCurrentNormalMode == NORMAL_EXEC){

		for(i=0;i<outformat.channels();i++){
			for(i2=0;i2<*lpdwPointsInBuf;i2++) 
				lpFilterBuf[i][i2] *= dNormalGain[i];
		}

		// compressor(limiter)
		if(lpFDat->bNormalUseCompressor)
			COMPRESS(outformat.rate(),lpFilterBuf,*lpdwPointsInBuf,outformat.channels(),
			lpFDat->dNormalTh,lpFDat->dNormalRatio,
			lpFDat->dNormalAttack,lpFDat->dNormalRelease,1);
	}

	// search peak,
	for(i=0;i<outformat.channels();i++) SET_PEAK(lpFilterBuf[i],*lpdwPointsInBuf,i);			

	if(dwCurrentNormalMode == NORMAL_NOT || dwCurrentNormalMode == NORMAL_EXEC){

		// restore wave level
		if( CONFIG::get().pre_normalization ){
			const double maxlevel = outformat.GetMaxWaveLevel();
			for(i=0;i<outformat.channels();i++){
				for(i2=0;i2<*lpdwPointsInBuf;i2++) lpFilterBuf[i][i2] *= maxlevel;
			}
		}

		// dither
		if(lpFDat->bDither){
			for(i=0;i<outformat.channels();i++){
				DITHER(lpFilterBuf[i],*lpdwPointsInBuf,i,lpFDat->dDitherAmp);
			}
		}
	}
}


//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------


//----------------------------------------------
// read option
BOOL ReadOption(int argc, char *argv_org[]){
	
	DWORD i,i2,i3,i4;
	double dFoo,dFoo2;
	int argc2;
	char szErr[CHR_BUF];
	
	if(argc < 3){
		ShowAbout();
		exit(1);
	}

	// initilize parameters
	SetDefaultOption(&FDAT);
	CONFIG::reset();

	//  Get Current Directory
	GetCurrentDirectory(MAX_PATH,SzCurDir); 
	fprintf(stderr,"current directory: %s\n",SzCurDir);
	wsprintf(SzCurDir,"%s\\",SzCurDir); // add '\'

	memset(SzUserDef,0,sizeof(char)*3*CHR_BUF);
	InputWaveFmt.clear();
	WriteWaveFmt.clear();
	InputWaveFmt.set(WAVE_FORMAT_PCM, 2,44100,16);
	N64FileDataSize = 180 * InputWaveFmt.avgbyte();

	// read config from file mapping or configuration file
	argc2 = 0; 
	for(i=0; i <(unsigned int)argc; i++, argc2++){

#ifdef USEWIN32API
		if(strcmp(argv_org[i],"-fmap")==0){ // file mapping

			if(!GetFileMappingData(argc,argv_org,szCmdLine,szErr)){
				fprintf(stderr,szErr);
				return false;
			}
			argc2 += (GetArgv(szCmdLine,argv+argc2,0)-1);
			argc--;
		}
		else 
#endif
			if(strcmp(argv_org[i],"-cfg")==0){ // configuration file
				
				if(!GetCmdLineFromFile(szCmdLine,argv_org[i+1])){
					return false;
				}
				argc2 += (GetArgv(szCmdLine,argv+argc2,0)-1);
				i++;
			}
			else  strcpy(argv[argc2],argv_org[i]);
	}
	argc = argc2;

	// information of input file
	if(strcmp(argv[1],"-info")==0){
		BlFileInfo = true;
		strcpy(SzReadFile,argv[2]);
		return true;
	}


	// read option
	i2 = 0;
	for(i=1;i<(unsigned int)argc-2;i++)
	{
		const int ret_config = CONFIG::analyze_argv( argv+i );
		if( ret_config ){
			i += ret_config-1;
			i2 += ret_config;
		}

		//
		// obsolete part
		// 

		// silent mode
		else if(strcmp(argv[i],"-silent")==0) 
		{
			BlVerbose = false;
			i2++;
		}
/* obsolete

		// HWND of lockon
		else if(strcmp(argv[i],"-lockon")==0) 
		{
			HdWndLkm = (HWND)strtoul(argv[i+1],NULL,10);
			i++; i2+=2;
		}
*/
		// delete input file
		else if(strcmp(argv[i],"-del")==0) 
		{
			BlDelFile = true;
			i2++;
		}

		// don't use SSE2
		else if(strcmp(argv[i],"-nosse2")==0) {
			BlUseSSE2 = false;
			i2++;
		}

		// don't check free space of HDD
		else if(strcmp(argv[i],"-nochkhdd")==0) 
		{
			BlNoChkHdd = true;
			i2++;
		}

		// set user defined strings
		else if(strcmp(argv[i],"-udef1")==0) 
		{
			strcpy(SzUserDef[0],argv[i+1]);
			i++; i2+=2;
		}
		else if(strcmp(argv[i],"-udef2")==0) {
			strcpy(SzUserDef[1],argv[i+1]);
			i++; i2+=2;
		}
		else if(strcmp(argv[i],"-udef3")==0) {
			strcpy(SzUserDef[2],argv[i+1]);
			i++; i2+=2;
		}

		// endless mode
		else if(strcmp(argv[i],"-endless")==0) 
		{
			BlEndless = true;
			i2++;
		}		

		// dither 
		else if(strcmp(argv[i],"-dither")==0) 
		{
			FDAT.bDither = true;
			FDAT.dDitherAmp  = atof(argv[i+1]);
			if(FDAT.dDitherAmp <= 0) FDAT.dDitherAmp = 0.5;
			i++; i2+=2;
		}

		// peak normalizer(dB)
		else if(strcmp(argv[i],"-normal")==0) 
		{
			DwNormalMode = NORMAL_PEAKBYDB;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}

		// average normalizer 
		else if(strcmp(argv[i],"-normal_avg")==0)
		{
			DwNormalMode = NORMAL_AVG;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}

		// RMS normalizer
		else if(strcmp(argv[i],"-normal_rms")==0)
		{
			DwNormalMode = NORMAL_RMS;
			DbNormalLevel = min(0,max(-192.0,atof(argv[i+1])));
			i++; i2+=2;
		}		

		// no stereolink in  normalizer
		else if(strcmp(argv[i],"-normal_nolink")==0) 
		{
			BlNormalStereo = false;
			i2++;
		}

		// config of compressor after normalizer
		else if(strcmp(argv[i],"-normal_comp")==0)
		{
			FDAT.dNormalRatio = max(1,min(1000,atof(argv[i+1])));
			FDAT.dNormalTh=  min(0,max(-192.0,atof(argv[i+2])));
			FDAT.dNormalAttack  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dNormalRelease  = max(0,min(5000,atof(argv[i+4])));
			FDAT.dwNormalCompRMS  = max(0,min(10000,atoi(argv[i+5])));

			i+=5; i2+=6;
		}

		// buffer size
		else if(strcmp(argv[i],"-buf")==0) 
		{
			DwBufSize = max(1,min(50,atoi(argv[i+1])));
			i++; i2+=2;
		}

		/* obsolete
		// buffer size of pipe
		else if(strcmp(argv[i],"-pipebuf")==0) 
		{
			DwPipeBufSize = max(1,min(50,atoi(argv[i+1])));
			i++; i2+=2;
		}
		*/

		// gain
		else if(strcmp(argv[i],"-vol")==0) 
		{
			FDAT.dVolume = max(0.0001,min(100.,atof(argv[i+1])));
			i++; i2+=2;
		}

		// balance
		else if(strcmp(argv[i],"-balance")==0) 
		{
			FDAT.bBalance = true;
			FDAT.dBalance[0] = max(0,min(100.,atof(argv[i+1])));
			FDAT.dBalance[1] = max(0,min(100.,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// fade in
		else if(strcmp(argv[i],"-fin")==0) {
			DbFadeInTime = max(0.1,min(10.,atof(argv[i+1])));
			i++; i2+=2;
		}

		// fade out
		else if(strcmp(argv[i],"-fout")==0) {
			DbFadeOutTime = max(0.1,min(10.,atof(argv[i+1])));
			i++; i2+=2;
		}
/*		
		// DC offset
		else if(strcmp(argv[i],"-ofs")==0) {
			FDAT.bOffset = true; 
			FDAT.dOffset[0] = atof(argv[i+1]);
			FDAT.dOffset[1] = atof(argv[i+2]);
			i+=2; i2+=3;
		}
*/
		// auto adjust DC offset
		else if(strcmp(argv[i],"-autoofs")==0) {
			FDAT.bAutoOffset = true; 
			FDAT.dwAutoOffsetTime = max(5,atoi(argv[i+1]));
			i++;i2+=2;
		}
		
		// cut
		else if(strcmp(argv[i],"-cut")==0) {
			BlCutFile = true;
			DwCopyBlock = 1;
			if(!ExchangeTime(argv[i+1],&DbStartTime[0]))
				DbStartTime[0] = max(0,atof(argv[i+1]));
			if(!ExchangeTime(argv[i+2],&DbEndTime[0]))
				DbEndTime[0] = max(0,atof(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-cutm")==0) {
			BlCutFile = true;
			DwCopyBlock = max(0,min(MAXCOPYBLOCK,atoi(argv[i+1])));
			i++; i2+=2;
			for(i3 = 0; i3 < DwCopyBlock; i3++){
				if(!ExchangeTime(argv[i+1],&DbStartTime[i3]))
					DbStartTime[i3] = max(0,atof(argv[i+1]));
				if(!ExchangeTime(argv[i+2],&DbEndTime[i3]))
					DbEndTime[i3] = max(0,atof(argv[i+2]));
				i+=2; i2+=2;
			}
		}
		
		else if(strcmp(argv[i],"-cuttail")==0) {
			BlCutFile = true;
			BlCutTail = true;
			DwCopyBlock = 1;
			if(!ExchangeTime(argv[i+1],&DbStartTime[0]))
				DbStartTime[0] = max(0,atof(argv[i+1]));
			if(!ExchangeTime(argv[i+2],&DbEndTime[0]))
				DbEndTime[0] = max(0,atof(argv[i+2]));
			i+=2; i2+=3;
		}


		// specify offset and size of blocks directly
#ifdef WIN32
		else if(strcmp(argv[i],"-cutb")==0) {
			BlCutFile = true;
			DwCopyBlock = 1;
			N64OffsetBlk[0] = _atoi64(argv[i+1]);
			N64DataSizeBlk[0] = _atoi64(argv[i+2]);
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-cutmb")==0) {
			BlCutFile = true;
			DwCopyBlock = max(0,min(MAXCOPYBLOCK,atoi(argv[i+1])));
			i++; i2+=2;
			for(i3 = 0; i3 < DwCopyBlock; i3++){
				N64OffsetBlk[i3] = _atoi64(argv[i+1]);
				N64DataSizeBlk[i3] = _atoi64(argv[i+2]);
				i+=2; i2+=2;
			}
		}
#endif

		// -shift
		else if(strcmp(argv[i],"-shift")==0) {
			DbShiftTime = max(0,atof(argv[i+1]));
			i++; i2+=2;
		}
		
		// mix
		else if(strcmp(argv[i],"-mix")==0) {
			FDAT.bMixLR = true;
			FDAT.dMixLRLevel = max(0,min(1,atof(argv[i+1])));
			i++; i2+=2;
		}

		// mixfile
		else if(strcmp(argv[i],"-mixfile")==0) {
			FDAT.bMixFile = true;
			FDAT.dMixLevel[0]  = max(0,min(10.,atof(argv[i+1])));
			FDAT.dMixLevel[1]  = max(0,min(10.,atof(argv[i+2])));
			if(!ExchangeTime(argv[i+3],&FDAT.dMixStartTime[0]))
				FDAT.dMixStartTime[0] = max(0,atof(argv[i+3]));
			if(!ExchangeTime(argv[i+4],&FDAT.dMixStartTime[1]))
				FDAT.dMixStartTime[1] = max(0,atof(argv[i+4]));
			strcpy(FDAT.szMixFile,argv[i+5]);
			i+=5; i2+=6;
		}

		// compressor
		else if(strcmp(argv[i],"-comp")==0){
			FDAT.bComp = true;
			FDAT.dCompRatio = max(1,min(1000,atof(argv[i+1])));
			FDAT.dCompTh=  min(0,max(-192.0,atof(argv[i+2])));
			FDAT.dCompAttack  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dCompRelease  = max(0,min(5000,atof(argv[i+4])));
			FDAT.dwCompRMS  = max(0,min(10000,atoi(argv[i+5])));

			i+=5; i2+=6;
		}	

		// subband noise gate
		else if(!strcmp(argv[i],"-ngate_sb")){
			FDAT.bNgate = true;
			FDAT.dNgateTh = max(-500,min(0,atof(argv[i+1])));
			FDAT.dNgateAttack  = max(0,min(5000,atof(argv[i+2])));
			FDAT.dNgateRelease  = max(0,min(5000,atof(argv[i+3])));
			FDAT.dwNgateRMS  = max(0,min(100,atoi(argv[i+4])));
			i+=4; i2+=5;
		}


		/* obsolete
		// adaptic filter
		else if(strcmp(argv[i],"-adp")==0) {
			FDAT.bADP = true;
			FDAT.dADPlevel = max(0,min(1,atof(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-adp_detail")==0) {
			FDAT.bADP = true;
			FDAT.dADPlevel = max(0,min(1,atof(argv[i+1])));
			FDAT.dwADPleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+2])));
			FDAT.dwADPDelayTime = max(1,min(5000,atoi(argv[i+3])));
			DwADPTrainTime = max(0,min(60,atoi(argv[i+4])));
			FDAT.dADPDb = min(-20,max(-200,atof(argv[i+5])));
			i+=5; i2+=6;
		}

		else if(strcmp(argv[i],"-adpchr")==0) {
			BlADPOutFilter = true;
			i2++;
		}

		// ADP-MDCT filter
		else if(strcmp(argv[i],"-adp_mdct")==0){
			FDAT.bWaveSh = true;
			FDAT.dWaveShLevel = max(-100,min(0,atof(argv[i+1])));
			i++; i2+=2;
		}
*/
			
		// FIR filter
		else if(strcmp(argv[i],"-fir_lpf")==0) {
			FDAT.dwFIRFilter = LPF;
			FDAT.dwFIRCutLow= max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-fir_hpf")==0) {
			FDAT.dwFIRFilter = HPF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-fir_bpf")==0) {
			FDAT.dwFIRFilter = BPF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwFIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-fir_bsf")==0) {
			FDAT.dwFIRFilter = BSF;
			FDAT.dwFIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwFIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}
		
		else if(strcmp(argv[i],"-firdb")==0) { // loss
			FDAT.dFIRDb = min(-20,max(-200,atof(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-firln")==0) {
			FDAT.dwFIRleng = max(3,min(MAXFILTERPOINT,atoi(argv[i+1])));
			i++; i2+=2;
		}
		
		else if(strcmp(argv[i],"-firchr")==0) {
			BlFIROutFilter = true;
			i2++;
		}

		// 10bands-EQ
		else if(strcmp(argv[i],"-fir_eq")==0){
			FDAT.bFIREQ = true;
			i2++;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 10bands-EQ (detail)
		else if(strcmp(argv[i],"-fir_eq_detail")==0) {
			FDAT.bFIREQ = true;
			FDAT.dwEQleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+1])));
			FDAT.dEQDb = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dEQ_Q = max(0.01,atof(argv[i+3]));
			i+=3; i2+=4;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 3bands-EQ
		else if(strcmp(argv[i],"-fir_eq3")==0){
			FDAT.bFIREQ = true;
			FDAT.dwEQband = 3;
			FDAT.dwEQleng = 255;
			FDAT.dEQDb = -60;
			FDAT.dEQ_Q = 0.643;
			i2++;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// 3bands-EQ (detail)
		else if(strcmp(argv[i],"-fir_eq3_detail")==0) {
			FDAT.bFIREQ = true;
			FDAT.dwEQband = 3;
			FDAT.dwEQleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+1])));
			FDAT.dEQDb = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dEQ_Q = max(0.01,atof(argv[i+3]));
			i+=3; i2+=4;
			for(i3 = 0; i3 < FDAT.dwEQband; i3++){
				FDAT.dEQLevel[i3] = max(-MAX_EQDB,min(MAX_EQDB,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// IIR filter

		// LPF
		else if(strcmp(argv[i],"-iir_lpf")==0) {
			FDAT.dwIIRFilter = LPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}

		// HPF
		else if(strcmp(argv[i],"-iir_hpf")==0) {
			FDAT.dwIIRFilter = HPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			i++; i2+=2;
		}

		// BPF
		else if(strcmp(argv[i],"-iir_bpf")==0) {
			FDAT.dwIIRFilter = BPF;
			FDAT.dwIIRCutLow = max(0,atoi(argv[i+1]));
			FDAT.dwIIRCutHigh = max(0,atoi(argv[i+2]));
			i+=2; i2+=3;
		}

		// de-emphasis
		else if(strcmp(argv[i],"-demp")==0) {
			FDAT.bDemp = true;
			FDAT.dDempDb = 1;
			i2++;
		}

		// emphasis
		else if(strcmp(argv[i],"-emp")==0) {
			FDAT.bDemp = true;
			FDAT.dDempDb = -1;
			i2++;
		}

		// shelving EQ low
		else if(strcmp(argv[i],"-sveq_l")==0) {
			FDAT.bSVEQL = true;
			FDAT.dwSVEQLfreq = max(0,atoi(argv[i+1]));
			FDAT.dSVEQLdb = max(-20,min(20,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// shelving EQ high
		else if(strcmp(argv[i],"-sveq_h")==0){
			FDAT.bSVEQH = true;
			FDAT.dwSVEQHfreq = max(0,atoi(argv[i+1]));
			FDAT.dSVEQHdb = max(-20,min(20,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// peaking EQ
		else if(strcmp(argv[i],"-pkeq")==0){
			FDAT.bPKEQ = true;
			FDAT.dwPKfreq = max(0,atoi(argv[i+1]));
			FDAT.dPKdb = max(-20,min(20,atof(argv[i+2])));
			FDAT.dPKQ = max(0,min(100,atof(argv[i+3])));
			i+=3; i2+=4;
		}

		else if(strcmp(argv[i],"-iirchr")==0) {
			BlIIROutFilter = true;
			i2++;
		}

		// phase invert
		else if(strcmp(argv[i],"-phinv")==0) {
			FDAT.bPhaseInv = true;
			i2++;
		}

		// configuration file of names of output files
		else if(strcmp(argv[i],"-outfile")==0){
			BlOutFile = true;
			strcpy(SzOutFile,argv[i+1]);
			i++; i2+=2;
		}
		
#ifdef WIN32
		else if(strcmp(argv[i],"-outfile_fm")==0){
			BlOutFile = true;
			HdOutFileMap = (HANDLE)strtoul(argv[i+1],NULL,10);
			i++; i2+=2;
		}
#endif
		// remove no sound part of in head of data
		else if(strcmp(argv[i],"-headcutting")==0) {
			FDAT.dwNoSndMode = NOSND_HEADCUTONLY;
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+1])));
			i++; i2+=2;
		}

		// split at no sound 
		else if(strcmp(argv[i],"-nosound")==0){
			FDAT.dwNoSndMode = NOSND_SEARCH;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+1])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+2])));
			FDAT.dNoSndBound = FDAT.dNoSndBoundHead;

			i+=2; i2+=3;
		}

		else if(strcmp(argv[i],"-nosound_detail")==0) {
			FDAT.dwNoSndMode = max(0,min(2, atoi(argv[i+1]))) +1;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+2])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+3])));
			FDAT.dNoSndBound = min(0,max(-192,atof(argv[i+4])));

			i+=4; i2+=5;
		}

		/* obsolete

		else if(strcmp(argv[i],"-nosound_detail2")==0) {
			FDAT.dwNoSndMode = max(0,min(2, atoi(argv[i+1]))) +1;
			FDAT.dwNoSndTime = max(1,min(10000,atoi(argv[i+2])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+3])));
			FDAT.dNoSndBound = min(0,max(-192,atof(argv[i+4])));
			FDAT.dwNoSndSuspend		= max(0,min(14400000,atoi(argv[i+5])));
			FDAT.dwNoSndM1toM2	= max(0,min(14400000,atoi(argv[i+6])));
			FDAT.dwNoSndRecStop = max(0,min(14400000,atoi(argv[i+7])));
			i+=7; i2+=8;
		}
		*/

		// FIR filtering befor nosond
		else if(strcmp(argv[i],"-nosound_fir")==0) {
			FDAT.dwNoSndFIRFilter = max(0,atoi(argv[i+1]));
			FDAT.dwNoSndFIRCutLow = max(0,atoi(argv[i+2]));
			FDAT.dwNoSndFIRCutHigh = max(0,atoi(argv[i+3]));
			FDAT.dwNoSndFIRleng = max(4,min(MAXFILTERPOINT,atoi(argv[i+4])));
			FDAT.dNoSndFIRDb = min(-20,max(-200,atof(argv[i+5])));
			i+=5; i2+=6;
		}

		/* obsolete

		// stop recording at no sound
		else if(strcmp(argv[i],"-nosound_stop") ==0) {
			FDAT.dwNoSndMode = NOSND_STOP;
			FDAT.dwNoSndRecStop = max(0,min(14400000,atoi(argv[i+1])));
			FDAT.dNoSndBoundHead = min(0,max(-192,atof(argv[i+2])));
			FDAT.dNoSndBound = FDAT.dNoSndBoundHead;
			i+=2; i2+=3;

		}
		*/

		// split at time
		else if(!strcmp(argv[i],"-split")) {
			FDAT.bSplit = true;
			if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[0]))
				FDAT.dSplitTime[0] = max(1,min(72000,atof(argv[i+1])));
			i++; i2+=2;
		}

		// split2
		else if(strcmp(argv[i],"-split2")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-split2: -split2 n t1 t2 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < i4; i3++){
				if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[i3]))
				FDAT.dSplitTime[i3] = max(.1,min(72000,atof(argv[i+1])));
				i++; i2++;
			}
		}

		// split3
		else if(strcmp(argv[i],"-split3")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-split3: -split3 n t1 t2 ...\n");
				return false;
			}
			i++; i2+=2;
			dFoo = 0;
			for(i3 = 0; i3 < i4; i3++){
				if(!ExchangeTime(argv[i+1],&FDAT.dSplitTime[i3]))
					FDAT.dSplitTime[i3] = max(.1,min(72000,atof(argv[i+1])));
				dFoo2 = FDAT.dSplitTime[i3];
				FDAT.dSplitTime[i3] -= dFoo;
				dFoo = dFoo2;
				i++; i2++;
			}
		}

		// split at size (kbyte) of output
		else if(!strcmp(argv[i],"-splitkbyte")) {
			FDAT.bSplit = true;
			FDAT.n64SplitByte = max(1,_atoi64(argv[i+1])) * 1024;  
			i++; i2+=2;
		}		

		
		// split at size (byte) of output
		else if(strcmp(argv[i],"-splitbm")==0) {
			FDAT.bSplit = true;
			i4 = max(0,min(MAX_SPLIT,atoi(argv[i+1])));
			if(i+i4+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-splitbm: -splitbm n b1 b2 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < i4; i3++){
				FDAT.n64SplitByteMalti[i3] = _atoi64(argv[i+1]);
				i++; i2++;
			}
			FDAT.n64SplitByte = FDAT.n64SplitByteMalti[0]; 
		}

		// add no sound to head and tail
		else if(strcmp(argv[i],"-addspc")==0) {
			DbAddSp[0] = max(0,min(5,atof(argv[i+1])));
			DbAddSp[1] = max(0,min(5,atof(argv[i+2])));
			i+=2; i2+=3;
		}

		// synthesize sine waves
		else if(strcmp(argv[i],"-addsin")==0) {
			FDAT.dwAddSinNum = max(1,min(MAX_SINE_WAVES,atoi(argv[i+1])));
			if(i+FDAT.dwAddSinNum+1 >= (DWORD)argc-2){
				fprintf(stderr,"\n-addsin: -addsin n freq1 db1 ph1 ...\n");
				return false;
			}
			i++; i2+=2;
			for(i3 = 0; i3 < FDAT.dwAddSinNum; i3++){
				FDAT.dwAddSinFreq[i3] = max(0,atoi(argv[i+1]));
				FDAT.dAddSinDb[i3] = min(0,max(-192.0,atof(argv[i+2])));
				FDAT.dwAddSinPhase[i3] = max(0,min(360,atoi(argv[i+3])));
				i+=3; i2+=3;
			}
		}

		// re-sampling
		else if(strcmp(argv[i],"-rsmp")==0) {
			FDAT.bRsmp = true;
			i2++;
		}

		else if(strcmp(argv[i],"-rsmp_detail")==0) {
			FDAT.bRsmp = true;
			FDAT.dwRsmpLn = max(4,atoi(argv[i+1]));
			FDAT.dRsmpDB = min(-20,max(-200,atof(argv[i+2])));
			FDAT.dwRsmpCut= max(0,atoi(argv[i+3]));
			i+=3; i2+=4;
		}

		else if(strcmp(argv[i],"-rsmpchr")==0) {
			BlRsmpOutFilter = true;
			i2++;
		}

		// bit of output file
		else if(strcmp(argv[i],"-bit8")==0) {
			FDAT.dwBitChange = 8;
			i2++;
		}
	
		else if(strcmp(argv[i],"-bit16")==0) {
			FDAT.dwBitChange = 16;
			i2++;
		}

		else if(strcmp(argv[i],"-bit24")==0) {
			FDAT.dwBitChange = 24;
			i2++;
		}

		else if(strcmp(argv[i],"-bit64")==0) {
			FDAT.dwBitChange = 64;
			FDAT.dwBitChangeTag = WAVE_FORMAT_IEEE_FLOAT; 
			i2++;
		}

		else if(strcmp(argv[i],"-bit32float")==0) {
			FDAT.dwBitChange = 32;
			FDAT.dwBitChangeTag = WAVE_FORMAT_IEEE_FLOAT; 
			i2++;
		}

		else if(strcmp(argv[i],"-bit32long")==0) {
			FDAT.dwBitChange = 32;
			i2++;
		}

		// header offset
		else if(strcmp(argv[i],"-headofs")==0){
			BlHeadOffset = true;
			N64FileOffset = _atoi64(argv[i+1]);
			i+=1; i2+=2;
		}

		// specify input format
		else if(strcmp(argv[i],"-format")==0){
			N64FileDataSize = atoi(argv[i+1]);
			InputWaveFmt.set(atoi(argv[i+5]), atoi(argv[i+3]), atoi(argv[i+2]), atoi(argv[i+4]) );
			N64FileDataSize = N64FileDataSize*InputWaveFmt.avgbyte();
			i+=5; i2+=6;
		}

		// no wave header output
		else if(strcmp(argv[i],"-nowavehdr")==0)
		{
			BlWaveHdrOut = false;
			i2++;
		}
/* obsolete
		// no extra wave header
		else if(strcmp(argv[i],"-noextrahdr")==0) 
		{
			BlExtChunkOfHdr = false;
			i2++;
		}
*/
		/*obsolete
		// set timestamp to creation time
		else if(strcmp(argv[i],"-fstamp")==0 || strcmp(argv[i],"-tstamp")==0) 
		{
			BlFileStamp = true;
			i2++;
		}

		*/

		/* obsolete
		// pipe
		else if(strcmp(argv[i],"-pipeout")==0){
#ifdef USEWIN32API  
			BlCreatePipe = true;
#endif
			strcpy(SzPipeComand,argv[i+1]);
			i++; i2+=2;
		}

		// exec
		else if(strcmp(argv[i],"-exec")==0){
#ifdef USEWIN32API  
			BlExecCom = true;
#endif
			strcpy(SzExecComand,argv[i+1]);
			i++; i2+=2;
		}
		
		// exec_nowin
		else if(strcmp(argv[i],"-exec_nowin")==0){
#ifdef USEWIN32API  
			BlExecCom = true;
#endif
			BlOpenWin = false;
			strcpy(SzExecComand,argv[i+1]);
			i++; i2+=2;
		}
		*/

		// sound device
		else if(strcmp(argv[i],"-device")==0){
			UnWaveDevID = atoi(argv[i+1]);
			i++; i2+=2;
		}

		// text out
		else if(strcmp(argv[i],"-text")==0){
			BlTextOut = true;
			i2++;
		}
			
		else{
			ShowAbout();
			exit(1);
		}
	}
	
	if((unsigned int)argc-i2 != 3) {
		ShowAbout();
		exit(1);
	}

	// input file
	strcpy(SzReadFile,argv[1+i2]);

	// output file
	strcpy(SzOrgWriteFile,argv[2+i2]);
	if(strcmp(SzOrgWriteFile,"NULL") == 0) strcpy(SzOrgWriteFile,"null");

	// init output file names
	if(BlOutFile){
		if(!PrepareOutputFileName(SzOutFile,HdOutFileMap,szErr)){
			fprintf(stderr,szErr);
			return false;
		}
	}

	return true;
}




//-----------------------------------------------------
// initialize parameters
BOOL SetParam(){
	
	DWORD i,dwFoo;
	double dFoo;
	
	char szNewName[MAX_PATH];
	char szDriveName[MAX_PATH],szPathName[MAX_PATH],szFileName[MAX_PATH];
	ULONGLONG n64HddSize;

	ULONGLONG n64Foo;

	/* obsolete
#ifdef USEWIN32API
	HANDLE hTimeFile;
	SYSTEMTIME sysTime;
	FILETIME ftLocal;
#endif
	*/

	// get current system time
	GetLocalTime(&SystemTime);

	//----------------------------------------------
	// analysis of input data
	//----------------------------------------------

	// empty file
	if(strcmp(SzReadFile,"nosignal")==0)
	{
		strcpy(SzReadFile,"null");
		BlNoSignal = true;
	}

	// stdin
	if(strcmp(SzReadFile,"stdin")==0 || strcmp(SzReadFile,"-")==0)
	{
		strcpy(SzReadFile,"stdin");
		BlStdin = true;
	}

	if(BlStdin)
	{
		// no normalize
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;

		// endless mode is set
		BlEndless = true;

		// no fade out
		DbFadeOutTime = 0;
	}

	fprintf(stderr,"input: ");

	// analyze the format of input file 
	if(!BlHeadOffset && !BlNoSignal)
	{
		InputWaveFmt.read(SzReadFile);
		N64FileDataSize = InputWaveFmt.get_datasize();
		N64FileOffset = InputWaveFmt.get_offset();
	}
	// notice: if input is stdin, file pointer has already moved at N64FileOffset byte.

	// set buffer size
	dwFoo = 1; 
	while((double)dwFoo/InputWaveFmt.avgbyte()*1000. < DwBufSize*100.) dwFoo = dwFoo << 1;
	DwBufSize = dwFoo;

	fprintf(stderr,"tag = %u, ",InputWaveFmt.tag());
	fprintf(stderr,"%u ch, ",InputWaveFmt.channels());
	fprintf(stderr,"%u Hz, ",InputWaveFmt.rate());
	fprintf(stderr,"%u bit, ",InputWaveFmt.bits());
	fprintf(stderr,"offset = %d byte\n",N64FileOffset);

	if(N64FileDataSize < 4 * 1024 * 1024)
		fprintf(stderr,"data size = %u byte, ",N64FileDataSize);
	else
	{
		dFoo = (double)N64FileDataSize/1024/1024;
		fprintf(stderr,"data size = %.2lf Mbyte, ",dFoo);
	}
	dFoo = (double)N64FileDataSize/InputWaveFmt.avgbyte();
	fprintf(stderr,"time = %.2lf sec \n",dFoo);

	// if -info option is specified, finish here.
	if(BlFileInfo)
	{
		ShowPeakWave(SzReadFile,DwBufSize,InputWaveFmt,N64FileOffset,N64FileDataSize); 
		return true;
	}

	//----------------------------------------------
	// analysis of output data
	//----------------------------------------------

	// get original file name
	strcpy(SzWriteFile,SzOrgWriteFile);
	if(((FDAT.dwNoSndMode != NOSND_NOT 
		&&  FDAT.dwNoSndMode != NOSND_HEADCUTONLY
		&&  FDAT.dwNoSndMode != NOSND_STOP)
		|| FDAT.bSplit)
		&& strcmp(SzOrgWriteFile,"null")!=0) 
		SetOutputFileName(SzOrgWriteFile,SzWriteFile,0);

#ifdef WIN32
	// wave out
	if(strcmp(SzOrgWriteFile,"waveout")==0)
	{
		BlStdout = true;
		BlWaveOut = true;
	}
#endif

	// stdout
	if(strcmp(SzOrgWriteFile,"stdout")==0)	BlStdout = true;
	
	if(BlStdout 
		) // stdout
	{
		// don't split file
		FDAT.dwNoSndMode = NOSND_NOT;
		FDAT.bSplit = false;
		
		if(BlWaveOut) // waveout
		{
			// don't output text
			BlTextOut = false;
		}
	}

	// get output format

	WriteWaveFmt.set(InputWaveFmt.tag(), InputWaveFmt.channels(),InputWaveFmt.rate(),InputWaveFmt.bits());

	// LR-mixing,
	if(FDAT.bMixLR)
	{ 
		if(InputWaveFmt.channels() == 1) FDAT.bMixLR = false;
		else WriteWaveFmt.set(WriteWaveFmt.tag(), 1,WriteWaveFmt.rate(),WriteWaveFmt.bits());
	}

	// change bit of output file
	if(FDAT.dwBitChange)
		WriteWaveFmt.set((WORD)FDAT.dwBitChangeTag, WriteWaveFmt.channels(),WriteWaveFmt.rate(),(WORD)FDAT.dwBitChange);

	// re-sampling
	if(FDAT.bRsmp)
	{
		FDAT.dwRsmpInFreq = WriteWaveFmt.rate();
	
		if(FDAT.dwRsmpInFreq != 48000){
			fprintf(stderr,"\n-rsmp: Input frequency must be 48000 hz.\n");
			return false;
		}
		
		FDAT.dwRsmpOutFreq = 44100;
		WriteWaveFmt.set(WriteWaveFmt.tag(), WriteWaveFmt.channels(), FDAT.dwRsmpOutFreq, WriteWaveFmt.bits() );
	}

	fprintf(stderr,"\noutput: ");
	fprintf(stderr,"tag = %u, ",WriteWaveFmt.tag());
	fprintf(stderr,"%u ch, ",WriteWaveFmt.channels());
	fprintf(stderr,"%u Hz, ",WriteWaveFmt.rate());
	fprintf(stderr,"%u bit\n",WriteWaveFmt.bits());

	if(BlEndless)
	{
		fprintf(stderr,"endless mode\n");

		// no fade out
		DbFadeOutTime = 0;
	}

	if(BlTextOut)
	{
		if(WriteWaveFmt.bits() == 64){
			fprintf(stderr,"\nCannot create 64bit text file.\n");
			return false;
		}

		fprintf(stderr,"text out\n");

		// no wave header
		BlWaveHdrOut = false;
	}

	if(!BlWaveHdrOut) fprintf(stderr,"no wave header\n");
/* obsolete
	if(!BlExtChunkOfHdr) fprintf(stderr,"no extra wave header\n");
*/

	//----------------------------------------------
	// get offsets and copy sizes of blocks.
	//----------------------------------------------
	
	// -cuttail 
	if(BlCutTail){
		DbEndTime[0] = (double)N64FileDataSize/InputWaveFmt.avgbyte() - DbEndTime[0];
		if(DbEndTime[0] < 0) DbEndTime[0] = 0;
	}
	
	if(BlCutFile && DbStartTime[0] > 0 || DbEndTime[0] > 0){
		
		for(i=0;i<DwCopyBlock;i++){

			if(DbStartTime[i] > DbEndTime[i]) DbEndTime[i] = DbStartTime[i];
			
			N64OffsetBlk[i] = InputWaveFmt.block()*(LONGLONG)(DbStartTime[i]*(double)InputWaveFmt.rate());
			N64DataSizeBlk[i] = InputWaveFmt.block()*(LONGLONG)((DbEndTime[i]-DbStartTime[i])*(double)InputWaveFmt.rate());
		}

	}
	else{ 
		// if block size has been specified in -cutb and -cutmb directly, don't set value here.
		if(!BlCutFile) N64DataSizeBlk[0] = N64FileDataSize;
	}
	
	// adjust alignment
	for(i=0;i<DwCopyBlock;i++){
		
		N64OffsetBlk[i] = N64OffsetBlk[i] / (InputWaveFmt.channels() * (InputWaveFmt.bits()/8)) * InputWaveFmt.block();
		
		// add header offset to block offset
		N64OffsetBlk[i] += N64FileOffset; 
		
		N64DataSizeBlk[i] 
			= N64DataSizeBlk[i] / (InputWaveFmt.channels() * (InputWaveFmt.bits()/8)) * InputWaveFmt.block();
	}

	// offset and size of eace block
	for(i=0;i<DwCopyBlock;i++){
		if(N64OffsetBlk[i] > N64FileOffset+N64FileDataSize){
			N64OffsetBlk[i] = N64FileOffset+N64FileDataSize;
		}

		if(N64OffsetBlk[i]+N64DataSizeBlk[i] > N64FileOffset+N64FileDataSize){
			N64DataSizeBlk[i]  = N64FileOffset+N64FileDataSize - N64OffsetBlk[i];
		}
	}
	
	// get total output size
	N64TotalDataSize = 0;	
	for(i=0;i<DwCopyBlock;i++) N64TotalDataSize += N64DataSizeBlk[i];
	if(memcmp(&InputWaveFmt.get_raw(),&WriteWaveFmt.get_raw(),sizeof(WAVEFORMAT_RAW)) != 0){
		N64RealTotalDataSize = 
			WriteWaveFmt.block() * 
			(LONGLONG)((double)N64TotalDataSize 
			/ (double)InputWaveFmt.avgbyte ()
			* (double)WriteWaveFmt.rate()
			);
	}
	else N64RealTotalDataSize = N64TotalDataSize;
	
	// add no sound part
	if(DbAddSp[0] > 0 || DbAddSp[1] > 0)
	{
		DwAddSp[0] = (DWORD)(DbAddSp[0] * WriteWaveFmt.avgbyte());
		DwAddSp[1] = (DWORD)(DbAddSp[1] * WriteWaveFmt.avgbyte());
	}

	/* obsolete
	// if data size < 4G, then don't use extra chunk
	n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
	if(n64Foo <= 0xFFFFFFFF-44) BlExtChunkOfHdr = false;
	*/

	//-----------------------------
	// get file name
	//-----------------------------

	SetCommandStrings(false,SzWriteFile,SzWriteFile,SzWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt);
	strcpy(SzRealWriteFile,SzWriteFile);

#ifdef WIN32

	// wave out
	if(strcmp(SzOrgWriteFile,"waveout")==0)	strcpy(SzWriteFile,"waveout");
#endif

	// stdout
	if(strcmp(SzOrgWriteFile,"stdout")==0) strcpy(SzWriteFile,"stdout");

	// if name of output file is the same as input file, rename input file to *.bkp.
	if(strcmp(SzReadFile,SzOrgWriteFile)==0)
	{
		_splitpath(SzReadFile,szDriveName,szPathName,szFileName,NULL);	
		wsprintf(szNewName,"%s%s%s.bkp",szDriveName,szPathName,szFileName);
		DeleteFile(szNewName); // delete in advance
		if(MoveFile(SzReadFile,szNewName))
		{
			strcpy(SzReadFile,szNewName);
			fprintf(stderr,"\nChanged input file name to \"%s\".",SzReadFile);
		}
		else
		{
			fprintf(stderr,"\nCould not change input file name to \"%s\"\n",SzReadFile);
			return false;
		}
	}

	fprintf(stderr,"\n");

	//------------------------------------------------------

	//-----------------------------------------------
	// options
	//----------------------------------------------

	fprintf(stderr,"option(s):\n");

	/* obsolete
	// show timestamp
#ifdef USEWIN32API
	if(BlFileStamp && !BlNoSignal)
	{
		if(!BlStdin)
		{
			hTimeFile = CreateFile(SzReadFile,GENERIC_READ,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
			if(hTimeFile)
			{
				GetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
				CloseHandle(hTimeFile);
				FileTimeToLocalFileTime(&FtLastWriteTime,&ftLocal);
				FileTimeToSystemTime(&ftLocal,&sysTime);
		
			}
		}
		else if(!BlNoSignal)// get local time
		{
			GetLocalTime(&sysTime);
			SystemTimeToFileTime(&sysTime,&ftLocal);
			LocalFileTimeToFileTime(&ftLocal,&FtCreationTime);
			FtLastAccessTime = FtCreationTime;
			FtLastWriteTime = FtCreationTime;
		}

		fprintf(stderr,"set timestamp to: %d/%d/%d %d:%02d:%02d\n",
					sysTime.wYear,sysTime.wMonth,sysTime.wDay,
					sysTime.wHour,sysTime.wMinute,sysTime.wSecond
					);
	}
#endif	
	*/

	if(BlUseSSE2){
		if(CheckSSE2()) fprintf(stderr,"SSE2: supported\n");
	}
	
	CONFIG::status();

	fprintf(stderr,"buffer size: %ldk\n",DwBufSize/1024);
	if(FDAT.dVolume != 1.0) fprintf(stderr,"volume: %lf\n",FDAT.dVolume);
	if(FDAT.bBalance) fprintf(stderr,"balance: L = %.3lf, R = %.3lf\n",FDAT.dBalance[0],FDAT.dBalance[1]);
	if(strlen(SzUserDef[0])) fprintf(stderr,"udef1: %%1 = %s\n",SzUserDef[0]);
	if(strlen(SzUserDef[1])) fprintf(stderr,"udef2: %%2 = %s\n",SzUserDef[1]);
	if(strlen(SzUserDef[2])) fprintf(stderr,"udef3: %%3 = %s\n",SzUserDef[2]);
//	if(FDAT.bOffset) fprintf(stderr,"DC offset: left += %lf right += %lf\n",FDAT.dOffset[0],FDAT.dOffset[1]);
	if(FDAT.bPhaseInv) fprintf(stderr,"invert phase:\n");
	if(DbShiftTime != 0) fprintf(stderr,"shift output: %.2lf msec\n",DbShiftTime);
	if(FDAT.bDemp){
		if(FDAT.dDempDb == 1)
			fprintf(stderr,"de-emphasis:\n");
		else
			fprintf(stderr,"emphasis:\n");
	}
	
	// shelving EQ low
	if(FDAT.bSVEQL){
		fprintf(stderr,"shelving EQ: low, ");
		fprintf(stderr,"%d Hz, ",FDAT.dwSVEQLfreq);
		fprintf(stderr,"%.3lf dB\n",FDAT.dSVEQLdb);
	}

	// shelving EQ high
	if(FDAT.bSVEQH){
		fprintf(stderr,"shelving EQ: high, ");
		fprintf(stderr,"%d Hz, ",FDAT.dwSVEQHfreq);
		fprintf(stderr,"%.3lf dB\n",FDAT.dSVEQHdb);
	}

	// peaking EQ high
	if(FDAT.bPKEQ){
		fprintf(stderr,"peaking EQ: ");
		fprintf(stderr,"%d Hz(%d - %d Hz), ",
			FDAT.dwPKfreq,
			FDAT.dwPKfreq-(DWORD)(FDAT.dwPKfreq/2/FDAT.dPKQ),
			FDAT.dwPKfreq+(DWORD)(FDAT.dwPKfreq/2/FDAT.dPKQ)
			);
		fprintf(stderr,"%.3lf dB, Q = %.3lf\n",FDAT.dPKdb,FDAT.dPKQ);
	}

	// auto DC offset 
	if(FDAT.bAutoOffset){ // -autoofs
//		FDAT.bOffset = false;  // if -autoofs is specified, then -ofs is canceled.
//		FDAT.dOffset[0] = 0;
//		FDAT.dOffset[1] = 0;
		fprintf(stderr,"Auto DC offset: %d sec\n",FDAT.dwAutoOffsetTime);
	}

	// -outfile
	if(BlOutFile){
		if(HdOutFileMap) fprintf(stderr,"outfile: file mapping\n");
		else fprintf(stderr,"outfile: %s\n",SzOutFile);
	}

	// add no sound part
	if(DbAddSp[0] > 0 || DbAddSp[1] > 0)
	{
		fprintf(stderr,"add space: %f sec (head), %f sec (tail)\n"
			,DbAddSp[0],DbAddSp[1]);
	}

	// dither
	if(WriteWaveFmt.bits() >= 24) FDAT.bDither = false;
	if(FDAT.bDither) fprintf(stderr,"dither: x%6.3lf\n",FDAT.dDitherAmp);

	// LR-mixing
	if(FDAT.bMixLR) fprintf(stderr,"LR-mixing: L:R = %6.3lf : %6.3lf\n",FDAT.dMixLRLevel,1-FDAT.dMixLRLevel);

	// file mixing
	if(FDAT.bMixFile){
		fprintf(stderr,"\n");
		fprintf(stderr,"file mixing: %s\n",FDAT.szMixFile);
		fprintf(stderr,"level = %6.3lf : %6.3f, ",FDAT.dMixLevel[0],FDAT.dMixLevel[1]);
		fprintf(stderr,"time =  %.1f sec : %.1lf sec\n",FDAT.dMixStartTime[0],FDAT.dMixStartTime[1]);
	}

	// compressor
	if(FDAT.bComp){
		fprintf(stderr,"\n");
		fprintf(stderr,"compressor:\nratio = %4.1lf, threshold = %6.3lf dB\n",
			FDAT.dCompRatio,FDAT.dCompTh);
		fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
			FDAT.dCompAttack,FDAT.dCompRelease,FDAT.dwCompRMS);
	}

	// subband noise gate
	if(FDAT.bNgate){
		fprintf(stderr,"\n");
		fprintf(stderr,"subband noise gate:\n");
		fprintf(stderr,"threshold =  %6.3lf dB\n",FDAT.dNgateTh);
		fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
			FDAT.dNgateAttack,FDAT.dNgateRelease,FDAT.dwNgateRMS);
	}

	//----------------
	
	// FIR
	if(FDAT.dwFIRFilter != NO_FILTER){
		fprintf(stderr,"\n");
		fprintf(stderr,"FIR filter: ");
		
		switch(FDAT.dwFIRFilter){
		case LPF: fprintf(stderr,"LPF\n"); break;
		case HPF: fprintf(stderr,"HPF\n"); break;
		case BPF: fprintf(stderr,"BPF\n"); break;
		case BSF: fprintf(stderr,"BSF\n"); break;
		}
		
		fprintf(stderr,"length = %d, ",FDAT.dwFIRleng);
		fprintf(stderr,"delay = %d(%.2lf msec), "
			,(FDAT.dwFIRleng-1)/2,(double)(FDAT.dwFIRleng-1)/2*1000/InputWaveFmt.rate());
		fprintf(stderr,"loss = %.2lf db, ",FDAT.dFIRDb);
		if(FDAT.dwFIRCutLow > FDAT.dwFIRCutHigh) FDAT.dwFIRCutHigh = FDAT.dwFIRCutLow;
		
		if(FDAT.dwFIRFilter == BPF || FDAT.dwFIRFilter == BSF){
			fprintf(stderr,"%d hz - ",FDAT.dwFIRCutLow);
			fprintf(stderr,"%d hz\n",FDAT.dwFIRCutHigh);
		}
		else fprintf(stderr,"%d hz\n",FDAT.dwFIRCutLow);
	
		
	}
	
	// FIR-EQ
	if(FDAT.bFIREQ){
		double dCntFreq[10] = {31.5, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000};
		
		if(FDAT.dwEQband == 3){
			dCntFreq[0] = 125;
			dCntFreq[1] = 1000;
			dCntFreq[2] = 8000;
		}		

		fprintf(stderr,"\n");
		fprintf(stderr,"FIR-EQ:\n");
		fprintf(stderr,"length = %d, ",FDAT.dwEQleng);
		fprintf(stderr,"delay = %d(%.2lf msec), "
			,(FDAT.dwEQleng-1)/2,(double)(FDAT.dwEQleng-1)/2*1000/InputWaveFmt.rate());
		fprintf(stderr,"loss = %.2lf db, ",FDAT.dEQDb);
		fprintf(stderr,"Q = %.2lf\n",FDAT.dEQ_Q);

		fprintf(stderr,"freq.: ");
		for(i=0;i<FDAT.dwEQband;i++) fprintf(stderr,"%6.0lf ",dCntFreq[i]);
		fprintf(stderr,"\nlevel: ");
		for(i=0;i<FDAT.dwEQband;i++) fprintf(stderr,"%6.0lf ",FDAT.dEQLevel[i]);
		fprintf(stderr,"\n");
	}
	
	// FIR filter length must be odd number
	if(FDAT.dwFIRleng % 2 == 0 || FDAT.dwEQleng % 2 == 0){
		fprintf(stderr,"\nLength of FIR filter must be odd number.\n");
		return false;
	}	

	//-----
	
	// IIR
	if(FDAT.dwIIRFilter != NO_FILTER){

		fprintf(stderr,"\n");
		fprintf(stderr,"IIR filter: ");
		
		switch(FDAT.dwIIRFilter){
		case LPF: fprintf(stderr,"LPF  "); break;
		case HPF: fprintf(stderr,"HPF  "); break;
		case BPF: fprintf(stderr,"BPF  "); break;
		case BSF: fprintf(stderr,"BSF  "); break;
		}
		
		if(FDAT.dwIIRCutLow > FDAT.dwIIRCutHigh) FDAT.dwIIRCutHigh = FDAT.dwIIRCutLow;

		if(FDAT.dwIIRFilter == BPF || FDAT.dwIIRFilter == BSF){
			fprintf(stderr,"%d hz - ",FDAT.dwIIRCutLow);
			fprintf(stderr,"%d hz\n",FDAT.dwIIRCutHigh);
		}
		else fprintf(stderr,"%d hz\n",FDAT.dwIIRCutLow);
	}



	// resampling
	if(FDAT.bRsmp){
		fprintf(stderr,"\n");
		fprintf(stderr,"re-sampling: ");
		fprintf(stderr,"%d hz -> %d hz\n",FDAT.dwRsmpInFreq,FDAT.dwRsmpOutFreq);
		fprintf(stderr,"length = %d, ",FDAT.dwRsmpLn);
		fprintf(stderr,"loss = %6.3lf db, ",FDAT.dRsmpDB);
		fprintf(stderr,"cut off = %d hz, ",FDAT.dwRsmpCut);
		
		if(FDAT.dwRsmpInFreq == 48000 && FDAT.dwRsmpOutFreq == 44100){  
			FDAT.dwRsmpUp = 147;   
			FDAT.dwRsmpDown = 160;
		}

		fprintf(stderr,"up to %d hz\n",FDAT.dwRsmpInFreq*FDAT.dwRsmpUp);
		
	}


	// synthesize sine waves
	if(FDAT.dwAddSinNum){
		fprintf(stderr,"\n");

		fprintf(stderr,"synthesize sine waves:\n");
		for(i=0;i<FDAT.dwAddSinNum;i++){
			fprintf(stderr,"[%2d] %d hz, %lf db, %d degrees\n",i,
				FDAT.dwAddSinFreq[i],FDAT.dAddSinDb[i],FDAT.dwAddSinPhase[i]);
		}
	}

	// split at no sound, or  delete no sound part in head
	if(FDAT.dwNoSndMode != NOSND_NOT){
		
		fprintf(stderr,"\n");
		
		if(FDAT.dwNoSndMode == NOSND_HEADCUTONLY){
			fprintf(stderr,"headcutting: level = %6.2lf dB\n",FDAT.dNoSndBoundHead);
		}
		else{
			fprintf(stderr,"nosound: mode %d\n",FDAT.dwNoSndMode-1);
			fprintf(stderr,"%d msec, sound level = %6.2lf dB, nosound leve = %6.2lf dB\n",
				FDAT.dwNoSndTime,FDAT.dNoSndBoundHead,FDAT.dNoSndBound);
			fprintf(stderr,"suspend time = %d msec, ",FDAT.dwNoSndSuspend);
			fprintf(stderr,"mode 1 to 2 = %d msec\n",FDAT.dwNoSndM1toM2);
			fprintf(stderr,"stop recording = %d msec\n",FDAT.dwNoSndRecStop);
		}
		
		// no normalizer
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;
		
		// no fade out
		DbFadeOutTime = 0;
		
		// FIR filter befor nosound
		if(FDAT.dwNoSndFIRFilter != NO_FILTER){

			fprintf(stderr,"FIR filter before nosound: ");
			
			switch(FDAT.dwNoSndFIRFilter){
			case LPF: fprintf(stderr,"LPF\n"); break;
			case HPF: fprintf(stderr,"HPF\n"); break;
			case BPF: fprintf(stderr,"BPF\n"); break;
			case BSF: fprintf(stderr,"BSF\n"); break;
			}
			
			fprintf(stderr,"length = %d, ",FDAT.dwNoSndFIRleng);
			fprintf(stderr,"delay = %d(%.2lf msec), "
				,(FDAT.dwNoSndFIRleng-1)/2,(double)(FDAT.dwNoSndFIRleng-1)/2*1000/InputWaveFmt.rate());
			fprintf(stderr,"loss = %.2lf db, ",FDAT.dNoSndFIRDb);
			if(FDAT.dwNoSndFIRCutLow > FDAT.dwNoSndFIRCutHigh) 
				FDAT.dwNoSndFIRCutHigh = FDAT.dwNoSndFIRCutLow;
			
			if(FDAT.dwNoSndFIRFilter == BPF || FDAT.dwNoSndFIRFilter == BSF){
				fprintf(stderr,"%d hz - ",FDAT.dwNoSndFIRCutLow);
				fprintf(stderr,"%d hz\n",FDAT.dwNoSndFIRCutHigh);
			}
			else fprintf(stderr,"%d hz\n",FDAT.dwNoSndFIRCutLow);
		}
		
	}
	else FDAT.dwNoSndFIRFilter = NO_FILTER;  // don't use FIR filter

	//--------------

	// split file at specified time or byte
	if(FDAT.bSplit){
		
		if(FDAT.dSplitTime[DwCurSplitNo]){  // -split1,2,3
			FDAT.n64SplitByte = (LONGLONG)((double)InputWaveFmt.avgbyte()*FDAT.dSplitTime[DwCurSplitNo]);
		} 

		// no nomilizer
		DwNormalMode = NORMAL_NOT;
		DbNormalLevel = 0;
		
		// no fade out
		DbFadeOutTime = 0;
	
		fprintf(stderr,"split file: %.2lf sec (%lu M)\n",
		(double)FDAT.n64SplitByte/InputWaveFmt.avgbyte(),
		FDAT.n64SplitByte/1024/1024);
	}

	//fade in/out
	FDAT.dwFadeIn = 0;
	FDAT.dwFadeOut = 0;
	if(DbFadeInTime > 0 || DbFadeOutTime > 0){

		fprintf(stderr,"\n");
		
		if(DbFadeInTime > 0){
			FDAT.dwFadeIn  = InputWaveFmt.block()*(DWORD)(DbFadeInTime*(double)WriteWaveFmt.rate());
			fprintf(stderr,"fade in: %lf sec\n",DbFadeInTime);
		}
		if(DbFadeOutTime > 0){
			FDAT.dwFadeOut = InputWaveFmt.block()*(DWORD)(DbFadeOutTime*(double)WriteWaveFmt.rate());
			fprintf(stderr,"fade out: %lf sec\n",DbFadeOutTime);
		}
		
		if(FDAT.dwFadeIn > N64TotalDataSize) FDAT.dwFadeIn = (DWORD)N64TotalDataSize;
		if(FDAT.dwFadeOut > N64TotalDataSize) FDAT.dwFadeOut = (DWORD)N64TotalDataSize;
	}

#ifdef USEWIN32API
	// check space of HDD
	if(!BlStdout && !BlNoChkHdd && strcmp(SzWriteFile,"null")!=0){

		GetFreeHddSpace64(&n64HddSize,SzWriteFile);

		n64Foo = (ULONGLONG)((double)(N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1])
			*((double)WriteWaveFmt.avgbyte()/InputWaveFmt.avgbyte()));

		if(n64HddSize < WAVEHDRSIZE()+n64Foo){
			fprintf(stderr,"\nSpace of hdd (=%d Mbyte) is not enough.\n",n64HddSize/1024/1024);
			return false;
		}
	}
#endif

	/////////////////////

	WaveFormat wavfmt = InputWaveFmt;

	// DC offset
	if( CONFIG::get().use_dcoffset ){
		DcOffset* dcoffset = new DcOffset( wavfmt );
		dcoffset->set_offset( 0, CONFIG::get().dcoffset );
		wavfmt = dcoffset->get_output_format();
				
		filters.push_back( dcoffset );
	}

	// setup blocks
	blockdata.resize(DwCopyBlock);
	for( unsigned int i = 0; i < DwCopyBlock; ++i ){

		if( BlStdin ) blockdata[i].input_type = TYPE_STDIN;
		else if( BlNoSignal ) blockdata[i].input_type = TYPE_NULL;
		else blockdata[i].input_type = TYPE_STORAGE;

		blockdata[i].offset = N64OffsetBlk[i];
		blockdata[i].points = N64DataSizeBlk[i]/InputWaveFmt.block();
	}

	// shift
	if( DbShiftTime > 0 ){

		// tail
		BlockData tail;
		tail.input_type = TYPE_NULL;
		tail.offset = 0;
		tail.points = (unsigned int)(DbShiftTime * WriteWaveFmt.avgbyte() / 1000.0)/WriteWaveFmt.block();
		blockdata.push_back( tail );
	}
	
	fprintf(stderr,"\nstream of data:\n");
	fprintf(stderr,"INPUT\n",SzReadFile);

	std::vector<Filter*>::iterator it = filters.begin();
	for( ; it != filters.end(); ++it ){
		fprintf(stderr,"=> ");
		(*it)->show_config();
	}

	fprintf(stderr,"=> OUTPUT");

    fprintf(stderr,"\n\nOUTPUT: ");
    if(BlStdout){
        if(BlWaveOut) fprintf(stderr,"[waveout]\n");
        else fprintf(stderr,"[stdout]\n");
    }
    else fprintf(stderr,"%s\n",SzWriteFile);
	return true;
}




//-------------------------------------------------------
// filter body
BOOL FilterBody()
{
	if(BlFileInfo) return true;
	
	double* lpFilterBuf[2] = {NULL,NULL}; 	// LR-buffer for filter(double)
	DWORD dwFilterBufSize2; // points, size of buffer 
	BYTE* lpBuffer = NULL; // buffer for fread()
	BYTE* lpWriteBuffer = NULL; // buffer for fwrite()
	DWORD dwOutBufSize; // buffer size of lpWriteBuffer

	// hadle of files
	FILE* hdReadFile = NULL;
	FILE* hdWriteFile = NULL;

	double dNormalGain[2] = {0,0}; // gain for normalizer
	LONGLONG n64PointerStdin;  // byte, current file pointer of stdin

	// ,and others
	DWORD dwFoo;
	BOOL bReturn = false; // return value
	DWORD dwWriteByte;  // write size of data returned after fwrite()
	DWORD dwSeed; // for seed of rand
	char szErr[CHR_BUF];

	TrackManager trackmanager;
	Track track(0, InputWaveFmt);

	//--------------------------
	// initializing
	
	dwSeed = timeGetTime();


	// in case of Ctrl+C 
#ifdef USEWIN32API
	SetConsoleCtrlHandler(ExitCtrlC,TRUE);
#else
	signal(SIGINT,ExitCtrlC);
#endif

	// buffer size of filter
	dwFilterBufSize2 = DwBufSize/InputWaveFmt.block();
	dwFoo = 1; while(dwFoo<dwFilterBufSize2) dwFoo = dwFoo << 1; // set align = 2^x  (for 24 bit file)
	dwFilterBufSize2 = dwFoo;

	if(!InitFilters(&FDAT,
		dwFilterBufSize2,
		InputWaveFmt,
		WriteWaveFmt,
		szErr)){
		fprintf(stderr,szErr);
		goto L_ERR;
	}

	// write characteristics of FIR filter, then exit
	if(BlFIROutFilter){

		if(FDAT.dwNoSndFIRFilter != NO_FILTER) dwFoo = ID_FIR_NOSND;
		else if(FDAT.dwFIRFilter != NO_FILTER) dwFoo = ID_FIR_NORMAL;
		else if(FDAT.bFIREQ) dwFoo = ID_FIR_EQ;

		OutputFIRFilterChr(SzCurDir,InputWaveFmt,dwFoo);
		bReturn = true;
		goto L_ERR;
	}

	// write characteristics of IIR filter, then exit
	if(BlIIROutFilter){
		
		if(FDAT.dwIIRFilter != NO_FILTER) dwFoo = ID_IIR_NORMAL;
		else if(FDAT.bDemp) dwFoo = ID_IIR_DEMP;
		else if(FDAT.bSVEQL) dwFoo = ID_IIR_SVEQL;
		else if(FDAT.bSVEQH) dwFoo = ID_IIR_SVEQH;
		else if(FDAT.bPKEQ) dwFoo = ID_IIR_PKEQ;

		printIIRcoef(dwFoo);
		OutputIIRFilterChr(SzCurDir,InputWaveFmt,dwFoo);
		bReturn = true;
		goto L_ERR;
	}
	

	if(BlRsmpOutFilter){
		OutputRsmpChr(SzCurDir,InputWaveFmt);
		bReturn = true;
		goto L_ERR;
	}

	// create buffer for read and write file
	lpBuffer = (BYTE*)malloc(DwBufSize+1024); 
	dwOutBufSize = DwBufSize;
	if(FDAT.bRsmp) dwOutBufSize *= 2; // size *= 2 when re-sampling
	if(WriteWaveFmt.bits() > InputWaveFmt.bits()){

		if(InputWaveFmt.bits() == 8){
			if(WriteWaveFmt.bits() == 24) dwOutBufSize *= 4;
			else dwOutBufSize *= WriteWaveFmt.bits()/InputWaveFmt.bits();
		}
		if(InputWaveFmt.bits() == 16){
			if(WriteWaveFmt.bits() == 24) dwOutBufSize *= 2;
			else dwOutBufSize *= WriteWaveFmt.bits()/InputWaveFmt.bits();
		}
		else if(InputWaveFmt.bits() == 24){
			if(WriteWaveFmt.bits() == 32) dwOutBufSize *= 2;
			else dwOutBufSize *= 4;
		}
		else dwOutBufSize *= WriteWaveFmt.bits()/InputWaveFmt.bits();

	}
	lpWriteBuffer = (BYTE*)malloc(dwOutBufSize+1024); 

	// create buffer for filter
	dwFoo = dwFilterBufSize2;
	if(FDAT.bRsmp) dwFoo *= 2; // size *= 2 when re-sampling
	for(unsigned int i=0;i<InputWaveFmt.channels();i++) lpFilterBuf[i] = (double*)malloc(sizeof(double)*dwFoo+1024); 

	// open input file
	if(!OpenReadFile(&hdReadFile,SzReadFile)){
		goto L_ERR;
	}

	track.fp = hdReadFile;
	track.raw = lpBuffer;
	track.raw_max_points = DwBufSize / InputWaveFmt.block();
	for(unsigned int i=0; i < InputWaveFmt.channels(); i++) track.data[i] = lpFilterBuf[i];
	track.data_max_points = dwFoo / WriteWaveFmt.block();
	track.set_verbose(BlVerbose);
	track.set_filters( filters );
	track.set_blockdata( blockdata );
	track.set_filename( SzReadFile );
	trackmanager.tracks.push_back( track );

	
	// open output file
	if(!OpenWriteFile(&hdWriteFile,SzWriteFile)){
		goto L_ERR;
	}

#ifdef USEWIN32API
	// open wave device
	if(BlWaveOut){
		if(!OpenWaveDevice(UnWaveDevID,WriteWaveFmt,dwOutBufSize+1024)){
			goto L_ERR;
		}
	}
#endif

	// if input is stdin and -headofs is specified, move file pointer of stdin here in advance.
	if(BlStdin && BlHeadOffset)	SeekStdin(lpBuffer,DwBufSize,N64FileOffset,0);
	n64PointerStdin = N64FileOffset;

	if(BlWaveHdrOut){

		// if output file is stdout and not waveout, write wave header here
		if(!BlWaveOut){ // not waveout
			
			if(BlStdout)
			{
				WriteWaveFmt.write(hdWriteFile, N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1]);
			}
			else if(hdWriteFile != NULL){
				// move file pointer of output file
				__int64 pos64 = WAVEHDRSIZE();
				_fseeki64( hdWriteFile , pos64, SEEK_SET);
			}
			
		}
	}

	// add space to head
	AddSpace(hdWriteFile,DwAddSp[0]);

	//-------------------------------------------------------------------
	//-------------------------------------------------------------------
	//-------------------------------------------------------------------


	bool normalizer_searching = false;

	//-------------------
	// initialize normalizer
	if(DwNormalMode != NORMAL_NOT){

		fprintf(stderr,"\n");
		
		if(DwNormalMode == NORMAL_PEAKBYDB)
			fprintf(stderr,"normalizer (peak): level = %5.3lf dB\n",DbNormalLevel);
		if(DwNormalMode == NORMAL_AVG)
			fprintf(stderr,"normalizer (average): level = %5.3lf dB\n",DbNormalLevel);
		if(DwNormalMode == NORMAL_RMS)
			fprintf(stderr,"normalizer (RMS): level = %5.3lf dB\n",DbNormalLevel);
		
		normalizer_searching = true;
	}

	dNormalGain[0] = dNormalGain[1] = 0;
	DWORD dwCurrentNormalMode = DwNormalMode; // current mode of normalizer

	bool mainloop = true;
	while(mainloop){ // get back here when normalizer finished search of peak.

		//-----------------------------
		// global initialization
		//-----------------------------

		unsigned long long total_out_size = 0;// total out size  (byte)
		unsigned long long total_realout_size = 0; // total 'real' out size  (byte)
		// NOTE: total_realout_size == total_out_size unless resampling is execued.
	
		srand(dwSeed);

		// obsolete
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
		dwFoo = (DWORD)(DbShiftTime * WriteWaveFmt.rate() / 1000);
		DWORD dwCutHeadOutPoints = dwFoo; // points for shiftting(-shift ). unless dwCutHeadOutPoints >0 , cut the head of output



		std::vector<Filter*>::iterator it_filter = filters.begin();
		for( ; it_filter != filters.end(); ++it_filter ) (*it_filter)->clear_buffer();

		trackmanager.start();

		while(1){

			//-----------------------------
			// INPUT
			//-----------------------------
			trackmanager.read();
			if( trackmanager.end_of_tracks() ) break;

			//------------------------------
			// filtering
			//-----------------------------
			it_filter = filters.begin();
			for( ; it_filter != filters.end(); ++it_filter ) (*it_filter)->process(trackmanager);


			// obsolete
			unsigned int points = trackmanager.get_track(0).raw_points;
			BOOL bChangeFile = false; // if true, change output file

			if(FDAT.bSplit){

				SPLIT(InputWaveFmt, 
					&points,
					total_out_size,
					DwCurSplitNo,
					&bChangeFile,
					&FDAT.n64SplitByte,

					FDAT.dSplitTime,
					FDAT.n64SplitByteMalti);

				if( bChangeFile ) trackmanager.exec_split( points );
			}

			unsigned int points_before_resampling = points;

/*
			WFLT_FILTER(
				&FDAT,  

				lpFilterBuf,  // buffer
				&points, // points
				points_before_resampling,

				&bChangeFile,
				dwCurrentNormalMode,
				dNormalGain,
				DwCurSplitNo,

				total_out_size,
				N64TotalDataSize,
				InputWaveFmt,
				WriteWaveFmt);
*/

				//-----------------------------
				// OUTPUT
				//-----------------------------
			
					if( !normalizer_searching)
					{
						
						// shiftting. (cut some data from the head of output.)
						if(dwCutHeadOutPoints){
							if(dwCutHeadOutPoints > points){
								dwCutHeadOutPoints -= points;
								points = 0;
								points_before_resampling = 0;
							}
							else
							{
								points -= dwCutHeadOutPoints;
								points_before_resampling = (unsigned int)((double)dwCutHeadOutPoints/WriteWaveFmt.rate()*InputWaveFmt.rate());
								for(unsigned int i=0;i<WriteWaveFmt.channels();i++)
									memmove(lpFilterBuf[i],lpFilterBuf[i]+dwCutHeadOutPoints,sizeof(double)*points);
								dwCutHeadOutPoints = 0; // not do shiftting anymore
							}
						}
						
						if(points){
							
							if(BlTextOut){ // text out
								WriteTextData(hdWriteFile,lpFilterBuf,points,WriteWaveFmt);
							}
							else
							{
								
								CopyBufferDtoB(lpWriteBuffer,lpFilterBuf,
									points,
									WriteWaveFmt,
									FDAT.bDither,
									
									&DwSaturate
									);
								
								// write to the output file
								if(hdWriteFile){
									
									dwWriteByte = WriteData(hdWriteFile, lpWriteBuffer, points*WriteWaveFmt.block() );
									if( dwWriteByte != points*WriteWaveFmt.block() ){
										// if fail, then write header and exit.
										if(BlWaveHdrOut && !BlStdout) 
											WriteWaveFmt.write(hdWriteFile,total_realout_size+DwAddSp[0]);
										goto L_ERR;
									}
									
									
								}
#ifdef USEWIN32API
								// waveout
								else if( BlWaveOut )
								{
									dwWriteByte = points*WriteWaveFmt.block();
									PlayWave(lpWriteBuffer,dwWriteByte);
								}
#endif
							}
							
						}

					}
						
						total_out_size += points_before_resampling * InputWaveFmt.block();
						total_realout_size += points * WriteWaveFmt.block();
						
						if(BlVerbose){

							double peak_tmp = 0;

							if( !normalizer_searching ){
							double dPeak[2];
							GET_PEAK(dPeak);
							peak_tmp = max(dPeak[0],dPeak[1]);
							peak_tmp = 20*log10(fabs(peak_tmp));
							}
							
							ShowStatus(WriteWaveFmt,SzRealWriteFile,
								total_realout_size,
								N64RealTotalDataSize,
								peak_tmp,
								normalizer_searching
								);
						}
						
		
						//-----------------------------
						// change output file
				     	//-----------------------------

						if( !normalizer_searching && bChangeFile)
						{
/*
							it_track = tracks.begin();
							for( ; it_track != tracks.end(); ++it_track ) (*it_track).exec_split( points_before_resampling );
*/
							// add space to tail
							AddSpace(hdWriteFile,DwAddSp[1]);
							
							// write wave header
							if(BlWaveHdrOut && !BlStdout) WriteWaveFmt.write(hdWriteFile, total_realout_size+DwAddSp[0]+DwAddSp[1]);
							
							// close output file handle
/*
#ifdef USEWIN32API
							if(hdWriteFile != NULL) CloseHandle(hdWriteFile);
							hdWriteFile = NULL;
#else
							*/
							if(hdWriteFile != NULL) fclose(hdWriteFile);
							hdWriteFile = NULL;
//#endif
			
							/* obsolete
#ifdef USEWIN32API
							
							// set timestamp
							if(BlFileStamp && !BlNoSignal)
							{
								HANDLE hTimeFile;
								hTimeFile = CreateFile(SzRealWriteFile,GENERIC_WRITE,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
								if(hTimeFile)
								{
									SetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
									CloseHandle(hTimeFile);
									Sleep(50);
								}
							}
#endif	
							*/


							//---------------------------------------
							
							// open new file
							
							// initialize
							if(N64RealTotalDataSize > 0)
							{
								N64TotalDataSize -= total_out_size;
								N64RealTotalDataSize -= total_realout_size;

								/*obsolete
								// if data size < 4G, then don't use extra chunk
								n64Foo = N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1];
								if(n64Foo <= 0xFFFFFFFF-44) BlExtChunkOfHdr = false;
								*/
							}

							total_out_size = 0;
							total_realout_size = 0;
							bChangeFile = false;
							ClearNOSOUND(); 

							std::vector<Filter*>::iterator it = filters.begin();
							for( ; it != filters.end(); ++it ){
								(*it)->output_changed();
							}

							// get current system time
							GetLocalTime(&SystemTime);
							
							// set new name of output file
							DwCurSplitNo++;
							if(strcmp(SzOrgWriteFile,"null")!=0)
								SetOutputFileName(SzOrgWriteFile,SzWriteFile,DwCurSplitNo);
							else strcpy(SzWriteFile,"null");

							SetCommandStrings(false,SzWriteFile,SzWriteFile,SzWriteFile,SzUserDef[0],SzUserDef[1],SzUserDef[2],SystemTime,WriteWaveFmt);
							strcpy(SzRealWriteFile,SzWriteFile);
			
							
							// open new output file
							if(!OpenWriteFile(&hdWriteFile,SzWriteFile))
							{
								goto L_ERR;
							}
							
							if(BlWaveHdrOut)
							{
								if(!BlWaveOut)
								{
									// write wave header if output is stdout
									if(BlStdout)
									{ 
										WriteWaveFmt.write(hdWriteFile, N64RealTotalDataSize+DwAddSp[0]+DwAddSp[1]);
									}
									else if(hdWriteFile != NULL)
									{
										// move file pointer
										__int64 pos64 = WAVEHDRSIZE();
										_fseeki64( hdWriteFile, pos64, SEEK_SET);
									}

								}
							}
				
							/* obsolete
#ifdef USEWIN32API
							// get local time
							if(BlFileStamp && !BlNoSignal)
							{
								SYSTEMTIME sysTime;
								FILETIME ftLocal;
								GetLocalTime(&sysTime);
								SystemTimeToFileTime(&sysTime,&ftLocal);
								LocalFileTimeToFileTime(&ftLocal,&FtCreationTime);
								FtLastAccessTime = FtCreationTime;
								FtLastWriteTime = FtCreationTime;
								fprintf(stderr,"set timestamp to: %d/%d/%d %d:%02d:%02d\n",
									sysTime.wYear,sysTime.wMonth,sysTime.wDay,
									sysTime.wHour,sysTime.wMinute,sysTime.wSecond
									);
							}
#endif	
							*/
							
							fprintf(stderr,"\n\nOUTPUT changed\n");
							fprintf(stderr,"new OUTPUT: ");
							if(BlStdout){
									fprintf(stderr,"[stdout]\n");
							}
							else fprintf(stderr,"%s\n",SzWriteFile);
							
							// add space to head
							AddSpace(hdWriteFile,DwAddSp[0]);

					}  
					// change output file
					//--------------------------------------
			/*  obsolete	
				}
			
				else  
				{
					// now, normalizer is searching the peak of output.
					block_total_out_size += points_before_resampling * InputWaveFmt.block();
					block_total_realout_size += points * WriteWaveFmt.block();

					if(BlVerbose)
						ShowStatus(WriteWaveFmt,SzRealWriteFile,
						total_realout_size+block_total_realout_size,
						N64RealTotalDataSize,
						0,
						true
						);

				}
				*/
				
			
			
		}
		// while(1)  
		
//	}
	
	// get gain for normalizer
	if( normalizer_searching )
	{
		GetGainForNormalizer(dNormalGain,BlNormalStereo,dwCurrentNormalMode,DbNormalLevel,total_realout_size,WriteWaveFmt);

		if(dwCurrentNormalMode != NORMAL_PEAKBYDB) FDAT.bNormalUseCompressor = true;

		// use compressor
		if(FDAT.bNormalUseCompressor){
			fprintf(stderr,"limiter after normalizer:\n");
			fprintf(stderr,"ratio = %4.1lf, threshold = %6.3lf dB\n",FDAT.dNormalRatio,FDAT.dNormalTh);
			fprintf(stderr,"attack = %4.0lf msec, release = %4.0lf msec, RMS = %d\n",
				FDAT.dNormalAttack,FDAT.dNormalRelease,FDAT.dwNormalCompRMS);
		}

		dwCurrentNormalMode = NORMAL_EXEC;
		normalizer_searching = false;
	}
	else{

		// add space to tail
		AddSpace(hdWriteFile,DwAddSp[1]);

		// write wave header
		if(BlWaveHdrOut && !BlStdout) WriteWaveFmt.write(hdWriteFile,total_realout_size+DwAddSp[0]+DwAddSp[1]);

		mainloop = false;	
	}

	} // while(mainloop)


	//-----------------------------------------------------------------------
	//-----------------------------------------------------------------------


	fprintf(stderr,"\n");

	// close file handle of output
	/*
#ifdef USEWIN32API
	if(hdWriteFile != NULL) CloseHandle(hdWriteFile);
	hdWriteFile = NULL;
#else
	*/
	if(hdWriteFile != NULL) fclose(hdWriteFile);
	hdWriteFile = NULL;
//#endif
	
	/* obsolete
#ifdef USEWIN32API
	
	// set timestamp
	if(BlFileStamp && !BlNoSignal)
	{
		HANDLE hTimeFile;
		hTimeFile = CreateFile(SzRealWriteFile,GENERIC_WRITE,0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
		if(hTimeFile)
		{
			SetFileTime(hTimeFile,&FtCreationTime,&FtLastAccessTime,&FtLastWriteTime);
			CloseHandle(hTimeFile);
			Sleep(50);
		}
	}
#endif	
	*/
	
	if(BlVerbose){
		std::vector<Filter*>::iterator it = filters.begin();
		for( ; it != filters.end(); ++it ){
			(*it)->show_result();
		}
	}

	if(FDAT.bAutoOffset && BlVerbose){
		
		double dOffset[2],dMaxLevel;
		GetAutoOffset(dOffset);		
		
		dMaxLevel = InputWaveFmt.GetMaxWaveLevel();
		
		fprintf(stderr,"\nauto DC offset: ");
		fprintf(stderr,"L =  %6.3lf dB (%.2lf), ",-20*log10(fabs(dOffset[0])),-dOffset[0]*dMaxLevel);
		fprintf(stderr,"R =  %6.3lf dB (%.2lf)\n",-20*log10(fabs(dOffset[1])),-dOffset[1]*dMaxLevel);
	}

	double peak_tmp[2];
	GET_PEAK(peak_tmp);
	fprintf(stderr,"peak: L = %6.3lf dB (%6.3lf), R = %6.3lf dB (%6.3lf)\n",
		20*log10(peak_tmp[0]),peak_tmp[0],20*log10(peak_tmp[1]),peak_tmp[1]);
	if(DwSaturate > 0) fprintf(stderr,"peak over: %d times.\n",DwSaturate);

	bReturn = true;

	//----------------------------------------------------

	// if error occured, jump here 
L_ERR: 

	UnprepareAllFilters();
	unprepareOutputFileName();

	// if input is stdin, wait for EOF
	if(BlStdin && bReturn) while(SeekStdin(lpBuffer,DwBufSize,DwBufSize,0));

	// close file handles
#ifdef USEWIN32API
//	if(hdReadFile) CloseHandle(hdReadFile);
//	if(hdWriteFile) CloseHandle(hdWriteFile);
	CloseWaveDevice(!bReturn);
#endif
	if(hdReadFile) fclose(hdReadFile);
	if(hdWriteFile) fclose(hdWriteFile);
	
	// free mem
	if(lpBuffer) free(lpBuffer);
	if(lpWriteBuffer) free(lpWriteBuffer);
	for(unsigned int i=0;i<2;i++) if(lpFilterBuf[i]) free(lpFilterBuf[i]);

	return bReturn;
}


void DeleteInputFile()
{
	// -del
	if(BlDelFile && !BlStdin){

		if(remove(SzReadFile) == 0){
			fprintf(stderr,"delete %s\n",SzReadFile);
		}
		else{
			std::string errmsg( "cannot delete " );
			errmsg += SzReadFile;
			errmsg += " : No such file";
			throw errmsg;
		}
	}
}
