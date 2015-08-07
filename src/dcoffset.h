// DC offset

#ifndef _DCOFFSET_H
#define _DCOFFSET_H

#include <vector>

#include "filter.h"

class DcOffset : public Filter
{
  private:

    const std::vector<double> offsets;

  public:

    DcOffset( DoubleBuffer& _data, const std::vector<double>& _offsets );
    virtual ~DcOffset();

    // Override
    virtual void connect( Filter* _next );
    virtual void init();
    virtual const unsigned int demanded_max_points();
    virtual const unsigned int demanded_points();
    virtual void show_config() const;
    virtual void start_track();
    virtual void begin_block();
    virtual const unsigned int process( const unsigned int points );
    virtual const bool is_over() const;
    virtual void show_result() const;

  private:

    // Override
    virtual void reset();
};


#endif
