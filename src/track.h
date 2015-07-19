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
    unsigned long long offset; // offset to block (byte)
    unsigned long long points; // points of block
};

class Track
{
  private:
    bool dbg;
    bool verbose;

    unsigned int track_no;
    WaveFormat input_format;

    std::vector<Filter*> filters;
    std::vector<BlockData> blockdata;
    std::string filename;

    unsigned int block_no;

    bool split;
    unsigned int split_pos;
    unsigned int split_points;

  public:
    FILE* fp;
    unsigned char* raw;   // buffer for raw data
    unsigned int raw_points;   // current points of raw buffer
    unsigned int raw_max_points;   // max_points of raw buffer
    unsigned long long block_total_raw_points;  // total raw points of current block

    std::vector<double*> data;
    unsigned int data_points;  // current points of data
    unsigned int data_max_points;  // max_points of data

    Track( const int _track_no, const WaveFormat& _input_format );
    ~Track();

    void set_verbose( const bool _verbose ){ verbose = _verbose; }
    void set_filters( std::vector<Filter*> _filters ){ filters = _filters; }
    void set_blockdata( const std::vector<BlockData>& _blockdata ){ blockdata = _blockdata; }
    void set_filename( const std::string& _filename ){ filename = _filename; }

    const unsigned int get_data_points() const { return data_points; }
    const WaveFormat& get_input_format() const { return input_format; }

    const bool end_of_track() const { return ( block_no == blockdata.size() ); }

    void start();
    void read();
    void exec_split( const unsigned int pos );

  private:
    void reset();
    void begin_new_block();
    void convert_raw_to_data();
};

#endif
