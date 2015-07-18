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
    unsigned long long size; //  data size of block (byte)
};

class Track
{
  private:
    bool dbg;
    bool verbose;

    unsigned int track_no;
    WaveFormat format;

    std::vector<Filter*> filters;
    std::vector<BlockData> blockdata;
    std::string filename;

    unsigned int block_no;
    unsigned int read_size; // size of read data in buffer (byte)
    unsigned int remain_size; // size of unused data in buffer (byte)
    unsigned long long block_total_read_size;  // total read size of current block (byte)

  public:
    FILE* fp;
    unsigned char* buffer;
    unsigned int buffer_size;

    std::vector<double*> data;
    unsigned int points;

    Track( const int _track_no, const WaveFormat& _format );
    ~Track();

    void reset();

    const bool end_of_track() const { return ( block_no == blockdata.size() ); }

    void set_verbose( const bool _verbose ){ verbose = _verbose; }
    void set_filters( std::vector<Filter*> _filters ){ filters = _filters; }
    void set_blockdata( const std::vector<BlockData>& _blockdata ){ blockdata = _blockdata; }
    void set_filename( const std::string& _filename ){ filename = _filename; }

    const unsigned int get_points() const { return points; }
    const WaveFormat& get_format() const { return format; }

    void start(){ reset();  begin_block(); }
    unsigned int read();
    void cut( const unsigned int offset_point );

  private:
    void begin_block();
    void convert_buffer();
};

#endif
