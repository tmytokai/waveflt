// buffer class

#include "doublebuffer.h"
#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

DoubleBuffer::DoubleBuffer()
    : dbg(false), raw(NULL)
{
    reset_all();
}


DoubleBuffer::~DoubleBuffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::~DoubleBuffer\n" );

    reset_all();
}


double* DoubleBuffer::operator [] ( const unsigned int ch )
{
    assert( data.size() > ch );
    assert( data[ch] );
    return data[ch];
}


// copy buffer
const DoubleBuffer& DoubleBuffer::operator << ( const DoubleBuffer& buffer )
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::operator << : points = %d, buffer.points = %d\n", points, buffer.points );

    if( data.size() && buffer.data.size() ){

        for( unsigned int i = 0; i < format.channels(); ++i )
            memcpy( data[i] + points, buffer.data[i], sizeof(double) * buffer.points );
    }

    if( raw && buffer.raw ) memcpy( raw + points*format.block(), buffer.raw, buffer.points*format.block() );

    points += buffer.points;
    assert( points <= max_points );

	over = buffer.over;

    return buffer;
}


void DoubleBuffer::reset_all()
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::reset_all\n" );

    max_points = 0;
    points = 0;
	over = false;

    if( data.size() ){
        for( unsigned int i = 0; i < data.size(); ++i ){
            assert( data[i] ); free( data[i] );
        }
        data.clear();
    }

    if( raw ) free( raw );
    raw = NULL;
}


void DoubleBuffer::clear_all_buffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::clear_all_buffer: max_points = %d\n", max_points);

    points = 0;
	over = false;

    if( data.size() > 0 ){

        for( unsigned int i = 0 ; i < format.channels()  ; ++i ){
            assert( data[i] ); memset( data[i], 0, sizeof(double) * max_points );
        }
    }

    if( raw ) memset( raw, 0, max_points * format.block() );
}


void DoubleBuffer::init( const WaveFormat& _format, const int _max_points, const bool use_data, const bool use_raw )
{
    format = _format;

    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::init : channels = %d, max_points = %d, use_data = %d, use_raw = %d\n", format.channels(), _max_points, use_data, use_raw );

    assert( !data.size() );
    assert( !raw );

    max_points = _max_points;

    if( use_data ){
        data.resize( format.channels() );
        for( unsigned int i = 0; i < format.channels(); ++i ) data[i] = (double*) malloc( sizeof(double) * max_points );
    }

    if( use_raw ){
         raw = (unsigned char*) malloc( max_points * format.block() );
    }
}


const unsigned int DoubleBuffer::read_raw( IO* io, const unsigned int _points_read )
{
    assert( io );
    assert( raw );
    assert( _points_read <= max_points );

    const unsigned int points_read = io->read( raw, _points_read * format.block() ) / format.block();
    points = points_read;
    if( data.size() > 0 ) convert_raw_to_double();
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::read_raw : read %d points\n", points_read );

    return points_read;
}


const unsigned int DoubleBuffer::write_raw( IO* io )
{
    assert( io );
    assert( raw );

    if( data.size() > 0 ) convert_double_to_raw();

    const unsigned int points_write = io->write( raw, points * format.block() ) / format.block();
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::write_raw : write %d points\n", points_write );

    return points_write;
}


// private
void DoubleBuffer::convert_raw_to_double()
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::convert_raw_to_double\n" );

    assert( data.size() > 0 );
    assert( raw );

    for( unsigned int i = 0 ; i < format.channels()  ; i++){

        if( format.bits() == 8 ){

            for( unsigned int i2 = 0; i2 < points; ++i2 ){

                data[i][i2] = (double)((int)raw[ i2*format.block() +i ]-0x80);
            }
        }

        else if( format.bits() == 16 ){  
		
            unsigned char* buffer = raw + sizeof(short)*i;
            for( unsigned int i2=0; i2 < points; ++i2 ){

                data[i][i2] = (double)(*((short*)buffer));
                buffer += format.block();
            }
        }

        else if( format.bits() == 24 ){

            unsigned char* buffer = raw + 3*i;
            for( unsigned int i2=0; i2 < points; ++i2 ){

                int val = 0;
                memcpy( (unsigned char*)(&val)+1, buffer, 3);
                val /= 256;

                data[i][i2] = (double)val;
                buffer += format.block();
            }
        }

        else if( format.bits() == 32 && format.tag() == WAVE_FORMAT_PCM ){
		
            unsigned char* buffer = raw + sizeof(int)*i;
            for( unsigned int i2=0; i2 < points; ++i2 ){

                data[i][i2] = (double)(*((int*)buffer));
                buffer += format.block();
            }
        }

        else if( format.bits() == 32 && format.tag() == WAVE_FORMAT_IEEE_FLOAT){

            unsigned char* buffer = raw + sizeof(float)*i;
            for( unsigned int i2=0; i2 < points; ++i2 ){

                data[i][i2] = (double)(*((float*)buffer));
                buffer += format.block();
            }
        }

        else if( format.bits() == 64 ){

            unsigned char* buffer = raw + sizeof(double)*i;
            for( unsigned int i2=0; i2 < points; ++i2 ){

                data[i][i2] = (double)(*((double*)buffer));
                buffer += format.block();
            }
        }
    }
}


// private
void DoubleBuffer::convert_double_to_raw()
{
    if( dbg ) fprintf( stderr, "\n[debug] DoubleBuffer::convert_double_to_raw\n" );

    assert( data.size() > 0 );
    assert( raw );

    for( unsigned int i = 0; i < format.channels(); ++i ){
        for( unsigned int i2 = 0; i2 < points; ++i2 )
            *((short*)( raw + i * format.bits()/8 + i2 * format.block() ) ) = (short)(data[i][i2]);
    }
}
