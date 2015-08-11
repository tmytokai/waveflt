// buffer class

#include <vector>

#ifndef _DOUBLEBUFFER_H
#define _DOUBLEBUFFER_H

#include "waveformat.h"

class IO;

class DoubleBuffer
{
  private:

    bool dbg;
    std::vector<double*> data;
    unsigned char* raw;   // buffer for raw data

    WaveFormat format;

  public:

    unsigned int max_points;  // max points of buffer
    unsigned int points; // current points in buffer
	bool over;

    DoubleBuffer();
    ~DoubleBuffer();

    void debugmode(){ dbg = true; }
    const unsigned int left() const{ return max_points - points; }
    const bool is_full() const{ return max_points == points; }

    double* operator [] ( const unsigned int ch );
    unsigned char* get_raw(){ return raw; }
    const DoubleBuffer& operator << ( const DoubleBuffer& buffer ); // copy buffer

    void reset_all();
    void clear_all_buffer();
    void init( const WaveFormat& _format, const int _max_points, const bool use_data, const bool use_raw );
    const unsigned int read_raw( IO* io, const unsigned int _points_read );
    const unsigned int write_raw( IO* io );

  private:

    void convert_raw_to_double();
    void convert_double_to_raw();
};

#endif
