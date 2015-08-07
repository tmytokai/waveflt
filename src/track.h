// track class

#ifndef _TRACK_H
#define _TRACK_H

#include <vector>
#include "filter.h"

class Track
{
  private:

    const unsigned int track_no;
    const std::string filename;

  public:

    Track( const int _track_no, const std::string& _filename, DoubleBuffer& _data );
    virtual ~Track();

    // Override
    virtual void init();
    virtual const unsigned int demanded_max_points() { return 0; }
    virtual const unsigned int demanded_points(){ return 0; }
    virtual void show_config() const;
    virtual void start_track();
    virtual void begin_block(){}
    virtual const unsigned int process( const unsigned int );
    virtual const bool is_over() const;
    virtual void show_result() const;

  private:

    // Override
    virtual void reset();
    virtual void set_in_format( const WaveFormat& ){}
};

#endif
