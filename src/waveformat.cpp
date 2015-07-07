#include "waveformat.h"

#include <stdio.h>
#include <string.h>
#include <io.h> // _setmode
#include <fcntl.h> // _O_BINARY
#include <assert.h>

enum
{
	ID_err = 0,
	ID_riff,
	ID_wave,
	ID_fmt,
	ID_data,
	ID_fact,
	ID_bext,
	ID_cue,
	ID_list,
	ID_plst,
	ID_junk,
	ID_wflt, // extension
	ID_unknown
};

void WaveFormat::clear()
{
	memset(&raw,0,sizeof(WAVEFORMAT_RAW));
}


void WaveFormat::set( const unsigned short _tag,	
					 const unsigned short _channels, 
					 const unsigned int _rate, 
					 const unsigned short _bits )
{
	raw.tag = _tag;
	raw.channels = _channels;
	raw.rate = _rate;
	raw.bits = _bits;
	raw.block = (unsigned short)(raw.channels*raw.bits/8);
	raw.avgbyte = (unsigned int)(raw.rate*raw.block);
	raw.size = 0;

	is_valid();
}


void WaveFormat::read( FILE *fp )
{
	datasize = 0;
	offset = 0;

	char chunk[5] = {0};
	unsigned int chunksize;

	if( GetChunkID( fp, chunk, chunksize ) != ID_riff ) throw( std::string( "This is not wave file." ) );
	offset += 8;

	while(1){

		unsigned int byte;
		const int id = GetChunkID( fp, chunk, chunksize );

		if( id == ID_err ) throw( std::string( "Invalid wave header." ) );
		else if( id == ID_unknown ) throw( std::string( "Unknown chunk '%s'." ) );
		else if( id == ID_wave ){
			offset += 4;
			continue;
		}
		else if( id == ID_fmt ){
			assert( chunksize <= sizeof(WAVEFORMAT_RAW) );
			offset += 8;
			byte = fread(&raw,1,chunksize,fp);
			if(byte != chunksize) throw( std::string( "Invalid fmt chunk." ) );
			offset += byte;  
			is_valid();
		}
		else if( id == ID_wflt ){
			offset += 8;
			byte = fread( &datasize, 1, sizeof(unsigned long long), fp);
			if(byte != sizeof(unsigned long long)) throw( std::string(  "Invalid wflt chunk." ) );
			offset += byte;  
		}
		else if( id == ID_data ){
			offset += 8;
			if( datasize == 0) datasize = chunksize;
			break;
		}
		else{
			offset += (8 + chunksize);
#ifndef WIN32
			off_t pos64 = offset;
			fseeko(fp, pos64, SEEK_SET);
#else
			__int64 pos64 = offset;
			_fseeki64( fp , pos64, SEEK_SET);
#endif
		}
	}
}


void WaveFormat::read( const std::string& filename )
{
	FILE* fp = NULL;

	// stdin
	if( filename == "stdin" ){
		fp = stdin;
#ifdef WIN32
		_setmode(_fileno(stdin),_O_BINARY); // set binary mode
#endif
	}
	// file
	else{
		fp = fopen(filename.c_str(),"rb");
		if(fp == NULL) throw( std::string( "Cannot open " + filename ) );
	}

	read( fp );

	if( fp && fp != stdin ) fclose(fp);
}


void WaveFormat::write( FILE* fp, const unsigned long long datasize, const bool extchunk )
{
	unsigned int tmp;

	if( fp == NULL) return;

	unsigned int headsize = 44;
	if( extchunk ) headsize += (4 + 4 + 8 + 4);

	fseek( fp , 0, SEEK_SET);

	// RIFF (8)
	fwrite( "RIFF", 1, 4, fp );
	if( datasize >= 0xFFFFFFFF-headsize+8) tmp = 0xFFFFFFFF; // = 4G
	else tmp = ((unsigned int)datasize + headsize) - 8;
	fwrite( &tmp, 1, sizeof(unsigned int), fp);

	// WAVE (4)
	fwrite( "WAVE", 1, 4, fp );

	// format chunk (24)
	fwrite( "fmt ",1 , 4, fp );
	tmp = 16;
	fwrite( &tmp, 1, sizeof(unsigned int), fp);
	fwrite( &raw, 1, 16, fp );

	// waveflt chunk (extra chunk)
	if(extchunk){
		fwrite( "wflt",1, 4, fp );
		tmp = sizeof(unsigned long long) + sizeof(unsigned int);
		fwrite( &tmp, 1, sizeof(unsigned int), fp);
		fwrite( &datasize, 1,  sizeof(unsigned long long), fp );
		tmp = 0;
		fwrite( &tmp, 1, sizeof(unsigned int), fp);
	}

	// data chunk (8  + datasize)
	fwrite( "data", 1, 4, fp );
	if( datasize >= 0xFFFFFFFF-headsize+8) tmp = 0xFFFFFFFF-headsize+8; // = 4G
	else tmp = (unsigned int)datasize;
	fwrite( &tmp, 1, sizeof(unsigned int), fp);
}


const double WaveFormat::GetMaxWaveLevel()
{
	double ret = 0;

	switch(raw.bits){

				case 8: 
					ret = 128; 
					break;
				case 16:
					ret = 32768; 
					break;
				case 24: 
					ret = 8388608; 
					break;
				case 64: 
					ret = 1; 
					break;
				case 32: 
					if( raw.tag == WAVE_FORMAT_PCM) ret = 2147483648;
					else ret = 1;
					break;
	}

	return ret;
}


void WaveFormat::is_valid()
{
	if( raw.tag != WAVE_FORMAT_IEEE_FLOAT && raw.tag != WAVE_FORMAT_PCM ) throw( std::string( "Not PCM" ) );
	if( raw.channels != 1 && raw.channels != 2) throw( std::string( raw.channels + "-channels is not supported." ) );
	if( raw.bits != 8 && raw.bits != 16 && raw.bits != 24 && raw.bits != 32 && raw.bits != 64) throw( std::string( raw.bits + "-bits is not supported." ) );
	if( raw.avgbyte != raw.channels*raw.rate*raw.bits/8) throw( std::string( "Invalid wave" ) );
}


const int WaveFormat::GetChunkID( FILE* fp,  char* chunk,  unsigned int& chunksize )
{
	assert( fp );
	assert( chunk );

	unsigned int byte;
	chunksize = 0;

	byte = fread( chunk, 1, 4, fp);
	if(byte != 4) return ID_err;

	if( strncmp(chunk,"WAVE",4) ==0 ) return ID_wave;

	byte = fread(&chunksize,1,sizeof(unsigned int),fp);
	if(byte != sizeof(unsigned int)) return ID_err;

	if( strncmp(chunk,"RIFF",4) ==0 ) return ID_riff;
	if( strncmp(chunk,"fmt ",4) ==0 ) return ID_fmt;
	if( strncmp(chunk,"data",4) ==0 ) return ID_data;
	if( strncmp(chunk,"fact",4) ==0 ) return ID_fact;
	if( strncmp(chunk,"bext",4) ==0 ) return ID_bext;
	if( strncmp(chunk,"cue ",4) ==0 ) return ID_cue;
	if( strncmp(chunk,"LIST",4) ==0 ) return ID_list;
	if( strncmp(chunk,"plst",4) ==0 ) return ID_plst;
	if( strncmp(chunk,"JUNK",4) ==0 ) return ID_junk;
	if( strncmp(chunk,"wflt",4) ==0 ) return ID_wflt;

	return ID_unknown;
}
