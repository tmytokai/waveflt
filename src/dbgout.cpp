// output class for debug message

#include <iostream>

#include "dbgout.h"
#include "storageio.h"


DbgOutStderr::DbgOutStderr()
  : DbgOutBase()
{}


DbgOutStderr::~DbgOutStderr()
{}


void DbgOutStderr::write( const std::string& msg )
{
    std::cerr << msg << std::endl; 
}

//////////////////////////

DbgOutLog::DbgOutLog( const std::string& logfile )
  : DbgOutBase()
{
    io = new StorageIO( 0, logfile );
    io->open( IOMODE_WRITE );
}


DbgOutLog::~DbgOutLog()
{
    if( io ) delete io;
}


void DbgOutLog::write( const std::string& msg )
{
    io->write( msg.c_str(), msg.size() );
#ifdef WIN32
    io->write( "\r\n", 2 );
#else
    io->write( "\n", 1 );
#endif
}
