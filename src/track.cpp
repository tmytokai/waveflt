// track class

#include <stdio.h>
#include <string.h>

#include "track.h"
#include "filter.h"
#include "io.h"


Track::Track(  const int _track_no, const WaveFormat& _input_format )
    : dbg(false), verbose( true ), track_no(_track_no), input_format(_input_format)
{
    dbg = true;
    data.resize( input_format.channels() );
    reset();
}

Track::~Track()
{}


void Track::reset()
{
    block_no = 0;
}


void Track::start()
{
    reset();
    begin_new_block();
}


void Track::read()
{
    if( end_of_track() ) return;

    raw_points = 0;
    data_points = 0;

    unsigned int raw_read_points = 0;
    if( blockdata[block_no].points == 0 ) raw_read_points = raw_max_points; // endless mode
    else if( blockdata[block_no].points > block_total_raw_points + raw_max_points ) raw_read_points = raw_max_points;
    else raw_read_points = (unsigned int)( blockdata[block_no].points - block_total_raw_points );

    if( split ){ // splitting has occured at split_pos
        
        split = false;
        memmove( raw, raw + split_pos*input_format.block(), split_points*input_format.block() );
        raw_points = split_points;
        raw_read_points -= raw_points;
        if( dbg ) fprintf(stderr, "[debug] Track::read: raw_read_points = %d, raw_points = %d, raw_max_points = %d\n", raw_read_points, raw_points, raw_max_points );
    }

    if( blockdata[block_no].input_type == TYPE_STORAGE || blockdata[block_no].input_type == TYPE_STDIN ){

        raw_read_points = ReadData( fp, raw + raw_points * input_format.block(), raw_read_points * input_format.block() );
        raw_read_points /= input_format.block();

        // think later
//        n64PointerStdin += read_size;  
    }
    else if( blockdata[block_no].input_type == TYPE_NULL ){

        if( input_format.bits() == 8) memset( raw + raw_points * input_format.block(), 0x80, raw_read_points * input_format.block() );
        else memset( raw + raw_points * input_format.block(), 0, raw_read_points * input_format.block() );
    }

    raw_points += raw_read_points;
    block_total_raw_points += raw_points;
    
    if( raw_points == 0 ){
        block_no++;
        begin_new_block();
    }
    else convert_raw_to_data();
}


// splitting occured at pos
void Track::exec_split( const unsigned int pos )
{
    if( end_of_track() ) return;
    if( pos >= raw_points ) return;

    split = true;
    split_pos = pos;
    split_points = raw_points - split_pos;

    raw_points = split_pos;
    block_total_raw_points -= split_points;

    data_points = split_pos;
}


void Track::begin_new_block()
{
    if( end_of_track() ) return;

    raw_points = 0;
    data_points = 0;
    block_total_raw_points = 0;
    split = false;

    if( verbose ){

        fprintf(stderr,"\n\n----------------\n");

        fprintf(stderr,"[Tr_%d: Block_%d : ", track_no, block_no);
        if( blockdata[block_no].offset > 0 ){
            double tmp_offset = (double)(blockdata[block_no].offset - input_format.get_offset());
            if( blockdata[block_no].offset < 1024*1024 )
                fprintf(stderr,"offset = %.2lf K", tmp_offset/1024);
            else
                fprintf(stderr,"offset = %.2lf M", tmp_offset/1024/1024);												
            fprintf(stderr,"(%.2lf sec), ", tmp_offset/input_format.avgbyte());
        }
        if(blockdata[block_no].points > 0){
            unsigned long long size = blockdata[block_no].points*input_format.block();
            if( size < 1024*1024 )
                fprintf(stderr,"size = %.2lf K", (double)size/1024);
            else
                fprintf(stderr,"size = %.2lf M", (double)size/1024/1024);
            fprintf(stderr,"(%.2lf sec)", (double)size/input_format.avgbyte());
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
    if( block_no == 0 || blockdata[block_no].offset != blockdata[block_no-1].offset + blockdata[block_no-1].points*input_format.block() ){

        if( dbg ) fprintf(stderr, "[debug] Track::begin_block: seek: offset = %d\n", (int)blockdata[block_no].offset );

        if( blockdata[block_no].input_type == TYPE_STORAGE ){
#ifndef WIN32
            off_t pos64 = blockdata[block_no].offset;
            fseeko(fp, pos64, SEEK_SET);
#else
            __int64 pos64 = blockdata[block_no].offset;
            _fseeki64( fp , pos64, SEEK_SET);
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
        for( ; it != filters.end(); ++it ){
            (*it)->track_seeked( track_no );
        }
    }
}


void Track::convert_raw_to_data()
{
    if( end_of_track() ) return;
    if( raw_points == 0 ) return;

    for( unsigned int i = 0 ; i < data.size()  ; i++ ) memset( data[i], 0, sizeof(double)*data_max_points );
    data_points = raw_points;
	
    if( input_format.bits() == 8 ){
		
        for( unsigned int i = 0 ; i < input_format.channels()  ; i++){

            unsigned int pos = 0;
            for( unsigned int i2 = 0; i2 < raw_points; ++i2 ){

                data[i][pos] = (double)((int)raw[ i2*input_format.block() +i ]-0x80);
                pos++;
            }
        }
    }

    else if( input_format.bits() == 16 ){  
		
        for( unsigned int i = 0 ; i < input_format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer = raw + sizeof(short)*i;
            for( unsigned int i2=0; i2 < raw_points; ++i2 ){

                data[i][pos] = (double)(*((short*)buffer));
                pos++;
                buffer += input_format.block();
            }
        }
    }

    else if( input_format.bits() == 24 ){
		
        for( unsigned int i = 0 ; i < input_format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer = raw + 3*i;
            for( unsigned int i2=0; i2 < raw_points; ++i2 ){

                int val = 0;
                memcpy( (unsigned char*)(&val)+1, buffer, 3);
                val /= 256;

                data[i][pos] = (double)val;
                pos ++;
                buffer += input_format.block();
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
}
