// DC offset

#include <stdio.h>

#include "dcoffset.h"

DcOffset::DcOffset( const WaveFormat& _input_format )
    : Filter( _input_format )
{
    dbg = true;
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::DcOffset\n");

    clear_buffer();
}

DcOffset::~DcOffset()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::~DcOffset\n");
}

void DcOffset::set_offset( const unsigned int track_no, const std::vector<double>& offset )
{
    if( track_no+1 > offsets.size() ) offsets.resize( track_no+1 );
    offsets[track_no] = offset;
}

void DcOffset::show_config() const
{
    fprintf(stderr,"DC offset: " );
    for( unsigned int no=0; no < offsets.size(); ++no){

        const std::vector<double>& offset = offsets[no];
        if( offset.size() == 0 ) continue;

        fprintf(stderr,"[Tr_%d: ", no );
        for( unsigned int i=0; i < offset.size(); ++i){
            fprintf(stderr,"Ch_%d=%.2lf", i, offset[i] );
            if( i < offset.size()-1 ) fprintf(stderr,", " );
        }
        fprintf(stderr,"]");
    }
    fprintf(stderr,"\n" );
}

void DcOffset::clear_buffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::clear_buffer\n");
}

void DcOffset::inputfile_seeked()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::inputfile_seeked\n");
}

void DcOffset::process( std::vector<Track>& tracks )
{
    check_rate_of_tracks( tracks );

    for( unsigned int track_no=0; track_no < tracks.size(); ++track_no){

        if( offsets.size() <= track_no ) return;

        const std::vector<double>& offset = offsets[track_no];
        if( offset.size() == 0 ) continue;

        Track& track = tracks[track_no];
        if( ! track.get_points() ) continue;

        assert( track.get_format().channels() == offset.size() );
        for( unsigned int i=0; i < track.get_format().channels(); ++i){
            for( unsigned int i2=0; i2 < track.get_points(); ++i2 ) track.raw[i][i2] += offset[i];
        }
    }
}

void DcOffset::outputfile_changed()
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::outputfile_changed\n");
}

void DcOffset::show_result() const
{
    if( dbg ) fprintf( stderr, "\n[debug] DcOffset::show_result\n");
}
