#include <iostream>
#include <assert.h>

#include "source.h"
#include "output.h"
#include "resampler.h"
#include "dbgmsg.h"

int main( int argc, char* argv[] )
{
    assert( argc == 3 );
    std::string in_file( argv[1] );
    std::string out_file( argv[2] );

    try{

	InitDbgMsg( new DbgOutStderr() );
//	InitDbgMsg( new DbgOutLog( "log.txt" ) );

        Host* host = new Host();

        Source* src = new Source( host, in_file );
//        src->debugmode();

        Resampler* rsmp = new Resampler( host, 48000 );
//        rsmp->debugmode();
        src->connect( rsmp );

        Output* output = new Output( host, out_file );
//        output->debugmode();
        rsmp->connect( output );

        host->init();

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

        std::cerr << "\nConfig:\n" << host->get_config() << std::endl;

        host->start();
        do{ output->process(); } while( !(output->is_over()) );

        std::cerr << "\nResult:\n" << host->get_result() << std::endl;

        delete host;

        ResetDbgMsg();
    }
    catch( const std::string& err ){

        std::cerr << err << std::endl;
    }

    return 0;
}
