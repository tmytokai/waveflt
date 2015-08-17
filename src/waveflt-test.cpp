#include <iostream>
#include <assert.h>

#include "source.h"
#include "output.h"
#include "resampler.h"
#include "dbgmsgbase.h"

int main( int argc, char* argv[] )
{
    assert( argc == 3 );
    std::string in_file( argv[1] );
    std::string out_file( argv[2] );

    try{

//		InitDbgMsg( new DbgOutStderr() );
		InitDbgMsg( new DbgOutLog( "log.txt" ) );

        Source* src = new Source( in_file );
        src->debugmode();

        Resampler* rsmp = new Resampler( 48000 );
//        rsmp->debugmode();
        src->connect( rsmp );

        Output* output = new Output( out_file );
//        output->debugmode();
        rsmp->connect( output );

        src->init();

        std::vector<EventData> event;
        EventData eventdata;

        eventdata.message = "delete";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "read";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "mute";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "read";
        eventdata.points = -1;
        event.push_back( eventdata );

        eventdata.message = "end";
        event.push_back( eventdata );

        src->set_event(event);

        std::cerr << src->get_config() << std::endl;

        src->start();
        do{ output->process(); } while( !(output->is_over()) );

		std::cerr << "\nResult:\n" << src->get_result() << std::endl;

        delete src;
		ClearDbgMsg();
    }
    catch( const std::string& err ){

        std::cerr << err << std::endl;
    }

    return 0;
}
