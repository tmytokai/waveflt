// track class

#include <stdio.h>
#include <string.h>

#include "track.h"
#include "filter.h"

// io.h
const unsigned int ReadData( FILE* fp, unsigned char* buffer, const unsigned int size );


Track::Track(  const int _track_no, const WaveFormat& _format )
    : dbg(false), track_no(_track_no), format(_format)
{
    dbg = true;
    data.resize( format.channels() );
    reset();
}

Track::~Track()
{
}

void Track::reset()
{
    block_no = 0;
}

void Track::begin_block()
{
    if( end_of_track() ) return;

    block_total_read_size = 0;
    remain_size = 0;

    fprintf(stderr,"\n\n----------------\n");

    fprintf(stderr,"[Tr_%d: Block_%d : ", track_no, block_no);
    if( blockdata[block_no].offset > 0 ){
        double tmp_offset = blockdata[block_no].offset - format.get_offset();
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
/*
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

    return read_size;
}


void Track::cut( const unsigned int offset )
{
    if( end_of_track() ) return;

    // move all data after offset to head
    remain_size = read_size - offset;
    memmove( buffer, buffer + offset, remain_size );
}
