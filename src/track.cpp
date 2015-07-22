// track class

#include <stdio.h>
#include <string.h>

#include "track.h"
#include "filter.h"
#include "io.h"


Track::Track(  const int _track_no, const WaveFormat& _raw_format, const WaveFormat& _data_format )
    : dbg(false), verbose( true ), track_no(_track_no), raw_format(_raw_format), data_format(_data_format)
{
    dbg = true;
    data.resize( data_format.channels() );
}


Track::~Track()
{}


void Track::start()
{
    block_no = 0;
    begin_new_block();
}


void Track::read()
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
    else convert_raw_to_data();
}


void Track::begin_new_block()
{
    if( end_of_track() ) return;

    raw_points = 0;
    data_points = 0;
    block_total_data_points = 0;

    if( verbose ){

        fprintf(stderr,"\n\n----------------\n");

        fprintf(stderr,"[Tr_%d: Blk_%d : ", track_no, block_no);
        if( blockdata[block_no].input_type == TYPE_STORAGE || blockdata[block_no].input_type == TYPE_STDIN ){

            const unsigned long long raw_offset = blockdata[block_no].raw_offset*raw_format.block();
            if( raw_offset < 1024*1024 )
                fprintf(stderr,"offset = %.2lf K", (double)raw_offset/1024);
            else
                fprintf(stderr,"offset = %.2lf M", (double)raw_offset/1024/1024);												
            fprintf(stderr,"(%.2lf sec), ", (double)raw_offset/raw_format.avgbyte());
        }

        if(blockdata[block_no].raw_points > 0){

            const unsigned long long raw_size = blockdata[block_no].raw_points*raw_format.block();
            if( raw_size < 1024*1024 )
                fprintf(stderr,"size = %.2lf K", (double)raw_size/1024);
            else
                fprintf(stderr,"size = %.2lf M", (double)raw_size/1024/1024);
            fprintf(stderr,"(%.2lf sec)", (double)raw_size/raw_format.avgbyte());
        }
        else fprintf(stderr," size = endless");
        fprintf(stderr,"]\n");

        fprintf(stderr,"INPUT: ");
        switch( blockdata[block_no].input_type ){
            case TYPE_STORAGE:
                fprintf(stderr,"%s\n",filename.c_str());
                break;
            case TYPE_STDIN:
                fprintf(stderr,"stdin\n");
                break;
            case TYPE_NULL:
                fprintf(stderr,"null\n");
                break;
        }
    }

    // move to head of current block
    if( blockdata[block_no].input_type == TYPE_STORAGE ){

        const unsigned long long seek_offset = raw_format.get_offset() + blockdata[block_no].raw_offset*raw_format.block();

        if( dbg ) fprintf(stderr, "[debug] Track::begin_new_block: seek: offset = %d\n", (int)seek_offset );
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
    std::vector<Filter*>::iterator it = filters.begin();
    for( ; it != filters.end(); ++it ) (*it)->track_seeked( track_no );
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

    data_points = raw_points;
    block_total_data_points += data_points;

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
}
