// storage source class

#include <stdio.h>
#include <assert.h>

#include "storageio.h"


StorageIO::StorageIO( const unsigned int _id, const std::string& _target )
    : IO( "StorageIO", _id, _target), fp(NULL)
{}


StorageIO::~StorageIO()
{
	if( dbgmsg ) (*dbgmsg << "~StorageIO").flush();
    close();
}


// Override
void StorageIO::open( const unsigned int _mode )
{
    mode = _mode;
	if( dbgmsg ) (*dbgmsg << "open : " << target << " mode = " << mode ).flush();

    if( !fp ){
        if( mode == IOMODE_READ ) fp = fopen( target.c_str(), "rb" );
        else fp = fopen( target.c_str(), "wb" );
        if(fp == NULL) throw std::string( "Cannot open " + target );
    }
}


// Override
void StorageIO::close()
{
	if( dbgmsg ) (*dbgmsg << "close : " << target ).flush();

    if( fp ) fclose( fp );
    fp = NULL;
}


// Override
void StorageIO::seek( const unsigned long long offset_byte )
{
	if( dbgmsg ) (*dbgmsg << "seek : " << target << ", offset = " << (unsigned int)offset_byte << " byte" ).flush();
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
	if( dbgmsg ) (*dbgmsg << "read : " << target << ", byte = " << byte << " byte").flush();
    assert(fp);

    if( byte == 0 ) return 0;
    return fread( data, 1, byte, fp );
}


// Override
const unsigned int StorageIO::write( const void* data, const unsigned int byte )
{
    if( dbgmsg ) (*dbgmsg << "write : " << target << ", byte = " << byte << " byte").flush();
    assert(fp);

    if( byte == 0 ) return 0;
    return fwrite( data, 1, byte, fp );
}
