#include<chunky.h>

float(* assign_oscillator_function(char *wfunc))(struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos)
{
    if (     ! strcmp( wfunc, "sin" ) )
        return &sin_wave;     
    else if( ! strcmp( wfunc, "sqx" ) )
        return &sqx_wave;
    else if( ! strcmp( wfunc, "sqr" ) )
        return &sqr_wave;
    else if( ! strcmp( wfunc, "sup" ) )
        return &sup_wave;
    else if( ! strcmp( wfunc, "sut" ) )
        return &sut_wave;
    else if( ! strcmp( wfunc, "sdn" ) )
        return &sdn_wave;
    else if( ! strcmp( wfunc, "sn3" ) )
        return &sn3_wave;
    else if( ! strcmp( wfunc, "sn5" ) )
        return &sn5_wave;
    else if( ! strcmp( wfunc, "tri" ) )
        return &tri_wave;
    else if( ! strcmp( wfunc, "trx" ) )
        return &trx_wave;
    else if( ! strcmp( wfunc, "wht" ) )
        return &wht_wave;
    else if( ! strcmp( wfunc, "rsq" ) )
        return &rsq_wave;
    else
        return NULL;
}

/* =========The oscillators======== */
/* N.B. they must all have the same args 
regardless of whether they use them or not
*/

/* --- sine wave --- */
float sin_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    sample_value = sin( node->f + node->phase*2*M_PI );
    
    return( sample_value );
}

/* --- sine cubed wave --- */
float sn3_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float sample_value;
    sample_value = sin( node->f + node->phase*2*M_PI );
    return( sample_value * sample_value * sample_value );
}

/* --- sine quinted wave --- */
float sn5_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float sample_value;
    sample_value = sin( node->f + node->phase*2*M_PI );
    return( sample_value * sample_value * sample_value * sample_value * sample_value );
}

/* --- a differentiable (smooth) version of the square wave --- */
float sqx_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    float    delta=0.005;
    sample_value = 2./M_PI*atan(sin( node->f + node->phase*2*M_PI )/delta);
    return( sample_value );
}


/* --- a triangular wave --- */
float tri_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    sample_value = 2./M_PI*asin(sin( node->f + node->phase*2*M_PI ));
    return( sample_value );
}


/* --- a smoothed triangular wave --- */
float trx_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    sample_value = 2./M_PI*asin(sin( node->f + node->phase*2*M_PI )*0.975);
    return( sample_value );
}



/* --- square wave --- */
float sqr_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float sample_value;
    sample_value = sin( node->f + node->phase*2*M_PI );
    if( sample_value > 0 ) sample_value = 1.0;
    else sample_value = -1.0 ;
    return( sample_value );
}

/* --- saw tooth (positive gradient) wave --- */
float sup_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float        samples_per_cycle, sample_value;
    long int    ppos;
    
    samples_per_cycle = ( pos * 2 * M_PI )/node->f;
    ppos = pos + node->phase * samples_per_cycle;
    if( ppos==0 ) return -1;
    sample_value = ( ( ( ( ( ppos % ( long ) samples_per_cycle ) ) / samples_per_cycle ) *2 ) - 1 ) ;
    return( sample_value );
}

/* --- trigonometric saw tooth (positive gradient) wave --- */
float sut_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    sample_value = 2./M_PI* atan(tan( (node->f + (node->phase+0.5)*2*M_PI)/2. ));
    return( sample_value );
}

/* --- saw tooth (negative gradient) wave --- */
float sdn_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float        samples_per_cycle, sample_value;
    long int    ppos;
    samples_per_cycle = ( pos * 2 * M_PI )/node->f;
    ppos = pos + ( node->phase ) * samples_per_cycle;
    if( ppos==0 ) return 1;
    sample_value =  ( ppos % ( long ) samples_per_cycle ) / samples_per_cycle ;
    sample_value =  2.*sample_value - 1. ;
    sample_value*=-1.;
    return( sample_value );
}

/* --- white noise --- */
float wht_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value ;
    sample_value = ( 2.0 * rand() / (float) RAND_MAX ) - 1.0;
    return( sample_value );
}


/* --- random square --- */
/*this is experimental doesn't
quite work as planned yet*/
float rsq_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value ;
    float    r, s;
    
    s = ( 2.0 * rand() / (float) RAND_MAX ) - 1.0;
    r = rand() / (float) RAND_MAX;

    node->frequency = fmt_chunk->SampleRate * node->f / ( pos * 2 * M_PI );   
    
    sample_value=node->mem1;
    if(r<(node->frequency/fmt_chunk->SampleRate)) sample_value=s;
    node->mem1 = sample_value;

    return( sample_value );
}









