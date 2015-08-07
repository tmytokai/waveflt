#if 0

// track manager

#include "trackmanager.h"
#include "track.h"

TrackManager::TrackManager()
    :eot(false)
{}


Filter* TrackManager::create_track( const int track_no, const std::string& filename, DoubleBuffer& data )
{
    Filter* track = new Track( track_no, filename, data );
    track->debugmode();
    tracks.push_back( track );
    return track;
}


void TrackManager::init()
{
    std::vector<Filter*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->init();
    }
}


void TrackManager::show_config()
{
    std::vector<Filter*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->show_config();
    }
}


void TrackManager::start()
{
    std::vector<Filter*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->start_track();
        (*it)->begin_block();
    }
}


void TrackManager::process()
{
    eot = true;
    std::vector<Filter*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->process( 0 );
        if( ! (*it)->is_over() ) eot = false;
    }
}


void TrackManager::show_result()
{
    std::vector<Filter*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->show_result();
    }
}


void TrackManager::free()
{
    while( tracks.size() ){
        delete tracks.back();
        tracks.pop_back();
    }
}

#endif