// standard WAVE format struct

#ifndef _WAVEFORMAT_H
#define _WAVEFORMAT_H


typedef struct
{
    unsigned short  format;      // type of format
    unsigned short  channels;    // channels
    unsigned int    rate;        // sampling rate
    unsigned int    avgbyte;     // = rate * block
    unsigned short  block;       // = channels * bits / 8
    unsigned short  bits;        // number of bits
} WAVFMT;

#endif
