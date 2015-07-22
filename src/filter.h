// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include <vector>

#include "waveformat.h"

class Filter
{
  protected:
    WaveFormat data_format;
    std::vector<double*> data;
    bool dbg;

  public:
    Filter( const WaveFormat& _data_format, std::vector<double*>& _data )
      : data_format( _data_format ), data( _data ), dbg( false ){}
    virtual ~Filter(){}

    void debugmode(){ dbg = true; }

    virtual void show_config() const = 0;
    virtual void clear() = 0;
    virtual void process( const unsigned int data_points ) = 0;
    virtual void show_result() const = 0;
};

#endif
