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
	fprintf(stdout,"-no-pre-normalization : don't pre-normalize data\n");
}

const int CONFIG::_analyze_argv( const char argv[][CHR_BUF] )
{
	if( strncmp(argv[0],"-no-pre-normalization",21)==0){
		pre_normalization = false;
		return 1;
	}

	return 0;
}