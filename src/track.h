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

    FILE* fp;
    std::string filename;
    std::vector<BlockData> blockdata;
    std::vector<Filter*> filters;

    WaveFormat raw_format;
    unsigned char* raw;   // buffer for raw data
    unsigned int raw_points;  // current points of raw data
    unsigned int raw_max_points;  // max points of raw data

    WaveFormat data_format;
    std::vector<double*> data;   // buffer for internal data (64bit double)
    unsigned int data_points;  // current points of data
    unsigned int data_max_points;  // max points of data
    unsigned long long block_total_data_points;  // total data points of current block

  public:

    Track( const int _track_no, const std::string& _filename, const WaveFormat& _raw_format, const WaveFormat& _data_format );
    ~Track();

    const bool end_of_track() const { return ( block_no == blockdata.size() ); }
    const WaveFormat& get_data_format() const { return data_format; }
    double* get_data( const int ch ){ return data[ch]; }
    const unsigned int get_data_points() const { return data_points; }
    const unsigned int get_data_max_points() const { return data_max_points; }

    void set_verbose( const bool _verbose ){ verbose = _verbose; }
    void set_blockdata( const std::vector<BlockData>& _blockdata ){ blockdata = _blockdata; }

    void init();
    void show_config();
    void start();
    void process();
    void show_result();

  private:
    void reset();
    void begin_new_block();
    void convert_raw_to_data();
};

#endif
