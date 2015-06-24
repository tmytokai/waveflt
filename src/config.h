// global configuration

#ifndef _WFLT_CONFIG_H
#define _WFLT_CONFIG_H

#ifndef CHR_BUF
#define CHR_BUF 256
#endif


class CONFIG
{
public:
	const static CONFIG& get();
	static void reset();
	static void status();
	static void usage();
	static const int analyze_argv( const char argv[][CHR_BUF] );

	CONFIG();

	// pre-normalize data between -1 to 1 before filtering
	bool pre_normalization;

private:
	void _reset();
	void _status();
	void _usage();
	const int _analyze_argv( const char argv[][CHR_BUF] );
};

#endif