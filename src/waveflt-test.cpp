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

        EventData eventdata;

        eventdata.message = "delete";
        eventdata.seconds = 1.0;
        src->add_event( eventdata );

        eventdata.message = "read";
        eventdata.seconds = 1.0;
        src->add_event( eventdata );

        eventdata.message = "mute";
        eventdata.seconds = 1.0;
        src->add_event( eventdata );

        eventdata.message = "read";
        eventdata.seconds = -1;
        src->add_event( eventdata );

        eventdata.message = "end";
        eventdata.seconds = 0;
        src->add_event( eventdata );

//        Resampler* rsmp = new Resampler( host, 48000 );
//        rsmp->debugmode();
//        src->connect( rsmp );

        Output* output = new Output( host, out_file );
//        output->debugmode();
        src->connect( output );

        host->init();

        std::cerr << "\n[CONFIG]\n\n" << host->get_config() << std::endl << std::endl;

        host->start();
        do{
            host->process();
            std::cerr << "\015" << host->get_status();
        } while( !(host->is_over()) );
        std::cerr << "\n\n[RESULT]\n\n" << host->get_result() << std::endl;
        delete host;

        ResetDbgMsg();
    }
    catch( const std::string& err ){

        std::cerr << err << std::endl;
    }

    return 0;
}
