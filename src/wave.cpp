// common wave functions

#include "wave.h"
#include "waveformat.h"


//-------------------------------------------------------------------
// change byte-data to double-data 
void WaveLevel(double dLevel[2],  // output of left and right
			   const unsigned char* lpWaveData,  // input
			   const WaveFormat& format)
{ 	
	long i;
	short data[2];
	long nData[2];
	double dData[2];
	float fData[2];

	nData[1] = 0;
	
	if(format.bits()==8){ // 8 bit
		for(i=0;i<format.channels();i++) nData[i] = (long)lpWaveData[i]-0x80;
	}
	else if(format.bits()==16){	// 16 bit
		memcpy(data,lpWaveData,sizeof(short)*format.channels());
		nData[0] = (long)data[0];
		nData[1] = (long)data[1];
	}
	else if(format.bits()==24){	// 24 bit
		for(i=0;i<format.channels();i++){
			nData[i] = 0;
			memcpy((unsigned char*)(nData+i)+1,lpWaveData+3*i,3);
			nData[i] /= 256;
		}
	}
	else if(format.bits()==32 && format.tag() == WAVE_FORMAT_PCM){	// 32 bit long
		memcpy(nData,lpWaveData,sizeof(long)*format.channels());
	}
	else if(format.bits()==32 && format.tag() == WAVE_FORMAT_IEEE_FLOAT){	// 32 bit float
		memcpy(fData,lpWaveData,sizeof(double)*format.channels());
		dLevel[0] = fData[0];
		dLevel[1] = fData[1];
		return;
	}	
	else if(format.bits()==64){	// 64 bit double
		memcpy(dData,lpWaveData,sizeof(double)*format.channels());
		dLevel[0] = dData[0];
		dLevel[1] = dData[1];
		return;
	}

	dLevel[0] = (double)nData[0];
	dLevel[1] = (double)nData[1];
}

