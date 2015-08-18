// debug message class

#include <stdio.h>

#include "dbgmsg.h"

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


void ResetDbgMsg()
{
	if( dbgout ) delete dbgout;
	dbgout = NULL;
}


DbgMsg::DbgMsg( const std::string& _name, const unsigned int _id )
: name(_name), id(_id)
{
	msg.clear();
}


DbgMsg::~DbgMsg()
{}


DbgMsg& DbgMsg::operator << ( const std::string& _msg )
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
	const unsigned int n = 1024;
	char tmp[n];
	snprintf( tmp, n, "[DBG %s(ID_%d)] ", name.c_str(), id );

	msg = tmp + msg;
	getDbgOut()->write( msg );
	msg.clear();
}
