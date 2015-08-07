// track manager

#ifndef _TRACKMANAGER_H
#define _TRACKMANAGER_H

#include <string>
#include <vector>

class WaveFormat;
class DoubleBuffer;
class Filter;

class TrackManager
{
  private:

    bool eot; // end_of_tracks
    std::vector<Filter*> tracks;

  public:

    TrackManager();

    Filter* create_track( const int track_no, const std::string& filename, DoubleBuffer& data );
    Filter* get_track( const int track_no ){ return tracks[track_no]; }

    const unsigned int size() const { return tracks.size(); }
    const bool end_of_tracks() const { return eot; }

    void init();
    void show_config();
    void start();
    void process();
    void show_result();
    void free();
};

#endif
