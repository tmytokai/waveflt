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
    unsigned int datasize;  // data size (byte)
    unsigned int offset;    // offset to data chunk (byte)

  public:

    const unsigned short tag() const{ return raw.tag;}
    const unsigned short channels() const{ return raw.channels;}
    const unsigned int   rate() const{ return raw.rate;}
    const unsigned int   avgbyte()const{ return raw.avgbyte;}
    const unsigned short block() const{ return raw.block;}
    const unsigned short bits() const{ return raw.bits; }
    const unsigned int get_datasize() const{ return datasize; }
    const unsigned int get_offset() const{ return offset;}
    const WAVEFORMAT_RAW& get_raw() const{ return raw;  }

    void clear();

    void set( const unsigned short _tag,	
              const unsigned short _channels, 
              const unsigned int _rate, 
              const unsigned short _bits );

    void read( FILE *fp );
    void read( const std::string& filename );
    void write( FILE* fp, const unsigned long long datasize );

    const double get_max_level();
    void is_valid();

  private:

    const int get_chunk_id( FILE* fp,  char* chunk,  unsigned int& chunksize );

};

#endif
