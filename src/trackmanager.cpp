// track manager

#include "trackmanager.h"


TrackManager::TrackManager()
    :eot(false)
{}


void TrackManager::init()
{
    std::vector<Track*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->init();
    }
}


void TrackManager::show_config()
{
    std::vector<Track*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->show_config();
    }
}


void TrackManager::start()
{
    std::vector<Track*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ) (*it)->start();
}


void TrackManager::process()
{
    eot = true;
    std::vector<Track*>::iterator it = tracks.begin();
    for( ; it != tracks.end(); ++it ){
        (*it)->process();
        if( ! (*it)->end_of_track() ) eot = false;
    }
}


void TrackManager::show_result()
{
    std::vector<Track*>::iterator it = tracks.begin();
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
