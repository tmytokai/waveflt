// track class

#ifndef _TRACK_H
#define _TRACK_H

#include <vector>
#include "waveformat.h"

class Track
{
  public:
    std::vector<double*> raw;
    unsigned int points;
    WaveFormat format;

    Track( const WaveFormat& _format):format(_format){
        raw.resize( format.channels() );
    }

    const unsigned int get_points() const { return points; }
    const WaveFormat& get_format() const { return format; }
};

#endif
