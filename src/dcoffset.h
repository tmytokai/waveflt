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
    DcOffset( const WaveFormat& _input_format );
    DcOffset( const WaveFormat& _input_format, const std::vector<double>& _offset );
	virtual ~DcOffset();

    void set_offset( const int channel, const double value );
    const double get_offset( const int channel ) const;

	virtual void show_config() const;
    virtual void clear_buffer();
	virtual void inputfile_seeked();
    virtual void process( Buffer& buffer );
	virtual void outputfile_changed();
	virtual void show_result() const;
};


#endif