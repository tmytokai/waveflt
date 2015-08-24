// output class

#include <stdio.h>
#include <assert.h>

#include "output.h"
#include "storageio.h"


Output::Output( Host* host, const std::string& _filename )
    : IOModule( host, "Output", _filename)
{
    reset();
}


Output::~Output()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::~Output : %s\n", filename.c_str() );

    reset();
}


// Override
void Output::connect( Module* _next )
{
    throw std::string( "cannot connect the output to other modules" );
}


// Override
void Output::reset()
{
    if( dbg ) fprintf( stderr, "reset" );

	IOModule::reset();
}


// Override
void Output::clear_buffer()
{
    if( dbg ) fprintf( stderr, "clear_buffer" );

	IOModule::clear_buffer();
}


// Override
void Output::init()
{
    if( dbg ) fprintf(stderr, "\n[debug] Output::init : %s\n", filename.c_str() );

    assert( prev );
    input_format = prev->get_output_format();
    output_format = input_format;

    const unsigned int max_points = (unsigned int)( 0.25 * output_format.rate() );
    if( dbg ) fprintf(stderr, "\n[debug] Output::init : max_points = %d\n", max_points );

	bool use_data = true; // if Source and Output are connected directly, send only raw data
    if( prev && prev->get_name() == "Source" ) use_data = false;

    if( dbg ) data.debugmode();
    data.init( output_format, max_points, use_data, true );

    clear_buffer();

    assert( !io ); io = new StorageIO( get_id(), filename );
    if( dbg ) io->debugmode();
}


// Override
const std::string Output::get_config() const
{
    std::string cfg;
    const size_t n = 1024;
    char tmpstr[n];

    snprintf( tmpstr, n, "%s(ID_%d): %s, %d (Hz), %d (Ch), %d (Bits)"
              , get_name().c_str(), get_id(), filename.c_str()
              , output_format.rate(), output_format.channels(), output_format.bits() );
    cfg += tmpstr;

    return cfg;
}


// Override
void Output::start()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::start : %s\n", filename.c_str() );

    over = false;

    total_processed_points = 0;

    clear_buffer();

    assert(io);{
        io->open( IOMODE_WRITE );
        io->seek( 44 );
    }
}


// Override
void Output::exec_event()
{}


// Override
void Output::requested( const unsigned int points_required )
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::requested : %s, required = %d\n", filename.c_str(), points_required );

    if( is_over() || points_required == 0 ) return;
    assert( prev ); prev->requested( points_required );
}


// Override
void Output::received( Module* sender, DoubleBuffer& _data )
{
	if( dbg ) fprintf( stderr, "\n[debug] Output::received : %s, points = %d, over = %d\n", filename.c_str(), _data.points, _data.over );

    assert( prev == sender );

    if( is_over() ) return;

	if( _data.mute ){

		if( !is_mute() ) clear_buffer();
		mute = true;
	}
	
	data << _data;

	if( data.over ){

        total_processed_points += data.write_raw( io );
        output_format.write( io, total_processed_points );
        over = true;
        return;
    }
    else if( data.is_full() ){

        total_processed_points += data.write_raw( io );
        clear_buffer();
    }
}


// Override
const std::string Output::get_status() const
{
    const size_t n = 1024;
    char status[n];

    snprintf(status, n, "%s(ID_%d): %.2lf sec"
             , get_name().c_str(), get_id(), (double)total_processed_points/output_format.rate());
	
    return status;
}


// Override
const std::string Output::get_result() const
{
    const size_t n = 1024;
    char result[n];

    snprintf(result, n, "%s(ID_%d): %s, total output size = %.2lf sec"
             , get_name().c_str(), get_id(), filename.c_str(), (double)total_processed_points/output_format.rate());
	
    return result;
}
