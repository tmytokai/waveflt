// abstract class of module

#ifndef _MODULE_H
#define _MODULE_H

#include <string>
#include <vector>

#include "waveformat.h"
#include "eventdata.h"

class DoubleBuffer;

class Module
{
  private:

    const unsigned int id;
    const std::string name;

  protected:

    Module* prev;
    Module* next;

    WaveFormat input_format;
    WaveFormat output_format;

    bool over;

    unsigned int event_no;
    unsigned long long event_start_point; // point at which the current event started
    std::vector<EventData> event;

    unsigned long long total_processed_points; // total processed points

    bool dbg;

  public:

    Module( const std::string& _name )
        : id(0), name(_name), prev(NULL), next(NULL), over(false), event_no(0), event_start_point(0), total_processed_points(0), dbg(false){}
    virtual ~Module(){}

    const unsigned int get_id() const { return id; }
    const std::string& get_name() const { return name; }

    const WaveFormat& get_input_format() const { return input_format; }
    const WaveFormat& get_output_format() const { return output_format; }

    const bool is_over() const { return over; }

    void set_event( const std::vector<EventData>& _event ){ event = _event; }

    void debugmode(){ dbg = true; }

    virtual void connect( Module* _next ){ next = _next; next->connected( this ); }
    virtual void connected( Module* _prev ){ prev = _prev; }

    virtual void reset_all() = 0;
    virtual void clear_all_buffer() = 0;
    virtual void init() = 0;
    virtual void show_config() const = 0;
    virtual void start() = 0;
    virtual void exec_event() = 0;
    virtual void requested( const unsigned int points_required ) = 0;
    virtual void received( Module* sender, DoubleBuffer& data, const bool fin ) = 0;
    virtual void show_result() const = 0;
};

#endif
