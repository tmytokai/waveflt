// buffer class

#ifndef _BUFFER_H
#define _BUFFER_H

#include <vector>
#include "waveformat.h"

class Buffer
{
public:
	std::vector<double*> buffer;
    unsigned int points;
    WAVFMT format;

	Buffer( const WAVFMT& _format):format(_format){
		buffer.resize( format.channels );
	}
};

#endif