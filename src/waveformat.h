// WAVE format class

#ifndef _WAVEFORMAT_H
#define _WAVEFORMAT_H

#include <string>

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 0x0001
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

class IO;

typedef struct
{
    unsigned short  tag;         // type of format
    unsigned short  channels;    // channels
    unsigned int    rate;        // sampling rate
    unsigned int    avgbyte;     // = rate * block
    unsigned short  block;       // = channels * bits / 8
    unsigned short  bits;        // number of bits
    unsigned short  size;        // extra size
} WAVEFORMAT_RAW;


class WaveFormat
{
  private:

    WAVEFORMAT_RAW raw;
    unsigned int offset;       // offset to the data chunk (byte)
    unsigned int data_size;    // size of the data chunk   (byte)
    unsigned int data_points;  // size of the data chunk   (points)

  public:

    const unsigned short tag() const{ return raw.tag;}
    const unsigned short channels() const{ return raw.channels;}
    const unsigned int   rate() const{ return raw.rate;}
    const unsigned int   avgbyte()const{ return raw.avgbyte;}
    const unsigned short block() const{ return raw.block;}
    const unsigned short bits() const{ return raw.bits; }
    const unsigned int get_offset() const{ return offset;}
    const unsigned int get_data_size() const{ return data_size; }
    const unsigned int get_data_points() const{ return data_points; }
    const WAVEFORMAT_RAW& get_raw() const{ return raw;  }

    void clear();

    void set( const unsigned short _tag,	
              const unsigned short _channels, 
              const unsigned int _rate, 
              const unsigned short _bits );

    void read( IO* io );
    void write( IO* io, const unsigned long long points );

    const double get_max_level();
    void is_valid();

  private:

    const int get_chunk_id( IO* io,  char* chunk,  unsigned int& chunksize );

};

#endif
