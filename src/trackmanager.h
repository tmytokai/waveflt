// track manager

#ifndef _TRACKMANAGER_H
#define _TRACKMANAGER_H

#include <vector>
#include "track.h"

class TrackManager
{
  private:
    bool eot; // end_of_tracks

  public:
    std::vector<Track> tracks;

    TrackManager();

    Track& get_track( const int track_no ){ return tracks[track_no]; }

    const unsigned int size() const { return tracks.size(); }
    const bool end_of_tracks() const { return eot; }

    void start();
    void read();
};

#endif
