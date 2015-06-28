// DC offset

#ifndef _DCOFFSET_H
#define _DCOFFSET_H

#include <vector>
#include "filter.h"

class DcOffset : public Filter
{
private:
    std::vector<double> offset;

public:
    DcOffset( const WAVFMT& _input_format );
    DcOffset( const WAVFMT& _input_format, const std::vector<double>& _offset );
	virtual ~DcOffset();

    void set_offset( const int channel, const double value );
    const double get_offset( const int channel );

	virtual void show_config();
    virtual void clear_buffer();
    virtual void process( Buffer& buffer );
	virtual void file_changed();
	virtual void show_result();
};


#endif