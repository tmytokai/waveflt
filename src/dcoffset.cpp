// DC offset

#include "dcoffset.h"

DcOffset::DcOffset( const WAVFMT& _input_format )
    : Filter( _input_format )
{
    offset.resize( output_format.channels );
    for( int i=0; i < output_format.channels; ++i) offset[i] = 0;
}

DcOffset::DcOffset( const WAVFMT& _input_format, const std::vector<double>& _offset )
    : Filter( _input_format ), offset( _offset )
{}

DcOffset::~DcOffset()
{
	printf("\nDcOffset::~DcOffset\n");
}

void DcOffset::set_offset( const int channel, const double value )
{
    assert( channel < format.channels );

    offset[ channel ] = value;
}

const double DcOffset::get_offset( const int channel )
{
    assert( channel < format.channels );

    return offset[ channel ];
}

void DcOffset::show_config()
{
	fprintf(stderr,"DC offset: " );
	for( int i=0; i < output_format.channels; ++i){
		fprintf(stderr,"ch%d = %lf ", i, offset[i] );
		if( i < output_format.channels-1 ) fprintf(stderr,", " );
	}
	fprintf(stderr,"\n" );
}

void DcOffset::clear_buffer()
{
	printf("\nDcOffset::clear_buffer\n");
}

void DcOffset::process( Buffer& buffer )
{
	check_input_format( buffer.format );

    if( ! buffer.points ) return;
    for( int i=0; i < output_format.channels; ++i){
		for( unsigned int i2=0; i2 < buffer.points; ++i2 ) buffer.buffer[i][i2] += offset[i];
    }

	buffer.format = output_format;
}

void DcOffset::file_changed()
{
	printf("\nDcOffset::file_changed\n");
}

void DcOffset::show_result()
{
	printf("\nDcOffset::show_result\n");
}


//------------------------------------------------------
// obsolete

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