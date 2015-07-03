// standard WAVE format struct

#ifndef _WAVEFORMAT_H
#define _WAVEFORMAT_H

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 0x0001
#endif

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

typedef struct
{
    unsigned short  tag;      // type of format
    unsigned short  channels;    // channels
    unsigned int    rate;        // sampling rate
    unsigned int    avgbyte;     // = rate * block
    unsigned short  block;       // = channels * bits / 8
    unsigned short  bits;        // number of bits
} WAVFMT;

#endif
