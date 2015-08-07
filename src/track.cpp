#if 0

// track class

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "track.h"
#include "doublebuffer.h"

#include "source.h"
#include "dcoffset.h"
#include "resampler.h"

Track::Track(  const int _track_no, const std::string& _filename, DoubleBuffer& _data )
    : Filter( _data ), track_no(_track_no), filename(_filename)
{
    reset();
}


Track::~Track()
{
    if( next ) delete next;
    next = NULL;
    if( dbg ) fprintf( stderr, "\n[debug] Track(%d)::~Track\n", track_no );
    reset();
}


// private
void Track::reset()
{
if( dbg ) fprintf( stderr, "\n[debug] Track(%d)::reset\n", track_no );
}


void Track::init()
{
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::init\n", track_no );
    assert( !track_no );

    try{

        Filter* parent = this;

        std::vector<BlockData> blockdata;
        BlockData block;
        block.mode = INPUTMODE_NORMAL;
        block.start = 0;
        block.length = in_format.get_datasize() / in_format.block();
        blockdata.push_back( block );
        Filter* filter = new Source( in_format, data, filename, blockdata );
        filter->debugmode();
        parent->connect( filter );
        parent = filter;

        std::vector<double> offsets;
        offsets.push_back(0);
        filter = new DcOffset( parent->get_out_format(), data, offsets );
        filter->debugmode();
        parent->connect( filter );
        parent = filter;

        filter = new Resampler( parent->get_out_format(), data, 48000 );
        filter->debugmode();
        parent->connect( filter );
        parent = filter;

    } catch( const std::string& err ){
        printf("%s\n", err.c_str() );
        exit(1);
    }


    if( next ) next->init();
}


void Track::show_config() const
{
    fprintf(stderr,"\n----------------\n");
    fprintf(stderr,"[configuration of Track_%d]\n\n", track_no );
    if( next ) next->show_config();
    fprintf(stderr,"\n");
}


void Track::start_track()
{
    if( next ) next->start_track();
    if( dbg ) fprintf( stderr, "\n[debug] Track(%d)::start_track\n", track_no );
}


const unsigned int Track::process( const unsigned int )
{
    if( next ) return next->process( 0 );
    return 0;
}


const bool Track::is_over() const
{
    if( next ) return next->is_over();
    return true;
}


void Track::show_result() const
{
    fprintf(stderr,"\n----------------\n");
    fprintf(stderr,"[result of Track_%d]\n\n", track_no );
    if( next ) next->show_result();
}

#endif