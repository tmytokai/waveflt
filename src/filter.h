// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include <assert.h>
#include <vector>
#include "track.h"

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
    virtual void clear_all_buffer() = 0;
    virtual void track_seeked( const int track_no ) = 0;
    virtual void process( std::vector<Track>& tracks ) = 0;
    virtual void output_changed() = 0;
    virtual void show_result() const = 0;

  protected:
    void check_rate_of_tracks( const std::vector<Track>& tracks ) const{

        std::vector<Track>::const_iterator it = tracks.begin();
        for(; it != tracks.end(); ++it ){
            assert( (*it).get_format().rate() == input_format.rate() );
        }
    }
};

#endif
