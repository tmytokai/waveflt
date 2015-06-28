// dither + noise shaper (2-order Delta-Sigma ADC)

#include "filter.h"

double buf_stage1[MAX_CHN];
double buf_stage2[MAX_CHN];
double buf_out[MAX_CHN];

//-----------------------------
// clear buffer data
void ClearNSFBuf()
{
	for(int i=0;i<MAX_CHN;i++)
	{
		buf_stage1[i] = buf_stage2[i]=buf_out[i] = 0;
	}
}


//--------------------------------------------------------
// dither + noise shaper
void DITHER(double* lpFilterBuf, // filter buffer
		 DWORD points,
		 int channel,
		 double amp
		 )
{
	for(unsigned int i=0;i<points;i++)
	{ 
		// dither using TPDF(Triangular Probability Density Function)
		double dither = amp * ( ( (double)rand() + (double)rand() )/RAND_MAX -1.0 ) ;
		lpFilterBuf[i] += dither;

		// noise shaper (2-order Delta-Sigma ADC)
		// Output(z) = [ { Input(z) - z^{-1} Output(z) }{1/(1-z^{-1})} -  z^{-1} Output(z) ]{1/(1-z^{-1})} + Noise(z)
		// Output(z) = Input(z) + (1-z^{-1})^2 Noise(z)
		double aout = lpFilterBuf[i];
		aout -= buf_out[channel];
		aout += buf_stage1[channel]; buf_stage1[channel] = aout;
		aout -= buf_out[channel];
		aout += buf_stage2[channel]; buf_stage2[channel] = aout;
		double dout = (double)(QUANTIZATION(aout,0.5));
		buf_out[channel] = dout;
		lpFilterBuf[i] = aout;
	}
}
