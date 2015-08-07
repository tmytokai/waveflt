// storage IO class

#ifndef _STORAGEIO_H
#define _STORAGEIO_H

#include "io.h"


class StorageIO : public IO
{
    FILE* fp;

  public:

    StorageIO( const std::string& _name );
    virtual ~StorageIO();

    // Override
    virtual void open( const unsigned int mode );
    virtual void close();
    virtual void seek( const unsigned long long offset_byte );
    virtual const unsigned int read( void* data, const unsigned int byte );
    virtual const unsigned int write( const void* data, const unsigned int byte );
};

#endif
