// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include "waveformat.h"

class DoubleBuffer;

class Filter
{
  protected:

    Filter* next;
    WaveFormat in_format;
    WaveFormat out_format;
    DoubleBuffer& data;
    bool dbg;

  public:

    Filter( DoubleBuffer& _data ) : next(NULL), data( _data ), dbg( false ){}
    virtual ~Filter(){}

    void debugmode(){ dbg = true; }
    void connect( Filter* _next ){
        next = _next;
        next->set_in_format( out_format );
    }

    const WaveFormat& get_in_format() const { return in_format; }
    const WaveFormat& get_out_format() const { return out_format; }

    virtual void init() = 0;
    virtual const unsigned int demanded_max_points() = 0;
    virtual const unsigned int demanded_points() = 0;
    virtual void show_config() const = 0;
    virtual void start_track() = 0;
    virtual void begin_block() = 0;
    virtual const unsigned int process( const unsigned int points ) = 0;
    virtual const bool is_over() const = 0;
    virtual void show_result() const = 0;

  private:
    virtual void reset() = 0;
    virtual void set_in_format( const WaveFormat& _in_format ) = 0;
};

#endif
