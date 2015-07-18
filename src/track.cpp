// track class

#include <stdio.h>
#include <string.h>

#include "track.h"
#include "filter.h"

// io.h
const unsigned int ReadData( FILE* fp, unsigned char* buffer, const unsigned int size );


Track::Track(  const int _track_no, const WaveFormat& _format )
    : dbg(false), verbose( true ), track_no(_track_no), format(_format)
{
    dbg = true;
    data.resize( format.channels() );
    reset();
}

Track::~Track()
{}

void Track::reset()
{
    block_no = 0;
}

void Track::begin_block()
{
    if( end_of_track() ) return;

    block_total_read_size = 0;
    remain_size = 0;

    if( verbose ){

        fprintf(stderr,"\n\n----------------\n");

        fprintf(stderr,"[Tr_%d: Block_%d : ", track_no, block_no);
        if( blockdata[block_no].offset > 0 ){
            double tmp_offset = (double)(blockdata[block_no].offset - format.get_offset());
            if( blockdata[block_no].offset < 1024*1024 )
                fprintf(stderr,"offset = %.2lf K", tmp_offset/1024);
            else
                fprintf(stderr,"offset = %.2lf M", tmp_offset/1024/1024);												
            fprintf(stderr,"(%.2lf sec), ", tmp_offset/format.avgbyte());
        }
        if(blockdata[block_no].size > 0){ 
            if( blockdata[block_no].size < 1024*1024 )
                fprintf(stderr,"size = %.2lf K", (double)blockdata[block_no].size/1024);
            else
                fprintf(stderr,"size = %.2lf M", (double)blockdata[block_no].size/1024/1024);
            fprintf(stderr,"(%.2lf sec)", (double)blockdata[block_no].size/format.avgbyte());
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
    if( block_no == 0 || blockdata[block_no].offset != blockdata[block_no-1].offset + blockdata[block_no-1].size ){

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


unsigned int Track::read()
{
    if( end_of_track() ) return 0;

    read_size = 0;
    points = 0;

    if( blockdata[block_no].size == 0 ) read_size = buffer_size; // endless mode
    else if( blockdata[block_no].size > block_total_read_size + buffer_size ) read_size = buffer_size;
    else read_size = (unsigned int)( blockdata[block_no].size - block_total_read_size );

    if( remain_size ){
        if( read_size + remain_size > buffer_size ) read_size = buffer_size - remain_size;
        if( dbg )  fprintf(stderr, "[debug] Track::read: remain_size = %d, read_size = %d, buffer_size = %d\n", remain_size, read_size, buffer_size );
    }

    // align the read
    read_size = read_size / ( format.channels() * (format.bits()/8) ) // to avoid optimization
        * format.block();

    if( blockdata[block_no].input_type == TYPE_STORAGE || blockdata[block_no].input_type == TYPE_STDIN ){

        read_size = ReadData( fp, buffer+remain_size, read_size );

        // think later
//        n64PointerStdin += read_size;  
    }
    else if( blockdata[block_no].input_type == TYPE_NULL ){

        if( format.bits() == 8) memset( buffer+remain_size, 0x80, read_size );
        else memset( buffer+remain_size, 0, read_size);
    }

    block_total_read_size += read_size;
    read_size += remain_size;
    remain_size = 0;
    
    if(read_size == 0){
        block_no++;
        begin_block();
    }
    else convert_buffer();

    return read_size;
}


// move all data after offset to head
void Track::cut( const unsigned int offset_point )
{
    remain_size = 0;
    if( end_of_track() ) return;
    if( offset_point >= points ) return;

    const int offset = offset_point * format.block();
    remain_size = read_size - offset;
    memmove( buffer, buffer + offset, remain_size );
}


// convert buffer(unsigned char*) to data(double*)
void Track::convert_buffer()
{
    if( end_of_track() ) return;
    if( read_size == 0 ) return;

    points = read_size / format.block();
    for( unsigned int i = 0 ; i < format.channels()  ; i++) memset( data[i], 0, sizeof(double)*points );
	
    if(format.bits() == 8){
		
        for( unsigned int i = 0 ; i < format.channels()  ; i++){

            unsigned int pos = 0;
            for( unsigned int i2 = 0; i2 < read_size; i2 += format.block() ){

                data[i][pos] = (double)((int)buffer[i2+i]-0x80);
                pos++;
            }
        }
    }

    else if( format.bits() == 16){  
		
        for( unsigned int i = 0 ; i < format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer2 = buffer + sizeof(short)*i;
            for( unsigned int i2=0; i2 < read_size; i2 += format.block() ){

                data[i][pos] = (double)(*((short*)buffer2));
                pos++;
                buffer2 += format.block();
            }
        }
    }

    else if(format.bits() == 24){
		
        for( unsigned int i = 0 ; i < format.channels()  ; i++){

            unsigned int pos = 0;
            unsigned char* buffer2 = buffer + 3*i;
            for( unsigned int i2=0; i2 < read_size; i2 += format.block() ){

                int val = 0;
                memcpy( (unsigned char*)(&val)+1, buffer2, 3);
                val /= 256;

                data[i][pos] = (double)val;
                pos ++;
                buffer2 += format.block();
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
