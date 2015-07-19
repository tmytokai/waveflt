// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include <assert.h>
#include <vector>
#include "trackmanager.h"

class Filter
{
  private:
    WaveFormat input_format;

  protected:
    WaveFormat output_format;
    bool dbg;

  public:
    Filter( const WaveFormat& _input_format )
        : input_format( _input_format ), output_format( _input_format ), dbg( false ){}
    virtual ~Filter(){}

    const WaveFormat& get_input_format() const { return input_format; }
    const WaveFormat& get_output_format() const { return output_format; }

    virtual void show_config() const = 0;
    virtual void clear_buffer() = 0;
    virtual void track_seeked( const int track_no ) = 0;
    virtual void process( TrackManager& trackmanager ) = 0;
    virtual void output_changed() = 0;
    virtual void show_result() const = 0;
};

#endif
