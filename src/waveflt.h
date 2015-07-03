// header of filters

#ifndef _WFLT_FILTER_H
#define _WFLT_FILTER_H

#ifdef WIN32
#define USEWIN32API  // use win32 APIs
#include <windows.h>
//#include <fcntl.h>  // _setmode
//#include <io.h>
#endif

#include <stdio.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "waveformat.h"

/*
#define VER_STR "WAVEFLT ver.2015.06.25 (c)1999-2015 T.Tokairin"
#define VER_NUM  110
*/

#define MAX_CHN		6   // max number of channel

#define QUANTIZATION(a,b) ((LONG)(a+b))

#define DEF_WAVEFLT

#define MAX_ARGC 255
#define CHR_BUF 256 

#define MAXCOPYBLOCK 64   // max size of blocks
#define MAX_WAVFLTOPTBUF 4096 // buffer size for reading options

#ifndef	BOOL
#define BOOL int
#endif


#ifndef	VOID
#define VOID void
#endif

#ifndef true
#define  true   1
#define  false  0
#endif

/* obsolete
// ID of MDCT 
#define ID_MDCT_ADPMDCT 0


// ID of FFT
#define ID_FFT_ADPMDCT 0
*/


/* obsolete
#ifdef WFLT2DLL_EXPORTS
#define	EXPORT __declspec(dllexport) WINAPI
#else
#define	EXPORT WINAPI
#endif
*/

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 0x0001
#endif
#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif


#define MAX_SPLIT	128 // max of split number
#define MAX_SINE_WAVES	16 // max of sine waves
#define MAXMDCTFILTERPOINT 8192  // max of  MDCT filter length
#define MAXFILTERPOINT	8191  // max of  FIR filter length
#define EQBANDS	10 // max band of EQ 
#define MAX_EQDB	24 // dB, max level of EQ

#define WAVEHDRSIZE(a) ( (a == true) ? 44 + (8 + 8 + 4) : 44)

// type of filter
#define NO_FILTER	0
#define LPF 1
#define HPF 2
#define BPF 3
#define BSF 4
#define SVEQL 5  // shelving EQ low
#define SVEQH 6  // shelving EQ high


// mode of nosound
#define NOSND_NOT			0	// no split
#define NOSND_EXEC			1	// searching no sound part
#define NOSND_SEARCH		2	// searching sound part after splitting, then move to NOSND_EXEC
#define NOSND_CUT			3	// deleting no sound part after splitting, then move to NOSND_EXEC
#define NOSND_HEADCUTONLY	4	// deleting no sound part,then move to NOSND_NOT 
#define NOSND_STOP			5	// stop recording of lockon if no sound part is continuing

// type of normalizer
#define NORMAL_NOT			0
#define NORMAL_EXEC			1  // execute normalizer
#define NORMAL_PEAKBYDB		2  // searching peak
#define NORMAL_AVG			3  // searching average
#define NORMAL_RMS			4  // searching RMS


// ID of FIR filter
#define ID_FIR_NORMAL	0  // FIR
#define ID_FIR_EQ		1  // FIR-EQ
#define ID_FIR_NOSND	2  // FIR before nosound


// ID of IIR filter
#define ID_IIR_NORMAL	0  // IIR
#define ID_IIR_DEMP		1  // IIR-(de)emphasys
#define ID_IIR_SVEQL	2  // IIR-shelving EQ low
#define ID_IIR_SVEQH	3  // IIR-shelving EQ high
#define ID_IIR_PKEQ		4  // IIR-peaking EQ

#ifndef COMPLEX
typedef struct
{
	double r;
	double i;
}COMPLEX,*LPCOMPLEX;
#endif


//----------------------------------------
// parameters
// note: you need to call SetDefaultOption() to set default value before filtering.

typedef struct
{
/*	
	// DC offset adjustment
	BOOL bOffset; 
	double dOffset[2]; // offset value (left,right)
*/
	// Auto DC offset adjustment
	BOOL bAutoOffset; 
	DWORD dwAutoOffsetTime; // sec, training time

	// split at no sound part
	DWORD dwNoSndMode; // initial mode. if NOSND_NOT, don't split
	DWORD  dwNoSndTime; // msec, time of no sound part
	double dNoSndBoundHead; // db, level of sound. when level of input value execeed this value, data goes into the sound part.
	double dNoSndBound; // db, level of no sound, when level of input value is under this value, data goes into the no sound part.
	DWORD dwNoSndSuspend; // msec. when split mode enter NOSND_EXEC, don't split file in dwNoSndSuspend msec.
	DWORD dwNoSndM1toM2; // msec. when time of no sound execeed this, change mode from NOSND_SEARCH to NOSND_CUT.
	DWORD dwNoSndRecStop; // msec. when time of no sound execeed this, stop recording of 'lockon'

	// FIR filter befor split
	DWORD dwNoSndFIRFilter; // type of filter, if NO_FILTER, don't use FIR
	DWORD dwNoSndFIRCutLow; // cut-off (low)
	DWORD dwNoSndFIRCutHigh; // cut-off (high)
	DWORD dwNoSndFIRleng; // length
	double dNoSndFIRDb; // db, loss of filter

	// split at specified size
	BOOL bSplit; 
	LONGLONG n64SplitByte; // byte,  if output size execeed this value, then split the file.
	double dSplitTime[MAX_SPLIT]; // sec, times
	LONGLONG n64SplitByteMalti[MAX_SPLIT]; // byte, size specified -splitbm 

	
	// LR-mixing
	BOOL bMixLR; 
	double dMixLRLevel; // level 0 << 1

	
	// file mixing
	BOOL bMixFile;
	char szMixFile[MAX_PATH];
	double dMixLevel[2]; // level 0< < 1
	double dMixStartTime[2]; // sec

	
	// synthesize sine waves
	DWORD dwAddSinNum;  // if = 0, don't synthesize
	DWORD dwAddSinFreq[MAX_SINE_WAVES];
	DWORD dwAddSinPhase[MAX_SINE_WAVES];
	double dAddSinDb[MAX_SINE_WAVES];

/* obsolete	
	// ADP
	BOOL bADP;
	DWORD dwADPDelayTime; // msec, delay time 
	double dADPlevel; // level
	DWORD dwADPleng; // length
	double dADPDb;  // db, loss of adp filter
*/
	
	// FIR 
	DWORD dwFIRFilter; // type of filter, if NO_FILTER, don't use FIR
	DWORD dwFIRCutLow; // cut-off (low)
	DWORD dwFIRCutHigh; // cut-off (high)
	DWORD dwFIRleng; // length
	double dFIRDb; // db, loss of filter

	
	// FIR-EQ
	BOOL bFIREQ;
	DWORD dwEQband; // bands
	double dEQLevel[EQBANDS]; // dB, level
	DWORD dwEQleng; // length
	double dEQDb; // db, loss of filter
	double dEQ_Q;  // Q  

	
	// IIR 
	DWORD dwIIRFilter;  // type of filter. if NO_FILTER, don't filtering
	DWORD dwIIRCutLow; // cut-off freq (low)
	DWORD dwIIRCutHigh; // cut-off freq (high)

	// (de)emphasys
	BOOL bDemp;
	double dDempDb;  // db, 10 or -10 , t1 = 0.000050, t2 = 0.000015 are const.

	// shelving EQ low
	BOOL bSVEQL;
	double dSVEQLdb;
	DWORD dwSVEQLfreq;

	// shelving EQ high
	BOOL bSVEQH;
	double dSVEQHdb;
	DWORD dwSVEQHfreq;

	// peaking EQ
	BOOL bPKEQ;
	double dPKdb;
	double dPKQ;
	DWORD dwPKfreq;

	/* obsolete
	// waveshrink filter
	BOOL bWaveSh;
	double dWaveShLevel;
	double dWaveShLowLevel;
	double dWaveShHiLevel;
	DWORD dwWaveShleng; // length
	*/

	// noise gate
	BOOL bNgate;
	double dNgateTh;   // db, threshold
	double dNgateRelease; // msec
	double dNgateAttack; // msec
	DWORD dwNgateRMS;  // RMS
	
	// phase invert
	BOOL bPhaseInv;
	
	// compressor 
	BOOL bComp;
	double dCompRatio; // ratio
	double dCompTh; // threshold (db)
	double dCompAttack; // attack time (msec)
	double dCompRelease; // release time (msec)
	DWORD dwCompRMS;  // RMS flame length 
	
	
	// limiter atter normaizing
	BOOL bNormalUseCompressor;  
	double dNormalRatio; // ratio
	double dNormalTh; // threshold (db)
	double dNormalAttack; // attack time (msec)
	double dNormalRelease; // release time (msec)
	DWORD dwNormalCompRMS; // RMS flame length 
	
	// volume
	double dVolume; // >= 0, if =1.0 , don't adjust gain
	
	// balance
	BOOL bBalance;
	double dBalance[2]; // L-R, >= 0

	
	// fade in/out
	DWORD dwFadeIn; // byte, size of fade in
	DWORD dwFadeOut; // byte, size of fade out
	
	
	// re-sampling
	BOOL bRsmp;
	DWORD dwRsmpInFreq; // input freq
	DWORD dwRsmpOutFreq; // output freq
	DWORD dwRsmpUp;  // up-sampling  points
	DWORD dwRsmpDown; // down-sampling points after up-sampling
	DWORD dwRsmpLn; // length of FIR-LPF filter
	DWORD dwRsmpCut; // cut-off freq of FIR-LPF filter
	double dRsmpDB; // db, loss of FIR-LPF filter

	
	// change bits of output
	DWORD dwBitChange; // = 0,8,16,32,64. if 0, don't change
	DWORD dwBitChangeTag; // tag
	
	// dither
	BOOL bDither;
	double dDitherAmp; // amp
	
}FILTER_DATA,*LPFILTER_DATA;



// filter.c
#if 0
void  CopyBufferBtoD(BYTE* lpBuffer,  // input, buffer (BYTE*)
						DWORD dwByte, // size of lpBuffer

						double* lpFilterBuf[2], // output, buffer of wave data (double*) (L-R)
						LPDWORD lpdwPointsInBuf, // points of data in lpFilterBuf
						WAVFMT waveFmt
						);

void CopyBufferDtoB(BYTE* lpBuffer,  // output, buffer(BYTE*)
						   
						   double* lpFilterBuf[2], // input, buffer(double*)
						   DWORD dwPointsInBuf, // points in lpFilterBuf
						   WAVFMT waveFmt,
   						   BOOL bRound,
						   
						   DWORD* lpdwSaturate  // number of saturation
						   );

void ClearAllFilters();
void UnprepareAllFilters();

BOOL InitFilters(LPFILTER_DATA lpFDat,
				 DWORD dwFilterPoints, // points of data in filter buffer
				 WAVFMT inWaveFmt,
				 WAVFMT outWaveFmt,
				 char* lpszErr);


void SetDefaultOption(LPFILTER_DATA lpFDat);

void WFLT_FILTER(LPFILTER_DATA lpFDat,  // parameter

				 double* lpFilterBuf[2],  // (input / output) buffer of wave data
				 LPDWORD lpdwPointsInBuf, // (input / output) data points in buffer
				 LPDWORD lpdwRealPointsInBuf, // (output) output data points
				 // note: if inWaveFmt.nSamplesPerSec = outWaveFmt.nSamplesPerSec,
				 // then *lpdwPointsInBuf = *lpdwRealPointsInBuf.

				 BOOL* lpbChangeFile, // if return value of *lpbChangeFile is true, change output file
 /* obsolete
				 BOOL bADPtrainMode, // if true , now ADP is in the training mode  
				 */
				 DWORD dwCurrentNormalMode, // current mode of normalizer
				 double dNormalGain[2], // L-R, gain for normalizer
				 DWORD dwCurFileNo,  // current file number

				 /* obsolete
				 HWND hWndLockon, // hwnd of 'lockon'
				 */

				 LONGLONG n64OutSize, // byte, total output size
				 LONGLONG n64DataSize, // byte, total data size of wave
				 WAVFMT inWaveFmt, // format of input
				 WAVFMT outWaveFmt // format of output
				 );
#endif



/* obsolete
//--------------------------------------------------
// adp.c
void PrepareADP(DWORD dwADPleng,  // length
				double dDB,   // loss
				double dbLevel, 
				DWORD dwBufferSize,
				DWORD dwChn
				);

void unprepareADP();
void ClearADPCoef();
void ADPLMS(double* lpFilterBuf, // buffer
			DWORD dwPointsInBuf, // points
			DWORD dwChn
			);


void ClearADPBuf();
DWORD GetADPCoef(double* coef[2]);

*/


//----------------------------
// conv.c
double CONV();
void InitCONV(DWORD dwLength);
void UnprepareCONV();
void SetConvSize(DWORD n);
double* GetConvBufA();
double* GetConvBufB();
BOOL  CheckSSE2(); // you need to call this function to use SSE2




//--------------------------------------------------
// fir.c
void FIR(double* lpFilterBuf, // filter buffer
		 DWORD dwPointsInBuf, // points of data in the filter buffer
		 DWORD dwFirNum, // number of filter
		 DWORD dwChn // channel
		 );


void prepareFIRCoefficient(DWORD dwFilter,  // type of filter
						   DWORD dwFIRleng, // length of filter
						double dbSmpRate,  // sampling rate
						double dDB,  // dB, loss
						DWORD dwCutLow,  // cut-off freq (low)
						DWORD dwCutHigh,  // cut-off freq (high)
						DWORD dwBufSize, // buffer size
						DWORD dwFirNum, // number of filter
						DWORD dwChn // channel
						);

void CalcEQCoefficient(DWORD dwFIRleng,
					   DWORD dwBand,  // band
					   double dbSmpRate, // sampling rate
					   double dDB,  // dB, loss
					   double* dbEQlevel,  // dB, levels
					   double dQ, // Q
					   DWORD dwBufSize, // buffer size
					   DWORD dwFirNum, // number of filter
					   DWORD dwChn // channel
					   );

void ClearFirBuf();
void unprepareFIR();
void RewindBufFIR(DWORD dwRewSize,DWORD dwFirNum,DWORD dwChn);

void CalcFirCoefficient(DWORD dwFilter,  // type of filter
						double * coeff, 

						DWORD dwLength,
						double dbSmpRate,  // sampling rate
						double dDB,  // loss
						DWORD dwCutLow,  // cut-off freq (low)
						DWORD dwCutHigh  // cut-off freq (high)
						);
DWORD GetFIRCoef(double** coef,DWORD dwFirNum,DWORD dwChn);




//--------------------------------------------------
// iir.c
void IIR(double* lpFilterBuf, // buffer
		 DWORD dwPointsInBuf, // points

		 DWORD dwIirNum,
		 DWORD dwChn
		 );


void prepareIIRCoefficient(DWORD dwFilter,// type of filter
						   double dCutFreqL, // low
						   double dCutFreqH, // hish
						   double dSmpRate,  // sampling rate
						   DWORD N,  // dimension

						   DWORD dwIirNum, // number of filter
						   DWORD dwChn // channel
						);

void CalcIirShelvingEQ(DWORD dwFilter,
					   double dCutFreq,
					 double dDb,
					 double dSmpRate,  // sampling rate
					 DWORD dwIirNum, // number of filter
					 DWORD dwChn // channel
					 );


void CalcIirPeakingEQ(double dCntFreq,  // center
					  double dQ,  // Q
					  double dDb,
					  double dSmpRate,  // sampling rate
					  DWORD dwIirNum, // number of filter
					  DWORD dwChn // channel
					  );

void ClearIirBuf();

void IIR2_Butterworth(DWORD dwFilter,// type of filter
					double a[3],
					double b[3],
					double k,
					double N,

					double dCutFreqL, // low
					double dCutFreqH, // hish
						
					double dSmpRate  // sampling rate
					);

void IIR4_BPF_Butterworth(double a[5],
					double b[5],
					double k,
					double N,

					double dCutFreqL, // low
					double dCutFreqH, // hish
						
					double dSmpRate  // sampling rate
					);

void IIR1_ShelvingEQ(DWORD dwFilter,
				   double a[2],
				   double b[2],
				   double dCutFreq,
				   double dDb,
				   double dSmpRate  // sampling rate
						   );


void IIR2_PeakingEQ(double a[3],
				   double b[3],

					double dCutFreqL, // low
					double dCutFreqH, // hish
					double dDb,

					double dSmpRate  // sampling rate
					);
DWORD GetIIRimpulse(double* imp,DWORD N,DWORD dwIirNum,DWORD dwChn);
DWORD GetIIRCoef(double* dCoef[2],DWORD dwIirNum,DWORD dwChn);


// iireq.c
DWORD GetIIREQimpulse(double* imp,DWORD N,DWORD dwChn);



/* obsolete
//--------------------------------------------------
// wavesh.c
DWORD SHRINK_MDCT(double* lpFilterBuf, // filter buffer
			DWORD dwPointsInBuf, // points
			DWORD dwCh			// channel
			);

void unprepareShrink();
void ClearShrink();

void prepareShrink(DWORD dwShLeng,
				   WAVFMT waveFmt,
				   double dThreshold  // dB
				   );
*/


//--------------------------------------------------
// rsmp.c
void prepareRsmp(DWORD dwInputFreq, DWORD dwOutputFreq, 
				 DWORD dwCutFreq, double dDB, 
				 DWORD dwLength, 
				 DWORD dwUp,DWORD dwDown,
				 DWORD dwFilterBufSize,DWORD dwChn);

void unprepareRsmp();
void ClearRsmp();
void RSAMP( double* lpFilterBuf, // filter buffer
		   DWORD dwPointsInBuf,
			DWORD dwCh,

			LPDWORD lpdwAfterPoints
			);
DWORD GetRSMPCoef(double** h,DWORD dwInputFreq,LPDWORD lpdwUp);


//--------------------------------------------------
// mixfile.c
void ClearMixFile();
void CloseMixFile();
void MixFile(WAVFMT waveOrgFmt, 
			 double* lpFilterBuf[2],
			 DWORD dwPoints, // points in buffer
			 LONGLONG n64OutSize, // output size of data
			 double dMixLevel[2],  // mixing level
			 double dMixStartTime // mixing start time
			 );

BOOL OpenMixFile(char* szMixFile,
				 WAVFMT waveOrgFmt, 
				 DWORD dwBufSize, // points of data in buffer
				 double dMixStartTime, // start time in mixing file
				 char* lpszErr
				 );

//--------------------------------------------------
// compr.c
void ClearCOMP();
void COMPRESS(long nSamplesPerSec,
			 double* lpFilterBuf[2],
			 DWORD dwPointsInBuf, 
			 DWORD dwChn,

			 double dTh,  // threshold
			 double dRatio,
			 double dAttackTime,  // msec
			 double dReleaseTime,  // msec
			 DWORD dwCompNum
			 );
void prepareCOMP(DWORD dwChn,DWORD dwRMSsize,DWORD dwCompNum);
void unprepareCOMP();



//--------------------------------------------------
// split.c
VOID SPLIT(WAVFMT waveFmt, 
		   DWORD* lpdwPointsInBuf, // points in buffer
		   LONGLONG n64TotalOutSize, // total output size
		   DWORD dwNoSndCurNo,
		   BOOL* bChangeFile, // if bChangeFile = true, then file is changed
		   LONGLONG* lpn64SplitByte, // if output size exceeds this size, then change file

		   double* lpdSplitTime, // for -split2,3
		   LONGLONG* lpn64SplitByteMalti // for -splitbm
		   );


VOID NOSOUND(double* lpFilterBuf[2], // buffer
			  DWORD* lpdwPointsInBuf, // points of data in buffer
			   BOOL* lpbChangeFile, // if *lpbChangeFile = true, change output file
			   /* obsolete
			   HWND hWnd, // HWND of LockOn
			   */
			   long nSamplesPerSec,
			   DWORD dwChn,

			   DWORD dwNoSndTime,
			   DWORD dwNoSndM1toM2, 
			   DWORD dwNoSndRecStop, 

			   double dNoSndBoundHead, // level of sound (dB)
			   double dNoSndBound // level of no sound (dB)
			   );
void PrepareNOSOUND(DWORD dwNoSndMode, DWORD dwNoSndSuspend, long nSamplesPerSec);

VOID ClearNOSOUND();  // you need to call this function after spliting file


//--------------------------------------------------
// dcoffset.c
VOID DCOFFSET(double* lpFilterBuf, // buffer
			  DWORD dwPointsInBuf, // points
			  double dOffset);

VOID AUTODCOFFSET(double* lpFilterBuf[2], // buffer (L-R)
			  DWORD dwPointsInBuf, // points
			  WAVFMT waveFmt,  // format
			  DWORD dwTrainSec // sec, training time
			  );

void ClearAutoDCOffset();
void GetAutoOffset(double dOffset[2]);


//--------------------------------------------------
// fft.c
void calcFftFast(COMPLEX* data,
			 DWORD dwFFTnum
			 );
void calcIfftFast(COMPLEX* data,
			 DWORD dwFFTnum
			 );
void prepareFft(DWORD dwFftPoint,
				DWORD dwFFTnum
				);
void unprepareFft();


/* obsolete
//--------------------------------------------------
// mdct.c
void prepareMDCT(DWORD N,DWORD dwMDCTnum,DWORD dwFFTnum);
void unprepareMDCT();
void MDCT_fast(double* x, // N	
		  double* f,	// N/2	
		  DWORD dwMDCTnum,
		  DWORD dwFFTnum
		  );
void IMDCT_fast(double* x,	// N 
				double* f,	// N/2 
				DWORD dwMDCTnum,
				DWORD dwFFTnum
				);
*/


//--------------------------------------------------
// fadeinout.c
void FADEINOUT(double* lpFilterBuf, // filter buffer
				   DWORD dwPointsInBuf, // points of data in buffer

				   WAVFMT waveFmt,
				   DWORD dwFadeIn, // points of fade in
				   DWORD dwFadeOut, // points of fase out

				   LONGLONG n64TotalOutSize, // current size of output
				   LONGLONG n64TotalSize // total size of input file
				   );


// normal.c
void ClearNormal();
void SET_PEAK(double* lpFilterBuf, // buffer
			   DWORD dwPointsInBuf, // points
			   DWORD dwChn // channel
			   );

void SET_AVG(double* lpFilterBuf, // buffer
			 DWORD dwPointsInBuf, // points
			 DWORD dwChn // channel
			 );

void SET_RMS(double* lpFilterBuf, // buffer
			 DWORD dwPointsInBuf, // points
			 DWORD dwChn // channel
			 );
void GET_RMS(double* dRMS);
void GET_PEAK(double* dPeak);
void GET_AVG(double* dAVG);




//-------------------------------------------------
// filterfunc.c


DWORD  GetVer(char* szVer); // get version

//double kaiserAlpha(double dDB);
//double I0(double x);
void Idft(LPCOMPLEX data, // input
		  LPCOMPLEX out, // output
		  DWORD N  
		  );
void KBDWin(double* win, DWORD dwSize,double alpha);

void AddSinCurve(double* lpFilterBuf,
				 DWORD dwPointsInBuf,

				 DWORD dwRate,
				 DWORD dwChn,
				 DWORD dwNum, //number of waves  (if 0,  then initialize)
				 LPDWORD dwFreq,double* lpdbDb,LPDWORD dwPhase);

/* obsolete
VOID StopRecording(HWND hWnd);
*/

double CalcSpec(LPCOMPLEX x,// input
				DWORD N, // number of data
				double freq,  // frequency
				double samp_freq // sampling frequency
				); 
/*
//------------------------------------------------
// fbank.c
void ClearFltBank();
VOID FltBank_analysis(double *dInput, // SUB_BAND point, input
				double *dOutput,   // SUB_BAND point, output
				DWORD dwCh);
// synthesis
VOID FltBank_synthesis(double *dOutput, // SUB_BAND point, output
				double *dInput,   // SUB_BAND point, input
				DWORD  dwCh // channel
				);
*/

//--------------------------------------------
// ngate.c
void NOISEGATE_FREQ(double* lpFilterBuf[2], // filter buffer
			DWORD dwPointsInBuf, // points
			const unsigned int channels
			);
void unprepareNGATE();
void ClearNGATE();
void prepareNGATE(WAVFMT waveFmt,
				  double dThreshold,  // dB
				  double dRelease, // msec
				  double dAttack, // msec
				  DWORD dwRMS
				   );

//--------------------------------------------
// dither.c
void ClearNSFBuf();
void DITHER(double* lpFilterBuf, // filter buffer
		 DWORD points,
		 int channel,
		 double amp
		 );

/* obsolete
//--------------------------------------------
// rand.c
void init_genrand(unsigned long s);
unsigned long genrand_int32(void);
*/

/*
BOOL ReadOption(int, char**);
BOOL SetParam();
BOOL Filter();
void DeleteInputFile();
*/


//--------------------------------------------------
// function.c

int GetArgv(char*,char[MAX_ARGC][CHR_BUF],int);
void ShowAbout();
BOOL GetCmdLineFromFile(char*,char*);
void ShowStatus(WAVFMT waveFmt,  
				char* szWriteFile, // name of output file
/* obsolete
				BOOL bCreatePipe, // pipe mode
*/
				LONGLONG u64DataSize,  // output size 
				LONGLONG u64TotalSize, // total size of output file
				double dPeak,	   // peak
/* obsolete
				BOOL bADPtrain, // ADP training mode
*/
				BOOL bNormalGain, // now, normalizer is searching peak
				BOOL bStdin // stdin mode
				);


#ifdef USEWIN32API
/* obsolete
BOOL ExecCommand(LPSTR lpszCommandLine,PROCESS_INFORMATION*,BOOL,BOOL);
BOOL ExecPipeCommand(LPSTR,PROCESS_INFORMATION*,HANDLE*);
*/
BOOL GetFileMappingData(int,char*[ ],LPSTR,LPSTR);
VOID GetFreeHddSpace64(ULONGLONG*,LPSTR);
#endif


VOID SetCommandStrings(
#ifndef DEF_WAVEFLT
					   BOOL bCommand,
#endif
					   BOOL bFstdin,  // use 'fstdin'
					   LPSTR lpszCommand, // output strings
					   LPSTR lpszCommandIn, // input strings
					   LPSTR lpszFile,     // file name
					   LPSTR lpszUserDef1, // user defined strings 
					   LPSTR lpszUserDef2,
					   LPSTR lpszUserDef3,
					   SYSTEMTIME sysTime, // current time
					   WAVFMT waveFmt,
					   LONGLONG n64DataSize // data size
#ifndef DEF_WAVEFLT
					   ,HWND hWnd,  // hwnd of lockon
					   DWORD dwRecTime, // recording time(sec) : if n64DataSize = 0, use this parameter
					   BOOL bOpenConsole, // open console
					   WORD wSaveMode // save mode
#endif
					   );

/* obsolete
#ifdef USEWIN32API
BOOL StopChildStop(PROCESS_INFORMATION,BOOL);
#endif
*/

#ifdef USEWIN32API
VOID AddSpace(HANDLE hdWriteFile,DWORD dwSize);
#else 
void AddSpace(FILE* hdWriteFile,DWORD dwSize);
#endif

BOOL ExchangeTime(LPSTR lpTime,double* lpSec);


BOOL ShowPeakWave(char* lpszFile, // name of input file
			 DWORD dwBufSize,
			 WAVFMT waveFmt,
			 LONGLONG n64Offset,  // offset 
			 LONGLONG n64DataSize);

void GetGainForNormalizer(double dNormalGain[2],
							BOOL bStereoLink,
							DWORD dwCurrentNormalMode,
							double dNormalLevel,
							LONGLONG n64TotalOutSize,
							WAVFMT waveFmt);
/* obsolete
void OutputADPFilterChr(char* lpszSaveDir,DWORD dwChn,DWORD dwSampleRate);
*/

void OutputFIRFilterChr(char* lpszSaveDir,WAVFMT waveFmt,DWORD dwFIRnum);
void OutputIIRFilterChr(char* lpszSaveDir,WAVFMT waveFmt,DWORD dwFIRnum);
void printIIRcoef(DWORD dwIirNum);
void OutputRsmpChr(char* lpszSaveDir,WAVFMT waveFmt);

//--------------------------------------------------
// playwave.c
#ifdef USEWIN32API
BOOL OpenWaveDevice(UINT,WAVFMT,DWORD);
BOOL PlayWave(BYTE*,DWORD);
BOOL CloseWaveDevice(BOOL);
#endif



//--------------------------------------------------
// io.c

#ifdef USEWIN32API
BOOL OpenReadFile(HANDLE* hdReadFile,char* szReadFile,
#else
BOOL OpenReadFile(FILE** hdReadFile,char* szReadFile,
#endif
					BOOL bStdin // stdin 
					);

#ifdef USEWIN32API
BOOL OpenWriteFile(HANDLE* hdWriteFile,char* szWriteFile,
				   /* obsolete
				   PROCESS_INFORMATION* pProInfo,
				   */
#else
BOOL OpenWriteFile(FILE** hdWriteFile,char* szWriteFile,
#endif
					BOOL bStdout // stdou4
/* obsolete
					BOOL bCreatePipe, // create pipe
					char* szPipeCmd, // command of pipe
					DWORD dwPipeSize  // buffer size of pipe
*/
					);

#ifdef USEWIN32API
void ReadData(HANDLE,BYTE*,DWORD,DWORD*);
#else
void ReadData(FILE*,BYTE*,DWORD,DWORD*);
#endif

#ifdef USEWIN32API
BOOL WriteData(HANDLE hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte
/* obsolete
			   ,BOOL bCreatePipe,
			   PROCESS_INFORMATION hProcessInfo
			   */
			   );
#else
BOOL WriteData(FILE* hdWriteFile,BYTE* lpBuffer,DWORD dwWriteByte,DWORD* lpdwByte,
/* obsolete
			   BOOL bCreatePipe
*/
			   );
#endif

BOOL WriteTextData(HANDLE hdWriteFile,
				   double* lpFilterBuf[2], // buffer
				   DWORD dwPointsInBuf, // points of data in buffer
				   WAVFMT waveFmt);



// outfile.c
void SetOutputFileName(LPSTR lpszBaseFile,  // base name
						 LPSTR lpszOutputFile, // output file name
						 DWORD dwNum // number
						 );


BOOL PrepareOutputFileName(LPSTR lpszConfigFile, // configuration file
#ifdef WIN32		
						HANDLE hFileMap, // handle of file mapping
#endif
						char* lpszErr
						);

void unprepareOutputFileName();


// rand.c
//void init_genrand(unsigned long s);




#endif
