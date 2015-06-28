// subband noise gate

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// in the experimental stage
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#ifdef WIN32
#include <windows.h>
#define _USE_MATH_DEFINES
#endif
#include <math.h>

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

//#define NG_USEFFT // use FFT

#define ID_FFT_NGATE 1

#define MAX_CHN		6   // max number of channels

#ifdef NG_USEFFT
#define NG_BAND	2048
#else
#define NG_BAND	32
#endif

double *NG_dGain[MAX_CHN];  // current gain at each band (0 <= NG_dGain <= 1)
double *NG_dThreshold[MAX_CHN]; // threshold (linear)

// subband signal
#ifdef NG_USEFFT
COMPLEX *NG_dBand[MAX_CHN]; 
#else
double *NG_dBand[MAX_CHN]; 
#endif

//----------------------------------
// noise gate (stereo link)
void NOISEGATE_FREQ(double* lpFilterBuf[2], // filter buffer
			DWORD dwPointsInBuf, // points
			const unsigned int channels
			)
{
	DWORD i,i2,dwCh,dwBand; 

	double dGain,dPrevGain,dLevel;

	// ”¼’[•ª‚Í‚Æ‚è‚ ‚¦‚¸Œã‰ñ‚µ
//	if(dwPointsInBuf % NG_BAND) printf("X\n");

	for(i=0;i< dwPointsInBuf/NG_BAND;i++)
	{  
		for(dwCh = 0; dwCh < channels; dwCh++)
		{
#ifdef NG_USEFFT
			memset(NG_dBand[dwCh],0,sizeof(COMPLEX)*NG_BAND);
			for(i2 = 0 ;i2 < NG_BAND; i2++) 
				NG_dBand[dwCh][i2].r = lpFilterBuf[dwCh][i*NG_BAND + i2];

			calcFftFast(NG_dBand[dwCh],ID_FFT_NGATE);
			for(i2 = 0 ;i2 < NG_BAND; i2++) // normalize
			{
				NG_dBand[dwCh][i2].r *= 1./NG_BAND;
				NG_dBand[dwCh][i2].i *= 1./NG_BAND;
			}
			dwBand = NG_BAND/2;
#else
			// analysis bank
			FltBank_analysis(lpFilterBuf[dwCh]+i*NG_BAND,NG_dBand[dwCh],dwCh);
			dwBand = NG_BAND;
#endif

			// renew gain at each band
			for(i2 = 0 ;i2 <= dwBand; i2++)
			{
				dPrevGain = NG_dGain[dwCh][i2];
#ifdef NG_USEFFT
				dLevel = 20.*(1./2.)*log10(NG_dBand[dwCh][i2].r*NG_dBand[dwCh][i2].r 
					+ NG_dBand[dwCh][i2].i*NG_dBand[dwCh][i2].i);
				if(dLevel <= NG_dThreshold[dwCh][i2]) 
					NG_dGain[dwCh][i2] = 0.5 * dPrevGain;
				else 
					NG_dGain[dwCh][i2] = 0.5 * dPrevGain + 0.5;

#else
				dLevel = NG_dBand[dwCh][i2];
				if(dLevel <= NG_dThreshold[dwCh][i2]) 
					NG_dGain[dwCh][i2] = 0.999 * dPrevGain;
				else 
					NG_dGain[dwCh][i2] = 0.99 * dPrevGain + 0.01;
#endif
			}
		}

		for(i2 = 0 ;i2 < dwBand; i2++)
		{
			// stereo link
			dGain = 1;
			for(dwCh = 0; dwCh < channels; dwCh++)	dGain = min(dGain,NG_dGain[dwCh][i2]);

			for(dwCh = 0; dwCh < channels; dwCh++)
			{
#ifdef NG_USEFFT
				NG_dBand[dwCh][i2].r *= dGain;
				NG_dBand[dwCh][i2].i *= dGain;
				NG_dBand[dwCh][NG_BAND-i2].r *= dGain;
				NG_dBand[dwCh][NG_BAND-i2].i *= dGain;
#else
				NG_dBand[dwCh][i2] *= dGain;
#endif
			}
		}

		// synthesis bank
		for(dwCh = 0; dwCh < channels; dwCh++)	
		{
#ifdef NG_USEFFT
			calcIfftFast(NG_dBand[dwCh],ID_FFT_NGATE);
			for(i2 = 0 ;i2 < NG_BAND; i2++) 
				lpFilterBuf[dwCh][i*NG_BAND + i2] = NG_dBand[dwCh][i2].r;

#else
			FltBank_synthesis(lpFilterBuf[dwCh]+i*NG_BAND,NG_dBand[dwCh],dwCh);
#endif
		}
	}
}




//--------------------------------------------------------------------
// unprepare
void unprepareNGATE()
{
	DWORD i;
	
	for(i=0;i<MAX_CHN;i++)
	{
		if(NG_dGain[i]) free(NG_dGain[i]);
		if(NG_dThreshold[i]) free(NG_dThreshold[i]);
		if(NG_dBand[i]) free(NG_dBand[i]);
		NG_dGain[i] = NULL;
		NG_dThreshold[i] = NULL;
		NG_dBand[i] = NULL;
	}
}




//----------------------------------------------------------------
// clear
void ClearNGATE()
{
	DWORD i,i2;

	for(i=0;i<MAX_CHN;i++)
	{
		for(i2=0;i2<NG_BAND;i2++)
		{
			if(NG_dGain[i]) NG_dGain[i][i2] = 0;
		}
	}
}




//--------------------------------------------------------------------
// prepare
void prepareNGATE(WAVEFORMATEX waveFmt,
				  double dThreshold,  // dB
				  double dRelease, // msec
				  double dAttack, // msec
				  DWORD dwRMS
				   )
{
	DWORD i,i2;
	
	for(i=0;i<waveFmt.nChannels;i++)
	{
		if(NG_dGain[i] == NULL) NG_dGain[i] = (double*)malloc(sizeof(double)*NG_BAND+1024);
		if(NG_dThreshold[i] == NULL) NG_dThreshold[i] = (double*)malloc(sizeof(double)*NG_BAND+1024);

#ifdef NG_USEFFT
		if(NG_dBand[i] == NULL) NG_dBand[i] = (COMPLEX*)malloc(sizeof(COMPLEX)*NG_BAND+1024);
#else
		if(NG_dBand[i] == NULL) NG_dBand[i] = (double*)malloc(sizeof(double)*NG_BAND+1024);
#endif

		for(i2=0;i2<NG_BAND;i2++)
		{
#ifdef NG_USEFFT
			NG_dThreshold[i][i2] = dThreshold;
#else
			NG_dThreshold[i][i2] = pow(10.,dThreshold/20.);
#endif
		}
	}

#ifdef NG_USEFFT
	prepareFft(NG_BAND,ID_FFT_NGATE); 
#else
	ClearFltBank();
#endif
}

// EOF
