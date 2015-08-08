// source class

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "source.h"
#include "storageio.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

Source::Source( const std::string& _filename )
    : IOModule( "Source", _filename )
{
    reset_all();
}


Source::~Source()
{
    if( dbg ) fprintf( stderr, "\n[debug] Source::~Source : %s\n", filename.c_str() );

    if( next ) delete next;
    next = NULL;
    reset_all();
}


// Override
void Source::connected( Module* )
{
    throw std::string( "cannot connect other modules to the source." );
}


// Override
void Source::reset_all()
{
    if( dbg ) fprintf( stderr, "\n[debug] Source::reset_all : %s\n", filename.c_str() );

    // defined in Module class
    over = false;
    event_no = 0;
    event_start_point = 0;
    event_end_point = 0;
    event.clear();
    total_processed_points = 0;

    // defined in IOModule class
    if( io ) delete io;
    io = NULL;
    data.reset_all();
    raw_mode = false;
}


// Override
void Source::clear_all_buffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] Source::clear_all_buffer : %s\n", filename.c_str() );

    data.clear_all_buffer();
}


// Override
void Source::init()
{
    if( dbg ) fprintf(stderr, "\n[debug] Source::init : %s\n", filename.c_str() );

    assert( !io ); io = new StorageIO( filename );
    if( dbg ) io->debugmode();
    io->open( IOMODE_READ );

    input_format.read( io );
    output_format = input_format;

    unsigned int max_points = 0;
    if( next ){
        next->init();
        max_points = (unsigned int)( 0.5 * input_format.rate() );
        if( next->get_name() == "Output" ) raw_mode = true;
    }
    else max_points = input_format.get_data_points();

    if( dbg ) fprintf(stderr, "\n[debug] Source::init : max_points = %d\n", max_points );

    if( dbg ) data.debugmode();
    data.init( input_format, max_points, !raw_mode, true);

    clear_all_buffer();

    if( !event.size() ){

        EventData eventdata;
        eventdata.message = "read";
        eventdata.points = input_format.get_data_points();
        event.push_back( eventdata );

        eventdata.message = "end";
        event.push_back( eventdata );
    }
}


// Override
const std::string Source::get_config() const
{
    assert( io );

    std::string cfg;
    const size_t n = 1024;
    char tmpstr[n];

    snprintf( tmpstr, n, "%s(ID_%d): %s, %d Hz, %d Ch, %d bits, %.2lf sec"
              , get_name().c_str(), get_id(), io->get_name().c_str()
              , input_format.rate(), input_format.channels(), input_format.bits()
              , (double)input_format.get_data_points()/input_format.rate() );
    cfg += tmpstr;
    if( raw_mode ) cfg += ", raw mode";
    cfg += "\n";

    if( event.size() ){

        unsigned long long total_points = 0;
        unsigned long long points_tmp = 0;
        bool endless = false;
        for( unsigned int i = 0; i < event.size(); ++i ){

            if( event[i].message == "end" ) break;

            snprintf( tmpstr, n, "        %.2lf - ", (double)points_tmp/input_format.rate());
            cfg += tmpstr;
            if(event[i].points != (unsigned long long)(-1) ){
                points_tmp += event[i].points;
                if( event[i].message == "read" ) total_points += event[i].points;
                snprintf( tmpstr, n, "%.2lf (%.2lf sec)"
                          , (double)points_tmp/input_format.rate(), (double)event[i].points/input_format.rate() );
                cfg += tmpstr;
            }
            else endless = true;
            cfg += " : " + event[i].message + " \n";
        }
        if( !endless ){
            snprintf( tmpstr, n,"        total %.2lf sec\n", (double)total_points/input_format.rate());
            cfg += tmpstr;
        }
    }

    if( next ){
        cfg += "=> " + next->get_config();
    }

    return cfg;
}


// Override
void Source::start()
{
    if( dbg ) fprintf( stderr, "\n[debug] Source::start : %s\n", filename.c_str() );

    if( next ) next->start();

    over = false;
    total_processed_points = 0;
    clear_all_buffer();

    event_no = 0;
    exec_event();
}


// Override
void Source::exec_event()
{
    if( is_over() ) return;
    if( !event.size() ) return;
    if( event_no == event.size() ) return;

    if( dbg ) fprintf( stderr, "\n[debug] Source::exec_event : %s, event_no = %d, event = %s\n", filename.c_str(), event_no, event[event_no].message.c_str() );

    event_start_point = total_processed_points;
    event_end_point = event_start_point + event[event_no].points;

    if( event[event_no].message == "read" ){}

    else if( event[event_no].message == "delete" ){
    
        unsigned long long seek_offset = input_format.get_offset();
        for( unsigned int i = 0; i <= event_no ; ++i ){
            if( event[i].message == "read" || event[i].message == "delete" ) seek_offset += event[i].points*input_format.block();
        }
        assert( io ); io->seek( seek_offset );
        total_processed_points += event[event_no].points;
    }

    else if( event[event_no].message == "end" ) over = true;

    if( total_processed_points == event_end_point ){

        ++event_no;
        exec_event();
    }
}


// Override
void Source::requested( const unsigned int points_required )
{
    if( dbg ) fprintf( stderr, "\n[debug] Source::requested : %s, required = %d points\n", filename.c_str(), points_required );

    data.clear_all_buffer();

    if( is_over() || !points_required ){
        if( next ) return next->received( this, data, is_over() );
        return;
    }

    unsigned int points_read = 0;
    if( event_end_point > total_processed_points + data.max_points ) points_read = data.max_points;
    else points_read = (unsigned int)( event_end_point - total_processed_points );
    if( points_read > points_required ) points_read = points_required;

    if( dbg ) fprintf( stderr, "\n[debug] Source::requested : read = %d / %d points\n", points_read, data.max_points );

    points_read = data.read_raw( io, points_read );
    total_processed_points += points_read;

    if( dbg ) fprintf( stderr, "\n[debug] Source::requested : read(actual) %d points\n", points_read );

    if( next ) next->received( this, data, is_over() );

    if( total_processed_points == event_end_point ){

        ++event_no;
        exec_event();
    }
    else if( data.points == 0 ) over = true;
}


// Override
void Source::received( Module* sender, DoubleBuffer& data, const bool fin )
{}


// Override
void Source::show_result() const
{
    if( next ) next->show_result();
}
