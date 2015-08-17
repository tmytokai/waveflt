// source class

#include <assert.h>

#include "source.h"
#include "storageio.h"


Source::Source( const std::string& _filename )
    : IOModule( "Source", _filename )
{
    reset();
}


Source::~Source()
{
	if( dbgmsg ) (*dbgmsg << "~Source" ).flush();

    reset();
}


// Override
void Source::connected( Module* )
{
    throw std::string( "cannot connect other modules to the source." );
}


// Override
void Source::reset()
{
	if( dbgmsg ) (*dbgmsg << "reset" ).flush();

	IOModule::reset();
}


// Override
void Source::clear_buffer()
{
	if( dbgmsg ) (*dbgmsg << "clear_buffer" ).flush();

	IOModule::clear_buffer();
}


// Override
void Source::init()
{
	if( dbgmsg ) (*dbgmsg << "init" ).flush();

    assert( !io ); io = new StorageIO( filename );
    if( dbgmsg ) io->debugmode();
    io->open( IOMODE_READ );

    input_format.read( io );
    output_format = input_format;

    unsigned int max_points = 0;
	bool use_data = true; // if Source and Output are connected directly, send only raw data
	if( next ){
        next->init();
        max_points = (unsigned int)( 0.5 * input_format.rate() );
        if( next->get_name() == "Output" ) use_data = false;
    }
    else max_points = input_format.get_data_points();

	if( dbgmsg ) (*dbgmsg << "init: max_points = " << max_points ).flush();

    if( dbg ) data.debugmode();
    data.init( input_format, max_points, use_data, true);

    clear_buffer();

	// setup default events
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
              , get_name().c_str(), get_id(), io->get_target().c_str()
              , input_format.rate(), input_format.channels(), input_format.bits()
              , (double)input_format.get_data_points()/input_format.rate() );
    cfg += tmpstr;
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
	if( dbgmsg ) (*dbgmsg << "start" ).flush();

    if( next ) next->start();

    clear_buffer();
    total_processed_points = 0;

    event_no = 0;
    exec_event();
}


// Override
void Source::exec_event()
{
    if( is_over() ) return;
    if( !event.size() ) return;
    if( event_no == event.size() ) return;

	if( dbgmsg ) (*dbgmsg << "exec_event : event_no = " << event_no << " event = " << event[event_no].message ).flush();

    event_start_point = total_processed_points;
    event_end_point = event_start_point + event[event_no].points;

    if( event[event_no].message == "read" ) mute = false;

    else if( event[event_no].message == "delete" ){
    
        unsigned long long seek_offset = input_format.get_offset();
        for( unsigned int i = 0; i <= event_no ; ++i ){
            if( event[i].message == "read" || event[i].message == "delete" ) seek_offset += event[i].points*input_format.block();
        }
        assert( io ); io->seek( seek_offset );
        total_processed_points += event[event_no].points;
    }

    else if( event[event_no].message == "end" ) over = true;
	    
	else if( event[event_no].message == "mute" ) mute = true;

    if( total_processed_points == event_end_point ){

        ++event_no;
        exec_event();
    }
}


// Override
void Source::requested( const unsigned int points_required )
{
	if( dbgmsg ) (*dbgmsg << "requested : required = " << points_required << " points" ).flush();

    data.clear_buffer();

    if( is_over() || !points_required ){
		data.over = is_over();
        if( next ) return next->received( this, data );
        return;
    }

    unsigned int points_read = 0;
    if( event_end_point > total_processed_points + data.max_points ) points_read = data.max_points;
    else points_read = (unsigned int)( event_end_point - total_processed_points );
    if( points_read > points_required ) points_read = points_required;

	if( dbgmsg ) (*dbgmsg << "requested : read = " << points_read << " / " << data.max_points << " points").flush();

	if( !mute ) points_read = data.read_raw( io, points_read );
	else data.points = points_read;
    total_processed_points += points_read;

	if( dbgmsg ) (*dbgmsg << "requested : read(actual) " << points_read << " points" ).flush();

    if( next ) next->received( this, data );

    if( total_processed_points == event_end_point ){

        ++event_no;
        exec_event();
    }
    else if( data.points == 0 ) over = true;
}


// Override
void Source::received( Module* sender, DoubleBuffer& data )
{}


// Override
const std::string Source::get_result() const
{
    if( next ) return next->get_result();

	return std::string();
}
