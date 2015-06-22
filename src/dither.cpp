// dither, noise shaping filter

#include "filter.h"

#define  NSF_LNG  24

// FIR noise shaping filter
// These coefficients are listed in 
// Negrescu,C., Stanomir,D. and Niculescu,A.,
// "On Optimal Designing of FIR Noise Shaping Filters",
// IEEE International Conference On Telecommunications,
// Bucharest, Romania, proceedings Vol.2, pp.334-339, 2001.
double NSF_dFiterCoef[NSF_LNG] = 
{
	-2.4007560,
		3.3612296,
		-3.8533006,
		3.9421514,
		-2.8353258,
		1.3603217,
		-0.1321115,
		-0.6898958,
		1.1006814,
		
		-0.8052982,
		0.3388205,
		0.0971337,
		-0.3287556,
		0.4012170,
		-0.2229419,
		0.0529483,
		0.1068104,
		-0.1383153,
		0.1361076,
		
		-0.0392093,
		-0.0053122,
		0.0650480,
		-0.0421644,
		0.0450175
};

double NSF_dBuffer[MAX_CHN][NSF_LNG];
DWORD NSF_dwBufferPos[MAX_CHN];

//-----------------------------
// clear buffer data
void ClearNSFBuf()
{
	DWORD i;
	
	for(i=0;i<MAX_CHN;i++)
	{
		memset(NSF_dBuffer[i],0,sizeof(double)*NSF_LNG);
		NSF_dwBufferPos[i] = 0;
	}
}


//--------------------------------------------------------
// dither + noise shaper
void DITHER(double* lpFilterBuf, // filter buffer
		 DWORD dwPointsInBuf, // points of data in the filter buffer
		 DWORD dwChn, // channel
		 double dAmp
		 )
{
	DWORD i,i2,i3;
	double dE,dDither,dOut;

	for(i=0;i<dwPointsInBuf;i++)
	{ 
		// noise shaper
		dE = 0;
		i3 = NSF_dwBufferPos[dwChn];
		for(i2=0;i2<NSF_LNG;i2++)
		{
			i3 = (i3+1)%NSF_LNG;
			dE -= NSF_dBuffer[dwChn][i3] * NSF_dFiterCoef[i2];
		}

		// TPDF
//		dDither = dAmp * (((double)genrand_int32()-(double)genrand_int32())/0xffffffff);
		dDither = dAmp * (1.0 - 2.0 * (double)rand() / RAND_MAX);
		lpFilterBuf[i] += (dDither+dE);

		dOut = (double)(QUANTIZATION(lpFilterBuf[i],0.5));
		NSF_dBuffer[dwChn][NSF_dwBufferPos[dwChn]] = dOut-lpFilterBuf[i];
		if(NSF_dwBufferPos[dwChn] == 0) NSF_dwBufferPos[dwChn] = NSF_LNG-1;
		else NSF_dwBufferPos[dwChn]--;
	}
}

// EOF