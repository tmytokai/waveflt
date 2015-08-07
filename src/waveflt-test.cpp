#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "source.h"
#include "output.h"
#include "resampler.h"

int main( int argc, char* argv[])
{
    assert( argc == 3 );
    std::string in_file( argv[1] );
    std::string out_file( argv[2] );
    printf("input = %s, output = %s\n\n", in_file.c_str(), out_file.c_str() );

    try{

        Source* src = new Source( in_file );
//        src->debugmode();

        Resampler* rsmp = new Resampler( 48000 );
//        rsmp->debugmode();
//        src->connect( rsmp );

        Output* output = new Output( out_file );
//        output->debugmode();
        src->connect( output );

        src->init();

        std::vector<EventData> event;
        EventData eventdata;

        eventdata.message = "delete";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "read";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "delete";
        eventdata.points = (unsigned int)(src->get_input_format().rate()*1);
        event.push_back( eventdata );

        eventdata.message = "read";
        eventdata.points = 0;
        event.push_back( eventdata );

        eventdata.message = "end";
        event.push_back( eventdata );

        src->set_event(event);

        src->show_config();

        src->start();
        do{ output->process(); }while( !(output->is_over()) );

        src->show_result();

        delete src;

    }catch( const std::string& err ){

        printf("%s\n", err.c_str() );
        exit(1);
    }

    return 0;
}
