// abstract class of filter

#ifndef _FILTER_H
#define _FILTER_H

#include <assert.h>
#include "buffer.h"

class Filter
{
private:
	WAVFMT input_format;

protected:
    WAVFMT output_format;

public:
	Filter( const WAVFMT& _input_format ) 
		: input_format( _input_format ), output_format( _input_format ){}
	virtual ~Filter(){}
	const WAVFMT& get_output_format() const { return output_format; }

	virtual void show_config() = 0;
    virtual void clear_buffer() = 0;
    virtual void process( Buffer& buffer ) = 0;
	virtual void file_changed() = 0;
	virtual void show_result() = 0;

protected:
	void check_input_format( const WAVFMT& _input_format ){
		assert( _input_format.channels == input_format.channels );
		assert( _input_format.rate == input_format.rate );
		assert( _input_format.bits == input_format.bits );
	}
};

#endif