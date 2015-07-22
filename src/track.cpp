// track class

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "track.h"
#include "filter.h"
#include "io.h"

#include "dcoffset.h"

Track::Track(  const int _track_no, const std::string& _filename, const WaveFormat& _raw_format, const WaveFormat& _data_format )
    : track_no(_track_no), filename(_filename), raw_format(_raw_format), data_format(_data_format)
{
    reset();
}


Track::~Track()
{
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::~Track\n", track_no );

    if( raw ) free( raw );
    while( data.size() ){
        free( data.back() );
        data.pop_back();
    }

    while( filters.size() ){
        delete filters.back();
        filters.pop_back();
    }

    if( fp ) fclose(fp);
}


void Track::reset()
{
    dbg = true;
    verbose = true;
    raw = NULL;
    block_no = 0;
    fp = NULL;
}


void Track::init()
{
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::init\n", track_no );

    data_max_points = (unsigned int)( 0.25 * data_format.rate() );

    // 32 byte alignment for SSE2
    data_max_points = (data_max_points/4)*4;

    raw_max_points = data_max_points;

    raw = (unsigned char*) malloc( raw_format.block() * raw_max_points );
    data.resize( data_format.channels() );
    for( unsigned int i = 0; i < data_format.channels(); ++i ) data[i] = (double*) malloc( sizeof(double) * data_max_points );

    // test
    std::vector<double> offset;
    offset.push_back(12);
    offset.push_back(34);
    filters.push_back( new DcOffset( data_format, data, offset ) );
    filters.back()->debugmode();

    fp = fopen( filename.c_str(), "rb" );
}


void Track::show_config()
{
    fprintf(stderr,"\n----------------\n");
    fprintf(stderr,"[configuration of Track_%d]\n", track_no );
    fprintf(stderr,"INPUT: file = ");
    switch( blockdata[block_no].input_type ){
        case TYPE_STORAGE:
            fprintf(stderr,"%s",filename.c_str());
            break;
        case TYPE_STDIN:
            fprintf(stderr,"stdin");
            break;
        case TYPE_NULL:
            fprintf(stderr,"null");
            break;
    }
    fprintf(stderr,", %d (Hz), %d (Ch), %d (Bits)\n", raw_format.rate(), raw_format.channels(), raw_format.bits() );
    std::vector<Filter*>::iterator it = filters.begin();
    for( ; it != filters.end(); ++it ){
        fprintf(stderr,"=> ");
        (*it)->show_config();
    }
    fprintf(stderr,"=> OUTPUT");
}


void Track::start()
{
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::start\n", track_no );

    block_no = 0;
    begin_new_block();
}


void Track::process()
{
    if( end_of_track() ) return;

    if( blockdata[block_no].data_points == 0 ) data_points = data_max_points; // endless mode
    else if( blockdata[block_no].data_points > block_total_data_points + data_max_points ) data_points = data_max_points;
    else data_points = (unsigned int)( blockdata[block_no].data_points - block_total_data_points );

    raw_points = data_points;

    if( blockdata[block_no].input_type == TYPE_STORAGE || blockdata[block_no].input_type == TYPE_STDIN ){

        raw_points = ReadData( fp, raw, raw_points * raw_format.block() ) / raw_format.block();

        // think later
//        n64PointerStdin += read_size;  
    }
    else if( blockdata[block_no].input_type == TYPE_NULL ){

        if( raw_format.bits() == 8 ) memset( raw, 0x80, raw_points * raw_format.block() );
        else memset( raw, 0, raw_points * raw_format.block() );
    }

    if( raw_points == 0 ){
        block_no++;
        begin_new_block();
    }
    else{
        convert_raw_to_data();

        std::vector<Filter*>::iterator it = filters.begin();
        for( ; it != filters.end(); ++it ) (*it)->process( data_points );
    }
}


void Track::show_result()
{
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::show_result\n", track_no );

    std::vector<Filter*>::iterator it = filters.begin();
    for( ; it != filters.end(); ++it ) (*it)->show_result();
}



void Track::begin_new_block()
{
    if( end_of_track() ) return;
    if( dbg ) fprintf(stderr, "\n[debug] Track(%d)::begin_new_block\n", track_no );

    raw_points = 0;
    data_points = 0;
    block_total_data_points = 0;

    std::vector<Filter*>::iterator it = filters.begin();
    for( ; it != filters.end(); ++it ) (*it)->clear();

    // move to head of current block
    if( blockdata[block_no].input_type == TYPE_STORAGE ){

        const unsigned long long seek_offset = raw_format.get_offset() + blockdata[block_no].raw_offset*raw_format.block();

        if( dbg ) fprintf(stderr, "\n[debug] seek: offset = %d byte\n", (int)seek_offset );
#ifndef WIN32
        fseeko(fp, (off_t)seek_offset, SEEK_SET);
#else
        _fseeki64( fp , (__int64)seek_offset, SEEK_SET);
#endif
    }
/* think later
   else if( blockdata[block_no].input_type == TYPE_STDIN ){

   n64PointerStdin += SeekStdin(lpBuffer,DwBufSize,
   blockdata[block_no].offset,
   n64PointerStdin
   );
   }
*/

    if( verbose ){

        fprintf(stderr,"\n\n----------------\n");

        fprintf(stderr,"[Track_%d: Block_%d : ", track_no, block_no);
        if( blockdata[block_no].input_type == TYPE_STORAGE || blockdata[block_no].input_type == TYPE_STDIN ){

            const unsigned long long raw_offset = blockdata[block_no].raw_offset*raw_format.block();
            fprintf(stderr,"start = %.2lf sec, ", (double)raw_offset/raw_format.avgbyte());
        }
        if(blockdata[block_no].raw_points > 0){

            const unsigned long long raw_size = blockdata[block_no].raw_points*raw_format.block();
            fprintf(stderr,"length = %.2lf sec ", (double)raw_size/raw_format.avgbyte());
        }
        else fprintf(stderr,"length = endless");
        fprintf(stderr,"]\n");
    }
}


void Track::convert_raw_to_data()
{
    if( end_of_track() ) return;
    if( raw_points == 0 ) return;

    for( unsigned int i = 0 ; i < data_format.channels()  ; i++ ) memset( data[i], 0, sizeof(double)*data_max_points );

    if( raw_format.bits() == 8 ){
		
        for( unsigned int i = 0 ; i < raw_format.channels()  ; i++){

            unsigned int pos = 0;
            for( unsigned int i2 = 0; i2 < raw_points; ++i2 ){

                data[i][pos] = (double)((int)raw[ i2*raw_format.block() +i ]-0x80);
                pos++;
            }
        }
    }

    else if( raw_format.bits() == 16 ){  
		
        for( unsigned int i = 0 ; i < raw_format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer = raw + sizeof(short)*i;
            for( unsigned int i2=0; i2 < raw_points; ++i2 ){

                data[i][pos] = (double)(*((short*)buffer));
                pos++;
                buffer += raw_format.block();
            }
        }
    }

    else if( raw_format.bits() == 24 ){
		
        for( unsigned int i = 0 ; i < raw_format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer = raw + 3*i;
            for( unsigned int i2=0; i2 < raw_points; ++i2 ){

                int val = 0;
                memcpy( (unsigned char*)(&val)+1, buffer, 3);
                val /= 256;

                data[i][pos] = (double)val;
                pos ++;
                buffer += raw_format.block();
            }
        }
    }

    /* think later
    else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_PCM)
    {  // 32 bit long
		
        for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
        {
            pos = 0;
            buffer2 = buffer+sizeof(long)*dwChn;
            memset(data[dwChn],0,sizeof(double)*size);
			
            for( unsigned int  i=0;i< read_size;i+=format.block())
            {
                data[dwChn][pos] = (double)(*((long*)buffer2));
                pos ++;
                buffer2 += format.block();
            }
        }
    }
    else if(format.bits() == 32 && format.tag() == WAVE_FORMAT_IEEE_FLOAT)
    {  // 32 bit float
        for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
        {
            pos = 0;
            buffer2 = buffer+sizeof(float)*dwChn;
            memset(data[dwChn],0,sizeof(double)*size);
			
            for(i=0;i< read_size;i+=format.block())
            {
                data[dwChn][pos] = (double)(*((float*)buffer2));
                pos ++;
                buffer2 += format.block();
            }
        }
    }
    else if(format.bits() == 64)
    {  // 64 bit(double)
		
        for(dwChn = 0 ; dwChn < format.channels()  ; dwChn++)
        {
            pos = 0;
            buffer2 = buffer+sizeof(double)*dwChn;
            memset(data[dwChn],0,sizeof(double)*size);
			
            for(unsigned int i=0;i< read_size;i+=format.block())
            {
                data[dwChn][pos] = (double)(*((double*)buffer2));
                pos ++;
                buffer2 += format.block();
            }
        }
    }
    */

    data_points = raw_points;
    block_total_data_points += data_points;
}
