// adjustment of DC offset

#ifdef WIN32
#include <windows.h>
#endif

BOOL BlDCoffFirstBuffer = true; // whether this is first buffer or not
double DbDCoffTotal = 0;
double DbAutoOffset[2] = {0,0}; // current adjustment value of auto DC offset.



//---------------------------------
// get auto offset value
void GetAutoOffset(double dOffset[2]){
	dOffset[0] = DbAutoOffset[0]/DbDCoffTotal;
	dOffset[1] = DbAutoOffset[1]/DbDCoffTotal;
}



//------------------------------------------
// DC adjustment 
VOID DCOFFSET(double* lpFilterBuf, // buffer
			  DWORD dwPointsInBuf, // points
			  double dOffset // offset
			  ){
	DWORD i;

	for(i=0;i<dwPointsInBuf;i++) lpFilterBuf[i] += dOffset;
}


	
//------------------------------------------
// AUTO DC adjustment 
VOID AUTODCOFFSET(double* lpFilterBuf[2], // buffer (L-R)
			  DWORD dwPointsInBuf, // points
			  WAVEFORMATEX waveFmt,  // format
			  DWORD dwTrainSec // sec, training time
			  ){

	DWORD i,i2;
	double dFoo;

	if(dwPointsInBuf 
		&& DbDCoffTotal < (double)waveFmt.nSamplesPerSec * 60 * dwTrainSec){ 
		
		// initialize
		if(BlDCoffFirstBuffer){
			
			for(i2=0;i2<waveFmt.nChannels;i2++){
				DbAutoOffset[i2] = 0;
				for(i=0;i<dwPointsInBuf;i++) DbAutoOffset[i2] += lpFilterBuf[i2][i];
				DbAutoOffset[i2] /= dwPointsInBuf;
			}

			BlDCoffFirstBuffer = false;
		}
		else // training
		{ 
			for(i2=0;i2<waveFmt.nChannels;i2++){
				for(i=0;i<dwPointsInBuf;i++) DbAutoOffset[i2] += lpFilterBuf[i2][i];
			}
		}

		DbDCoffTotal += (double)dwPointsInBuf;

	}
	
	// adjust offset
	if(DbDCoffTotal > 0){
		for(i2=0;i2<waveFmt.nChannels;i2++){
			dFoo = DbAutoOffset[i2]/DbDCoffTotal;
			for(i=0;i<dwPointsInBuf;i++) lpFilterBuf[i2][i] -= dFoo;
		}
	}
}


//---------------------------------
// clear
void ClearAutoDCOffset(){
	BlDCoffFirstBuffer = true; 
	DbDCoffTotal = 0;
	DbAutoOffset[0] = DbAutoOffset[1] = 0;
}




// EOF