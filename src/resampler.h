// resampler class

#ifndef _RESAMPLER_H
#define _RESAMPLER_H

#include "module.h"
#include "waveformat.h"
#include "doublebuffer.h"


class Resampler : public Module
{
  private:

    DoubleBuffer data;

    unsigned int rate;
    unsigned int up;
    unsigned int down;
    unsigned int convolution_size;
    unsigned int phase;

    unsigned int points_required;

    // shift data as much as group_ delay/down
    unsigned int group_delay;
    unsigned int shift_before;
    unsigned int shift_after;

    DoubleBuffer pastdata;
    unsigned int pastdata_offset;

    double **fir;  // polyphase decomposition of FIR coefficients

  public:

    Resampler( const unsigned int _rate );
    virtual ~Resampler();

    // Override
    virtual void reset_all();
    virtual void clear_all_buffer();
    virtual void init();
    virtual const std::string get_config() const;
    virtual void start();
    virtual void exec_event();
    virtual void requested( const unsigned int points_required );
    virtual void received( Module* sender, DoubleBuffer& data, const bool fin );
    virtual void show_result() const;

  private:

    const unsigned int calc_points_required( const unsigned int points );
};

#endif
