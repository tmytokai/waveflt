#include <iostream>

#include "dbgmsgbase.h"
#include "storageio.h"


#ifdef WIN32
#define snprintf _snprintf
#endif


DbgOutBase* dbgout = NULL;


DbgOutBase* getDbgOut()
{ 
	return dbgout; 
}


void InitDbgMsg( DbgOutBase* _dbgout )
{
	if( dbgout ) delete dbgout;
	dbgout = _dbgout;
}


void ClearDbgMsg()
{
	if( dbgout ) delete dbgout;
	dbgout = NULL;
}


DbgOutStderr::DbgOutStderr()
: DbgOutBase()
{}


DbgOutStderr::~DbgOutStderr()
{}


void DbgOutStderr::write( const std::string msg )
{
	std::cerr << msg << std::endl; 
}


DbgOutLog::DbgOutLog( const std::string& logfile )
: DbgOutBase()
{
	io = new StorageIO( logfile );
	io->open( IOMODE_WRITE );
}


DbgOutLog::~DbgOutLog()
{
	if( io ) delete io;
}


void DbgOutLog::write( const std::string msg )
{
	io->write( msg.c_str(), msg.size() );
#ifdef WIN32
	io->write( "\r\n", 2 );
#else
	io->write( "\n", 1 );
#endif
}


DbgMsg::DbgMsg( const std::string _name, const int id )
{
	const unsigned int n = 1024;
	char tmp[n];
	snprintf( tmp, n, "%s(ID_%d)", _name.c_str(), id );
	name = tmp;
}


DbgMsg::~DbgMsg()
{}


DbgMsg& DbgMsg::operator << ( const std::string _msg )
{ 
	msg += _msg; 
	return *this; 
}


DbgMsg& DbgMsg::operator << ( const unsigned int a )
{ 
	const unsigned int n = 1024;
	char tmp[n];
	snprintf( tmp, n, "%d", a );
	msg += tmp; 
	return *this; 
}


void DbgMsg::flush()
{
	msg = "[DBG: " + name + "] " + msg;
	getDbgOut()->write( msg );
	msg.clear();
}
