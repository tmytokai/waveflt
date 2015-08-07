// resampler class

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <assert.h>

#include "resampler.h"


double get_kaiser_alpha( double db ){

    double alpha;

    if( db < -50 ) alpha = 0.1102*(-db-8.7);
    else if( db >= -21 ) alpha = 0;
    else alpha = 0.5842 * pow((double)(-db-21),0.4) + 0.07880*(-db-21);

    return alpha;
}


#define MAXSUM 20 
double I0(double x){
	
	int i;
	double k,foo,out;
	
	out = 0;
	k = 1;
	for(i=1;i<MAXSUM;i++){
		k *= i;
		foo = pow(x/2,i)/k;
		out += foo*foo;
	}
	
	return(out+1);
}
#undef MAXSUM


void calc_fir_coefficient( double * h, unsigned int lng, double rate, double db, double cutoff )
{
    const unsigned int center = (lng-1)/2;
    const double fc = cutoff/(rate/2.0); 
    const double alpha = get_kaiser_alpha(db);

    h[center] = fc;
    for( unsigned int i=1; i <= center; ++i ) h[center+i] = sin(M_PI*i*fc)/(M_PI*i);

    for( unsigned int i = 1; i <=center; ++i) {

        const double foo = h[center+i] * I0(alpha*sqrt(1-(2*i/(double)(lng-1)) * (2*i/(double)(lng-1)) ))/I0(alpha);
        h[center+i] = h[center-i] = foo;
    }
}

/////////////////////////////


Resampler::Resampler( const unsigned int _rate )
    : Module( "Resampler" ), rate(_rate), fir(NULL)
{
    reset_all();
}


Resampler::~Resampler()
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::~Resampler\n");

    if( next ) delete next;
    next = NULL;
    reset_all();
}


// Override
void Resampler::reset_all()
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::reset_all\n");

    over = false;

    data.reset_all();

    up = 0;
    down = 0;
    convolution_size = 0;
    phase = 0;

    points_required = 0;

    group_delay = 0;
    shift_before = 0;
    shift_after = 0;

    pastdata.reset_all();
    pastdata_offset = 0;

    if( fir ){
        free( fir[0] );
        free( fir );
    }
    fir = NULL;
}


// Override
void Resampler::clear_all_buffer()
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::clear_all_buffer\n" );

    // defined in Module class
    over = false;

    data.clear_all_buffer();
    pastdata.clear_all_buffer();
    pastdata_offset = 0;
}


// Override
void Resampler::init()
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::init\n");

    assert( prev );
    input_format = prev->get_output_format();
    output_format.set( input_format.tag(), input_format.channels(), rate, input_format.bits() );

    if( next ) next->init();

    unsigned int length;  // length of FIR filter

    // oversampling 22050 -> 48000
    // 22050 * 320 = 48000 * 147 = 7056000
    length = 32767;
    up = 320;
    down = 147;
    convolution_size = length/up;
    phase = 0;
    
    group_delay = (length-1)/2;
    shift_before = shift_after = group_delay/down;

    if( dbg ) data.debugmode();
    const unsigned int max_points = (unsigned int)( 0.5 * 2 * output_format.rate() );
    data.init( output_format, max_points, true, false );

    if( dbg ) pastdata.debugmode();
    pastdata.init(  output_format, ( convolution_size-1 ) + data.max_points, true, false );
    pastdata_offset = 0;

    clear_all_buffer();

    // polyphase decomposition of FIR coefficients
    double *h = (double*)malloc(sizeof(double)*length);

    fir = (double**)malloc(sizeof(double)*up);
    fir[0] = (double*)malloc(sizeof(double)*length);
    for(unsigned int i=1;i<up;i++) fir[i] = fir[i-1] + convolution_size;

    calc_fir_coefficient( h, length, 22050*up, -150, 22050/2*0.95 );
    for( unsigned int i=0;i<up;i++){
        for( unsigned int i2=0;i2<convolution_size;i2++){
            fir[i][convolution_size-1 - i2] = h[i + i2*up] * up;
            assert( i+i2*up < length );
        }
    }

    free(h);

    if( dbg ){
        fprintf( stderr, "\n[debug] Resampler::init : \n");
        fprintf( stderr, "up = %d, down = %d, length = %d, convolution_size = %d, group_delay = %d, shift_* = %d\n", up, down, length, convolution_size, group_delay, shift_before );
        fprintf( stderr, "pastdata_max_points = %d\n", pastdata.max_points - ( convolution_size-1 ) );
    }
}


// Override
void Resampler::show_config() const
{
    fprintf(stderr,"%s(ID_%d): %d (Hz) -> %d (Hz)\n", get_name().c_str(), get_id(), input_format.rate(), output_format.rate() );

    if( next ){
        fprintf(stderr,"=> " );
        next->show_config();
    }
}


// Override
void Resampler::start()
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::start\n");
    if( next ) next->start();

    over = false;
    shift_before = shift_after = group_delay/down;
    clear_all_buffer();
}


// Override
void Resampler::exec_event()
{}


// Override
void Resampler::requested( const unsigned int _points_required )
{
    points_required = _points_required;
    if( points_required > data.max_points ) points_required = data.max_points;

    if( dbg ) fprintf( stderr, "\n[debug] Resampler::requested : required = %d\n", points_required );

    data.clear_all_buffer();

    const unsigned int new_points_required = calc_points_required( points_required );
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::requested : new required = %d\n", new_points_required );
    assert( prev ); prev->requested( new_points_required );
}


// Override
void Resampler::received( Module* sender, DoubleBuffer& _data, const bool fin )
{
    if( is_over() ){
        if( next ) return next->received( this, data, is_over() );
        return;
    }

    unsigned int input_points = _data.points;

    if( dbg ){
        fprintf( stderr, "\n[debug] Resampler::received : points = %d, ", input_points );
        fprintf( stderr," pastdata_offset = %d, shift_before = %d, shift_after = %d\n", pastdata_offset, shift_before, shift_after );
    }

    assert( prev == sender );

    bool exec_shift_after = false;
    if( !input_points && fin ){

        if( shift_after == 0 ){
            over = true;
            if( next ) return next->received( this, data, is_over() );
            return;
        }

        exec_shift_after = true;
        input_points = calc_points_required( shift_after );
        for( unsigned int i = 0; i < output_format.channels(); ++i )
            memset( pastdata[i] + (convolution_size-1) + pastdata_offset, 0, sizeof(double)*input_points);
    }
    else{
        for( unsigned int i = 0; i < output_format.channels(); ++i )
            memcpy( pastdata[i] + (convolution_size-1) + pastdata_offset, _data[i], sizeof(double)*input_points );
    }

    pastdata.points = pastdata_offset + input_points;
    assert( ( convolution_size-1 ) + pastdata.points <= pastdata.max_points );

    unsigned int pastdata_pos = 0;
    while(1){

        if( ! shift_before ){

            // convolution
            for( unsigned int i = 0; i < output_format.channels(); ++i ){

                double tmp = 0;
                double *pastdata_tmp = pastdata[i];
                for( unsigned int i2 = 0; i2 < convolution_size; ++i2 ) tmp += fir[phase][i2] * pastdata_tmp[ pastdata_pos + i2 ];

                data[i][data.points] = tmp;
            }

            ++data.points;
            assert( data.points <= data.max_points );
        }
        else --shift_before;

        phase += down;
        while( phase >= up){

            phase -= up;
            ++pastdata_pos;
            assert( ( convolution_size-1 ) + pastdata_pos <= pastdata.max_points );
        }
        if( data.points == points_required ) break;
        if( pastdata_pos == pastdata.points ) break;
        if( exec_shift_after && data.points == shift_after ) break;
    }

    pastdata_offset = pastdata.points - pastdata_pos;

    for( unsigned int i = 0; i < output_format.channels(); ++i ){
        memmove( pastdata[i], pastdata[i] + (convolution_size-1) + pastdata_pos - (convolution_size-1), sizeof(double)*(convolution_size-1+pastdata_offset) );
    }

    if( exec_shift_after ) shift_after -= data.points;

    if( dbg ){
        fprintf( stderr, "\n[debug] Resampler::received : \n" );
        fprintf( stderr, "phase = %d, pastdata_offset = %d, pastdata_pos = %d, pastdata_points = %d, pastdata_max_points = %d\n", phase, pastdata_offset, pastdata_pos, pastdata.points, pastdata.max_points - ( convolution_size-1 ) );
        fprintf( stderr, "data.points = %d, data_max_points = %d\n\n", data.points, data.max_points );
    }

    if( next ) return next->received( this, data, false );
}


// Override
void Resampler::show_result() const
{
    if( dbg ) fprintf( stderr, "\n[debug] Resampler::show_result\n");
    if( next ) next->show_result();
}


// private
const unsigned int Resampler::calc_points_required( const unsigned int points_required )
{
    if( ! points_required ) return 0;

    unsigned int data_points = 0;
    unsigned int ret = 1;
    unsigned int phase_tmp = phase;
    unsigned int shift_before_tmp = shift_before;
    while(1){

        if( ! shift_before_tmp ) ++data_points;
        else --shift_before_tmp;

        if( data_points == points_required ) break;

        phase_tmp += down;
        while( phase_tmp >= up){
            phase_tmp -= up;
            ++ret;
        }
    }

    assert( ret - pastdata_offset >= 0 );
    return ret - pastdata_offset;
}
