// serch peak, avg, RMS for normalizer

#include "filter.h"


double DbPeak[2] = {0,0}; // peak
double DbAvg[2] = {0,0};  // avg
double DbRms[2] = {0,0};  // RMS


//-------------------------
// clear
void ClearNormal(){

	DbPeak[0] = DbPeak[1] = 0;
	DbAvg[0] = DbAvg[1] = 0;
	DbRms[0] = DbRms[1] = 0;
}


//---------------------------------
// peak 
void SET_PEAK(double* lpFilterBuf, // buffer
			   DWORD dwPointsInBuf, // points
			   DWORD dwChn // channel
			   ){
	
	DWORD i;
	for(i=0;i<dwPointsInBuf;i++) DbPeak[dwChn] = max(DbPeak[dwChn],fabs(lpFilterBuf[i]));

}


void GET_PEAK(double* dPeak){ 
	dPeak[0] = DbPeak[0];
	dPeak[1] = DbPeak[1];
}



//---------------------------------
// average
void SET_AVG(double* lpFilterBuf, // buffer
			 DWORD dwPointsInBuf, // points
			 DWORD dwChn // channel
			 ){
	DWORD i;
	
	for(i=0;i<dwPointsInBuf;i++) DbAvg[dwChn] += fabs(lpFilterBuf[i]);
				
}


void GET_AVG(double* dAVG){
	dAVG[0] = DbAvg[0];
	dAVG[1] = DbAvg[1];
}



//---------------------------------
// RMS
void SET_RMS(double* lpFilterBuf, // buffer
			 DWORD dwPointsInBuf, // points
			 DWORD dwChn // channel
			 ){
	DWORD i;
	
	for(i=0;i<dwPointsInBuf;i++) DbRms[dwChn] += lpFilterBuf[i]*lpFilterBuf[i];
				
}

void GET_RMS(double* dRMS){
	dRMS[0] = DbRms[0];
	dRMS[1] = DbRms[1];
}



// EOF