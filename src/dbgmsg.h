// debug message class

#ifndef _DBGMSG_H
#define _DBGMSG_H

#include <string>

#include "dbgout.h"

DbgOutBase* getDbgOut();
void InitDbgMsg( DbgOutBase* _dbgout );
void ResetDbgMsg();

class DbgMsg
{
private:
	const std::string name;
	const unsigned int id;

	std::string msg;

public:
	DbgMsg( const std::string& _name, const unsigned int _id );
	~DbgMsg();

	DbgMsg& operator << ( const std::string& _msg );
	DbgMsg& operator << ( const unsigned int a );
	void flush();
};


#endif
