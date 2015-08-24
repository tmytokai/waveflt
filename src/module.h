// abstract class of module

#ifndef _MODULE_H
#define _MODULE_H

#include <string>
#include <vector>

#include "waveformat.h"
#include "eventdata.h"
#include "host.h"
#include "dbgmsg.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

class DoubleBuffer;

class Module
{
  private:

    const std::string name;
    const unsigned int id;

  protected:

    Module* prev;
    Module* next;

    WaveFormat input_format;
    WaveFormat output_format;

    bool over;
    bool mute;

    unsigned int event_no;
    unsigned long long event_start_point; // point at which the current event started
    unsigned long long event_end_point;   // point at which the current event ends
    std::vector<EventData> events;

    unsigned long long total_processed_points; // total processed points

    bool dbg;
    DbgMsg* dbgmsg;

  public:

    Module( Host* host, const std::string& _name )
        :  name(_name), id(host->get_id()), prev(NULL), next(NULL), over(false), mute(false),event_no(0), event_start_point(0), event_end_point(0), total_processed_points(0), dbg(false),dbgmsg(NULL)
    {
        host->regist(this);
    }

    virtual ~Module(){
        if( dbgmsg ) delete dbgmsg;
        dbgmsg = NULL;
    }

    const unsigned int get_id() const { return id; }
    const std::string& get_name() const { return name; }

    const WaveFormat& get_input_format() const { return input_format; }
    const WaveFormat& get_output_format() const { return output_format; }

    const bool is_over() const { return over; }
    const bool is_mute() const { return mute; }

    void add_event( const EventData& _eventdata ){ events.push_back(_eventdata ); }

    void debugmode(){ if( !dbgmsg ) dbgmsg = new DbgMsg( name, id ); dbg = true; }

    virtual void connect( Module* _next ){ next = _next; next->connected( this ); }
    virtual void connected( Module* _prev ){ prev = _prev; }

	virtual void reset(){
		over = false;
		mute = false;
		event_no = 0;
		event_start_point = 0;
		event_end_point = 0;
		events.clear();
		total_processed_points = 0;
	}
	virtual void clear_buffer(){
		over = false;
		mute = false;
	}
    virtual void init() = 0;
    virtual const std::string get_config() const = 0;
    virtual void start() = 0;
    virtual void exec_event() = 0;
    virtual void requested( const unsigned int points_required ) = 0;
    virtual void received( Module* sender, DoubleBuffer& data ) = 0;
    virtual const std::string get_status() const = 0;
    virtual const std::string get_result() const = 0;
};

#endif
