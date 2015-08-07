// abstract class of wave source

#ifndef _WAVESOURCE_H
#define _WAVESOURCE_H

#include <string>

class WaveSource
{
  protected:
    bool dbg;
    std::string name;

  public:
    WaveSource( const std::string& _name): dbg(false), name(_name){}
    virtual ~WaveSource(){}

    const std::string& get_name() const { return name; }

    virtual void open() = 0;
    virtual void seek( const unsigned long long offset_byte ) = 0;
    virtual const unsigned int seek_back( const unsigned int back_byte ) = 0;
    virtual const unsigned int read( unsigned char* data, const unsigned int byte ) = 0;
};

#endif
