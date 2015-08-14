// debug message class

#ifndef _DBGMSGBASE_H
#define _DBGMSGBASE_H

#include <string>
#include <iostream>

#ifdef WIN32
#define snprintf _snprintf
#endif

class DbgMsgBase
{
protected:
	std::string name;
	std::string msg;

public:
	DbgMsgBase( const std::string _name, const int id ){
		const unsigned int n = 1024;
		char tmp[n];
		snprintf( tmp, n, "%s(ID_%d)", _name.c_str(), id );
		name = tmp;
	}
	virtual ~DbgMsgBase(){}

	DbgMsgBase& operator << ( const std::string _msg ){ msg += _msg; return *this; }

	DbgMsgBase& operator << ( const unsigned int a ){ 
		const unsigned int n = 1024;
		char tmp[n];
		snprintf( tmp, n, "%d", a );
		msg += tmp; 
		return *this; 
	}

	virtual void flush() = 0;
};


class DbgMsgStderr : public DbgMsgBase
{
public:
	DbgMsgStderr( const std::string _name, const int id ) : DbgMsgBase(_name,id){}
	virtual ~DbgMsgStderr(){}

	virtual void flush(){
		std::cerr << "[DBG: " << name << "] " << msg << std::endl;
		msg.clear();
	}
};

#endif