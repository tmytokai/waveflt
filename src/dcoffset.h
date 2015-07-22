// DC offset

#ifndef _DCOFFSET_H
#define _DCOFFSET_H

#include "filter.h"

class DcOffset : public Filter
{
  private:
    std::vector<double> offset;

  public:
    DcOffset( const WaveFormat& _data_format, std::vector<double*>& _data, const std::vector<double>& _offset );
    virtual ~DcOffset();

    // Override
    virtual void show_config() const;
    virtual void clear();
    virtual void process( const unsigned int data_points );
    virtual void show_result() const;
};


#endif
