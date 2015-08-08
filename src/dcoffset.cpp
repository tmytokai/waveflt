#if 0

// DC offset

#include <stdio.h>
#include <assert.h>

#include "dcoffset.h"
#include "doublebuffer.h"

DcOffset::DcOffset( DoubleBuffer& _data, const std::vector<double>& _offsets )
    : Filter( _data ), offsets(_offsets)
{
    reset();
}

DcOffset::~DcOffset()
{
    if( next ) delete next;
    next = NULL;
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::~DcOffset\n");
    reset();
}


// private
void DcOffset::reset()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::reset\n");
}


void DcOffset::connect( Filter* _next )
{
    next = _next;
    next->set_in_format( out_format );
}


void DcOffset::init()
{
    if( next ) next->init();
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::init\n");
}


const unsigned int DcOffset::demanded_max_points()
{
    if( next ) return next->demanded_max_points();
    return data.max_points;
}


const unsigned int DcOffset::demanded_points()
{
    if( next ) return next->demanded_points();
    return data.left();
}


void DcOffset::show_config() const
{
    fprintf(stderr,"DC offset: " );
    for( unsigned int i=0; i < out_format.channels(); ++i){
        fprintf(stderr,"Ch_%d = %.2lf", i, offsets[i] );
        if( i+1 < out_format.channels() ) fprintf(stderr,", " );
    }
    fprintf(stderr,"\n" );

    if( next ){
        fprintf(stderr,"=> " );
        next->show_config();
    }
}


void DcOffset::start_track()
{
    if( next ) next->start_track();
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::start_track\n");
}


void DcOffset::begin_block()
{
    if( next ) next->begin_block();
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::begin_block\n");
}

const unsigned int DcOffset::process( const unsigned int points )
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::process : offset = %d, points = %d\n", data.points, points );
    assert( data.size() >= out_format.channels() );
    assert( data.points + points <= data.max_points );
    assert( points <= demanded_points() );
    assert( offsets.size() == out_format.channels() );

    if( points ){

        for( unsigned int i=0; i < out_format.channels(); ++i){
            for( unsigned int i2=0; i2 < points; ++i2 ) data[i][i2+data.points] += offsets[i];
        }
    }

    if( next ) return next->process( points );
    return points;
}


const bool DcOffset::is_over() const
{
    if( next ) return next->is_over();
    return true;
}


void DcOffset::show_result() const
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::show_result\n");
    if( next ) next->show_result();
}


#endif
