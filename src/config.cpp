#include "config.h"
#include <stdio.h>
#include <string.h>

static CONFIG singleton;

const CONFIG& CONFIG::get()
{
	return singleton;
}

void CONFIG::reset()
{
	singleton._reset();
}

void CONFIG::status()
{
	singleton._status();
}

void CONFIG::usage()
{
	singleton._usage();
}

const int CONFIG::analyze_argv( const char argv[][CHR_BUF] )
{
	return singleton._analyze_argv( argv );
}

CONFIG::CONFIG()
{
	_reset();
}

void CONFIG::_reset()
{
	// DC offset
	use_dcoffset = false;
	dcoffset.clear();


	pre_normalization = true;
}

void CONFIG::_status()
{
	if(!pre_normalization){
		fprintf(stderr,"pre-normalization: no\n");
	}
}

void CONFIG::_usage()
{
	// DC offset
	fprintf(stdout,"-ofs left right : adjust DC offset\n");

	fprintf(stdout,"-no-pre-normalization : don't pre-normalize data\n");
}

const int CONFIG::_analyze_argv( const char argv[][CHR_BUF] )
{			
	// DC offset
	if(strcmp(argv[0],"-ofs")==0) {
		use_dcoffset = true;
		dcoffset.push_back( atof(argv[1]) );
		dcoffset.push_back( atof(argv[2]) );
		return 3;
	}


	else if( strncmp(argv[0],"-no-pre-normalization",21)==0){
		pre_normalization = false;
		return 1;
	}

	return 0;
}