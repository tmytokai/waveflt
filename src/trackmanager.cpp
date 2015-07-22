// track manager

#include "trackmanager.h"


TrackManager::TrackManager()
    :eot(false)
{}


void TrackManager::start()
{
    std::vector<Track>::iterator it_track = tracks.begin();
    for( ; it_track != tracks.end(); ++it_track ) (*it_track).start();
}


void TrackManager::read()
{
    eot = true;
    std::vector<Track>::iterator it_track = tracks.begin();
    for( ; it_track != tracks.end(); ++it_track ){
        (*it_track).read();
        if( ! (*it_track).end_of_track() ) eot = false;
    }
}
