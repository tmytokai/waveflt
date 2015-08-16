#include "dbgmsgbase.h"

DbgOutBase* dbgout = NULL;

DbgOutBase* getDbgOut()
{ 
	return dbgout; 
}

void InitDbgMsg()
{
	dbgout = new DbgOutBase();
}

void ClearDbgMsg()
{
	if( dbgout ) delete dbgout;
}


DbgMsg::DbgMsg( const std::string _name, const int id )
{
	const unsigned int n = 1024;
	char tmp[n];
	snprintf( tmp, n, "%s(ID_%d)", _name.c_str(), id );
	name = tmp;
}


DbgMsg::~DbgMsg(){}


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
