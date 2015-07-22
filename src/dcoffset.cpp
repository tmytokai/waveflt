// DC offset

#include <stdio.h>

#include "dcoffset.h"

DcOffset::DcOffset( const WaveFormat& _data_format, std::vector<double*>& _data, const std::vector<double>& _offset )
    : Filter( _data_format, _data )
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::DcOffset\n");

    offset = _offset;
    clear();
}

DcOffset::~DcOffset()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::~DcOffset\n");
}

void DcOffset::show_config() const
{
    if( offset.size() == 0 ) return;

    fprintf(stderr,"DC offset: " );
    for( unsigned int i=0; i < offset.size(); ++i){
        fprintf(stderr,"Ch_%d = %.2lf", i, offset[i] );
        if( i < offset.size()-1 ) fprintf(stderr,", " );
    }
    fprintf(stderr,"\n" );
}

void DcOffset::clear()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::clear\n");
}

void DcOffset::process( const unsigned int data_points )
{
    if( offset.size() == 0 ) return;
    if( data_points == 0 ) return;

    for( unsigned int i=0; i < data_format.channels(); ++i){
        for( unsigned int i2=0; i2 < data_points; ++i2 ) data[i][i2] += offset[i];
    }
}

void DcOffset::show_result() const
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::show_result\n");
}
