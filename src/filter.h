// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include <assert.h>
#include "buffer.h"

class Filter
{
private:
	WaveFormat input_format;

protected:
    WaveFormat output_format;

public:
	Filter( const WaveFormat& _input_format ) 
		: input_format( _input_format ), output_format( _input_format ){}
	virtual ~Filter(){}
	const WaveFormat& get_output_format() const { return output_format; }

	virtual void show_config() const = 0;
    virtual void clear_buffer() = 0;
	virtual void inputfile_seeked() = 0;
    virtual void process( Buffer& buffer ) = 0;
	virtual void outputfile_changed() = 0;
	virtual void show_result() const = 0;

protected:
	void check_input_format( const WaveFormat& _input_format ) const{
		assert( _input_format.channels() == input_format.channels() );
		assert( _input_format.rate() == input_format.rate() );
		assert( _input_format.bit()s == input_format.bits() );
	}
};

#endif