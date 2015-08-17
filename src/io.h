// abstract class of IO source

#ifndef _IO_H
#define _IO_H

#include <string>

#include "dbgmsgbase.h"

enum
{
    IOMODE_READ = 0,
    IOMODE_WRITE,
    IOMODE_INIT
};


class IO
{
private:

	const std::string name;
	const unsigned int id;

  protected:

    std::string target;
    unsigned int mode;

	DbgMsg* dbgmsg;

  public:

    IO( const std::string& _name, const unsigned int _id, const std::string& _target)
		: name(_name), id(_id), target(_target), mode(IOMODE_INIT), dbgmsg(NULL){}
	virtual ~IO(){
		if( dbgmsg ) delete dbgmsg;
		dbgmsg = NULL;
	}

	const unsigned int get_id() const { return id; }
	const std::string& get_name() const { return name; }
    const std::string& get_target() const { return target; }
    const unsigned int get_mode() const { return mode; }

	void debugmode(){ if( !dbgmsg ) dbgmsg = new DbgMsg( name, id ); }

    virtual void open( const unsigned int mode ) = 0;
    virtual void close() = 0;
    virtual void seek( const unsigned long long offset_byte ) = 0;
    virtual const unsigned int read( void* data, const unsigned int byte ) = 0;
    virtual const unsigned int write( const void* data, const unsigned int byte ) = 0;
};

#endif
