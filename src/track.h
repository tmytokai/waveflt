// track class

#ifndef _TRACK_H
#define _TRACK_H

#include <vector>
#include "waveformat.h"

class Filter;

enum
{
    TYPE_STORAGE = 0,
    TYPE_STDIN,
    TYPE_NULL
};

struct BlockData
{
    int input_type; // type of input
    unsigned long long raw_offset; // raw offset points to block
    unsigned long long raw_points; // raw points of block
    unsigned long long data_points; // data points of block
};

class Track
{
  private:
    bool dbg;
    bool verbose;

    unsigned int track_no;
    unsigned int block_no;

    std::vector<Filter*> filters;
    std::vector<BlockData> blockdata;
    std::string filename;

  public:

    FILE* fp;

    WaveFormat raw_format;
    unsigned char* raw;   // buffer for raw data
    unsigned int raw_points;  // current points of raw data
    unsigned int raw_max_points;  // max points of raw data

    WaveFormat data_format;
    std::vector<double*> data;   // buffer for internal data (64bit double)
    unsigned int data_points;  // current points of data
    unsigned int data_max_points;  // max points of data
    unsigned long long block_total_data_points;  // total data points of current block


    Track( const int _track_no, const WaveFormat& _raw_format, const WaveFormat& _data_format );
    ~Track();

    const bool end_of_track() const { return ( block_no == blockdata.size() ); }

    void set_verbose( const bool _verbose ){ verbose = _verbose; }
    void set_filters( std::vector<Filter*> _filters ){ filters = _filters; }
    void set_blockdata( const std::vector<BlockData>& _blockdata ){ blockdata = _blockdata; }
    void set_filename( const std::string& _filename ){ filename = _filename; }

    const WaveFormat& get_raw_format() const { return raw_format; }

    const WaveFormat& get_data_format() const { return data_format; }
    const unsigned int get_data_points() const { return data_points; }

    void start();
    void read();

  private:
    void begin_new_block();
    void convert_raw_to_data();
};

#endif
