#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "waveformat.h"
#include "filter.h"
#include "trackmanager.h"

int main( int argc, char* argv[])
{
    assert( argc == 3 );
    std::string in_file( argv[1] );
    std::string out_file( argv[2] );
    printf("input = %s, output = %s\n", in_file.c_str(), out_file.c_str() );

    double start = 60;
    double sec = 10;

    TrackManager trackmanager;
    std::vector<Filter*> filters;

    WaveFormat raw_format;
    WaveFormat data_format;
    WaveFormat out_format;
    try{
        raw_format.read( in_file );
        printf("raw_format:\n");
        printf("datasize = %d\n", raw_format.get_datasize() );
        printf("channels = %d\n", raw_format.channels() );
        printf("bits = %d\n", raw_format.bits() );
        printf("rate = %d\n", raw_format.rate() );

        data_format.set( raw_format.tag(), raw_format.channels(), raw_format.rate(), raw_format.bits() );
        printf("data_format:\n");
        printf("channels = %d\n", data_format.channels() );
        printf("bits = %d\n", data_format.bits() );
        printf("rate = %d\n", data_format.rate() );

        out_format.set( data_format.tag(), data_format.channels(), data_format.rate(), 16 );
    } catch( std::string err ){
        printf("%s\n", err.c_str() );
        exit(1);
    }

    std::vector<BlockData> blockdata;
    BlockData block;
    block.input_type = TYPE_STORAGE;
    block.raw_offset = (unsigned long long)(start*raw_format.rate());
    block.raw_points = (unsigned long long)(sec*raw_format.rate());
    block.data_points = block.raw_points;
    blockdata.push_back( block );
    printf("block:\n");
    printf("raw_offset = %d\n", (unsigned int)block.raw_offset );
    printf("raw_points = %d\n", (unsigned int)block.raw_points );
    printf("data_points = %d\n", (unsigned int)block.data_points );

    FILE *fp_in = fopen( in_file.c_str(), "rb" );

    Track track( 0, raw_format, data_format );
    track.fp = fp_in;
    track.data_max_points = (unsigned int)( 0.25 * data_format.rate() );
    printf("%d -> ", track.data_max_points );
    track.data_max_points = (track.data_max_points/4)*4;
    printf("%d\n", track.data_max_points );
    track.raw_max_points = track.data_max_points;
    track.raw = (unsigned char*) malloc( raw_format.block() * track.raw_max_points );
    for( unsigned int i = 0; i < data_format.channels(); ++i ) track.data[i] = (double*) malloc( sizeof(double) * track.data_max_points );
    track.set_verbose(true);
    track.set_filters( filters );
    track.set_blockdata( blockdata );
    track.set_filename( in_file );
    trackmanager.tracks.push_back( track );

    FILE *fp_out = fopen( out_file.c_str(), "wb" );
    fseek( fp_out, 44, SEEK_SET );
    short* outdata = (short*) malloc( sizeof(short) * data_format.channels() * track.data_max_points );

    unsigned long long outdatasize = 0;

    trackmanager.start();

    while(1){

        //-----------------------------
        // INPUT
        //-----------------------------
        trackmanager.read();
        if( trackmanager.end_of_tracks() ) break;

        //-----------------------------
        // filtering
        //-----------------------------

        //-----------------------------
        // mixing
        //-----------------------------
        Track& track = trackmanager.get_track(0);
        for( unsigned int i = 0; i < data_format.channels(); ++i ){
            for( unsigned int i2 = 0; i2 < track.get_data_points(); ++i2 ) outdata[i+i2*data_format.channels()] = (short)(track.data[i][i2]/data_format.get_max_level()*out_format.get_max_level());
        }

        //-----------------------------
        // resampling
        //-----------------------------

        //-----------------------------
        // dithering
        //-----------------------------

        //-----------------------------
        // peak searching
        //-----------------------------

        //-----------------------------
        // splitting
        //-----------------------------

        //-----------------------------
        // output
        //-----------------------------
        outdatasize += fwrite( outdata, 1, sizeof(short)*track.get_data_points()*data_format.channels(), fp_out);
    }

    out_format.write( fp_out, outdatasize );

    free( outdata );
    fclose(fp_in);
    fclose(fp_out);

    return 0;
}
