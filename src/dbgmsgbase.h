// debug message class

#ifndef _DBGMSG_H
#define _DBGMSG_H

#include <string>

class IO;

class DbgOutBase
{
public:
	DbgOutBase(){}
	virtual ~DbgOutBase(){}
	virtual void write( const std::string msg ) = 0;
};


class DbgOutStderr : public DbgOutBase
{
public:
	DbgOutStderr();
	virtual ~DbgOutStderr();
	virtual void write( const std::string msg );
};


class DbgOutLog : public DbgOutBase
{
	IO* io;

public:
	DbgOutLog( const std::string& logfile );
	virtual ~DbgOutLog();
	virtual void write( const std::string msg );
};


DbgOutBase* getDbgOut();
void InitDbgMsg( DbgOutBase* _dbgout );
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