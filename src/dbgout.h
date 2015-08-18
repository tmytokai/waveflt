// output class for debug message

#ifndef _DBGOUT_H
#define _DBGOUT_H

#include <string>

class DbgOutBase
{
  public:

    DbgOutBase(){}
    virtual ~DbgOutBase(){}
    virtual void write( const std::string& msg ) = 0;
};

//////////////////////////

class DbgOutStderr : public DbgOutBase
{
  public:

    DbgOutStderr();
    virtual ~DbgOutStderr();
    virtual void write( const std::string& msg );
};

//////////////////////////

class IO;

class DbgOutLog : public DbgOutBase
{
    IO* io;

  public:

    DbgOutLog( const std::string& logfile );
    virtual ~DbgOutLog();
    virtual void write( const std::string& msg );
};

#endif
