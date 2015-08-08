// output class

#include <stdio.h>
#include <assert.h>

#include "output.h"
#include "storageio.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

Output::Output( const std::string& _filename )
    : IOModule( "Output", _filename)
{
    reset_all();
}


Output::~Output()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::~Output : %s\n", filename.c_str() );

    reset_all();
}


// Override
void Output::connect( Module* _next )
{
    throw std::string( "cannot connect the output to other modules" );
}


// Override
void Output::reset_all()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::reset_all : %s\n", filename.c_str() );

    // defined in Module class    
    over = false;
    event_no = 0;
    event_start_point = 0;
    event.clear();
    total_processed_points = 0;

    // defined in IOModule class
    if( io ) delete io;
    io = NULL;
    data.reset_all();
    raw_mode = false;
}


// Override
void Output::clear_all_buffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::clear_all_buffer : %s\n", filename.c_str() );

    data.clear_all_buffer();
}


// Override
void Output::init()
{
    if( dbg ) fprintf(stderr, "\n[debug] Output::init : %s\n", filename.c_str() );

    assert( prev );
    input_format = prev->get_output_format();
    output_format = input_format;

    if( prev && prev->get_name() == "Source" ) raw_mode = true;

    const unsigned int max_points = (unsigned int)( 0.25 * output_format.rate() );
    if( dbg ) fprintf(stderr, "\n[debug] Output::init : max_points = %d\n", max_points );

    if( dbg ) data.debugmode();
    data.init( output_format, max_points, !raw_mode, true );

    clear_all_buffer();

    assert( !io ); io = new StorageIO( filename );
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
    if( raw_mode ) cfg += ", raw mode";

    return cfg;
}


// Override
void Output::start()
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::start : %s\n", filename.c_str() );

    over = false;

    total_processed_points = 0;

    clear_all_buffer();

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
void Output::received( Module* sender, DoubleBuffer& _data, const bool fin )
{
    if( dbg ) fprintf( stderr, "\n[debug] Output::received : %s, points = %d, fin = %d\n", filename.c_str(), _data.points, fin );

    assert( prev == sender );

    if( is_over() ) return;

    data << _data;

    if( fin ){

        total_processed_points += data.write_raw( io );
        output_format.write( io, total_processed_points );
        over = true;
        return;
    }
    else if( data.is_full() ){

        total_processed_points += data.write_raw( io );
        data.clear_all_buffer();
    }
}


// Override
void Output::show_result() const
{
    fprintf(stderr,"\n\nRESULT: output %.2lf sec\n", (double)total_processed_points/output_format.rate());
}
