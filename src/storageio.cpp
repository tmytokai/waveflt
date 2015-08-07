// storage source class

#include <stdio.h>
#include <assert.h>

#include "storageio.h"


StorageIO::StorageIO( const std::string& _name )
    : IO( _name), fp(NULL)
{}


StorageIO::~StorageIO()
{
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::~StorageIO : %s\n", name.c_str() );
    close();
}


// Override
void StorageIO::open( const unsigned int _mode )
{
    mode = _mode;
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::open : %s mode = %d\n", name.c_str(), mode );

    if( !fp ){
        if( mode == IOMODE_READ ) fp = fopen( name.c_str(), "rb" );
        else fp = fopen( name.c_str(), "wb" );
        if(fp == NULL) throw std::string( "Cannot open " + name );
    }
}


// Override
void StorageIO::close()
{
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::close : %s\n", name.c_str() );

    if( fp ) fclose( fp );
    fp = NULL;
}


// Override
void StorageIO::seek( const unsigned long long offset_byte )
{
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::seek : %s, offset = %d byte\n", name.c_str(), (unsigned int)offset_byte );
    assert(fp);

#ifndef WIN32
    fseeko(fp, (off_t)offset_byte, SEEK_SET);
#else
    _fseeki64( fp , (__int64)offset_byte, SEEK_SET);
#endif
}


// Override
const unsigned int StorageIO::read( void* data, const unsigned int byte )
{
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::read : %s, byte = %d byte\n", name.c_str(), byte );
    assert(fp);

    if( byte == 0 ) return 0;
    return fread( data, 1, byte, fp );
}


// Override
const unsigned int StorageIO::write( const void* data, const unsigned int byte )
{
    if( dbg ) fprintf( stderr, "\n[debug] StorageIO::write : %s, byte = %d byte\n", name.c_str(), byte );
    assert(fp);

    if( byte == 0 ) return 0;
    return fwrite( data, 1, byte, fp );
}
