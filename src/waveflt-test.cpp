#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "waveformat.h"
#include "trackmanager.h"

int main( int argc, char* argv[])
{
    assert( argc == 3 );
    std::string in_file( argv[1] );
    std::string out_file( argv[2] );
    printf("input = %s, output = %s\n", in_file.c_str(), out_file.c_str() );

    double start = 0;
    double sec = 10;

    TrackManager trackmanager;

    WaveFormat raw_format;
    WaveFormat data_format;
    WaveFormat out_format;
    try{
        raw_format.read( in_file );
        printf("\nraw_format:\n");
        printf("datasize = %d\n", raw_format.get_datasize() );
        printf("channels = %d\n", raw_format.channels() );
        printf("bits = %d\n", raw_format.bits() );
        printf("rate = %d\n", raw_format.rate() );

        data_format.set( raw_format.tag(), raw_format.channels(), raw_format.rate(), raw_format.bits() );
        printf("\ndata_format:\n");
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
    printf("\nblock:\n");
    printf("raw_offset = %d\n", (unsigned int)block.raw_offset );
    printf("raw_points = %d\n", (unsigned int)block.raw_points );
    printf("data_points = %d\n", (unsigned int)block.data_points );

    Track* track = new Track( 0, in_file, raw_format, data_format );
    track->set_verbose(true);
    track->set_blockdata( blockdata );
    trackmanager.tracks.push_back( track );
    trackmanager.init();
    trackmanager.show_config();

    FILE *fp_out = fopen( out_file.c_str(), "wb" );
    fseek( fp_out, 44, SEEK_SET );
    short* outdata = (short*) malloc( sizeof(short) * data_format.channels() * track->get_data_max_points() );
    unsigned long long outdatasize = 0;

    trackmanager.start();

    while(1){

        //-----------------------------
        // input & filtering
        //-----------------------------
        trackmanager.process();
        if( trackmanager.end_of_tracks() ) break;

        //-----------------------------
        // mixing
        //-----------------------------
        Track* track = trackmanager.get_track(0);
        for( unsigned int i = 0; i < data_format.channels(); ++i ){
            for( unsigned int i2 = 0; i2 < track->get_data_points(); ++i2 )
                outdata[i+i2*data_format.channels()] = (short)(track->get_data(i)[i2]/data_format.get_max_level()*out_format.get_max_level());
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
        outdatasize += fwrite( outdata, 1, sizeof(short)*track->get_data_points()*data_format.channels(), fp_out);
    }

    out_format.write( fp_out, outdatasize );

    trackmanager.show_result();
    trackmanager.free();

    free( outdata );
    fclose(fp_out);

    return 0;
}
