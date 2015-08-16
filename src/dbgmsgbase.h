// debug message class

#ifndef _DBGMSG_H
#define _DBGMSG_H

#include <string>
#include <iostream>

#ifdef WIN32
#define snprintf _snprintf
#endif


class DbgOutBase
{
public:
	DbgOutBase(){}
	virtual ~DbgOutBase(){}
	virtual void write( const std::string msg ){ std::cerr << msg << std::endl; }
};


DbgOutBase* getDbgOut();
void InitDbgMsg();
void ClearDbgMsg();


class DbgMsg
{
protected:
	std::string name;
	std::string msg;

public:
	DbgMsg( const std::string _name, const int id );
	~DbgMsg();

	DbgMsg& operator << ( const std::string _msg );
	DbgMsg& operator << ( const unsigned int a );
	void flush();
};


#endif