// DC offset

#ifndef _DCOFFSET_H
#define _DCOFFSET_H

#include <vector>
#include "filter.h"

class DcOffset : public Filter
{
  private:
    std::vector< std::vector<double> > offsets;

  public:
    DcOffset( const WaveFormat& _input_format );
    virtual ~DcOffset();

    void set_offset( const unsigned int track_no, const std::vector<double>& offset );

    // Override
    virtual void show_config() const;
    virtual void clear_buffer();
    virtual void track_seeked( const int track_no );
    virtual void process( std::vector<Track>& tracks );
    virtual void output_changed();
    virtual void show_result() const;
};


#endif
