#include<chunky.h>

/*
A recursive function which reads the data in line by line.
Comments are removed and blank lines are ignored.
What remains is then fed into a parser, line by line, to
set up the core data structure. 

Should probably be re-written so that it isn't recursive
but it is not a time limiting factor in a typical run.

This function also checks to see if the number of channels
at the top level is constant on every line of the input
data. This could now be done in the text parsing function
instead.
*/
struct wave_node *setup_waveform_data_structures( long int *nlines, long int *nwaves, PCM_fmt_chnk *format  )
{

    char    line[ MAX_LINE_LEN ];
    char    tmp1[ MAX_LINE_LEN ], tmp2[ MAX_LINE_LEN ];    
    long    counter=0;

    struct wave_node *node ;    
    node = NULL ;

    while( fgets( line, MAX_LINE_LEN, stdin ) != NULL ){
    
        *nlines++;

        /*strip comments and ignore blank lines*/
        strip_comments( line, ';' );
        if( is_string_blank( line ) ) continue;
      
        
        /*=============================================================
        Start with an initial parse of each line to check the number of  
        channels is consistent. Use the first line to set the number of 
        channels.
        ==============================================================*/
        counter=0;
        strcpy( tmp1, line ) ;
                  
        /*Chop out modulators, i.e. within {}*/
        if( chop_out_bracketed_from_string( tmp1, '{', '}' ) < 0 ){
            fprintf( stderr, "ill formed {} on line %ld\n", *nlines );
            exit( EXIT_FAILURE );
        }
        
        /*count number of fields*/
        while( get_first_string_element( tmp1, tmp2 ) ) counter++ ;

        if( *nwaves == 0 ){
            /*If first line of data, set number of channels 
            (3 is the number of elements preceding the the amplitudes)*/
            format->Channels = counter - 3 ;                              
            if( counter < 4 ){            
                fprintf( stderr, "Error on line %ld of input data: insufficient number of fields\n", *nlines );
                exit( EXIT_FAILURE );
            }
        }else if((counter-3) != format->Channels){
            /*If this is not the first line of data then
            make sure the number of channels matches*/
                fprintf( stderr, "Error on line %ld of input data: inconsistent number of fields\n", *nlines );
                exit( EXIT_FAILURE );
        }
        

        /*===============================================
        Now read the input file into the data structure
        =================================================*/
        
        /*Allocate the wave node*/    
        if( ( node = wnalloc(  ) ) == NULL ){        
            fprintf( stderr, "Failure to allocate memory for data structure\n" );
            exit( EXIT_FAILURE );
        }
                
        /* parse the string and set up the data structure*/
        parse_modulator( node, line, 0, nlines, format );    
         
        /*track the total number of lines*/        
        (*nwaves)++;

        /*Read the next data line into the wnode tree*/
        node->next = setup_waveform_data_structures( nlines, nwaves, format );

    }
    return( node );
} 



/*
This function is the main part of the input file parsing
code. It chops up the input ascii data into the waveforms
and their component modulators and builds the data 
structure which blow_chunks then goes on to interpret
to make the data with.

Still needs work. Usable return values might be nice
for example. The code could probably also be simplified
a lot too and error checking on the calls to strtod
should also be implimented.

** THIS FUNCTION NEEDS UPDATING WHEN NEW WAVEFORMS ARE**
              **ADDED TO THE CODE**

*/
int parse_modulator( struct wave_node *node, char *line, unsigned long depth, long int *nlines, PCM_fmt_chnk *format )
{
    
    char    wfunc[ 10 ];
    char    tmp1[ MAX_LINE_LEN ], tmp2[ MAX_LINE_LEN ];
    char    modulator[ MAX_LINE_LEN ];
    
    long      i, counter=0;

    struct ampl_node *a_node ;

    /*
    Perform a couple of checks of the string that has been passed
    N.B. Something similar is done in the calling function too
    and could be placed here too.
    */

    strcpy( tmp1, line ) ;                
    if( chop_out_bracketed_from_string( tmp1, '{', '}' ) < 0 ){
        fprintf( stderr, "ill formed {} on line %ld\n", *nlines );
        exit( EXIT_FAILURE );
    }            
                
    while( get_first_string_element( tmp1, tmp2 ) ) counter++ ;

    /*modulators only have a single amplitude*/
    if( counter > 4 && depth > 0 ){
        fprintf( stderr, "modulator with more then one amplitude on line %ld\n", *nlines );
        exit( EXIT_FAILURE );
    }

    /*Initialise the other node pointers to NULL*/
    /*(except amp - which is dealt with later)*/    
    node->f_mod = NULL ; 
    node->p_mod = NULL ; 
    node->next  = NULL ;

    /*++++++++++++++++++++++++++++++++++++++++++++++++
    The following multi-amplitude operations only need
    to performed for one channel unless we are at the
    top level in which case we need one per channel.
    +++++++++++++++++++++++++++++++++++++++++++++++++*/            
        
    /*Set up first amplitude node*/
    node->amp_list = analloc(  ) ;
    a_node = node->amp_list ;
    a_node->a_mod = NULL ;
    a_node->amp_next = NULL ;
        
    /*If we're at the top level set
    up the subsequent channels*/
    if( depth == 0 ){
        for( i=1; i<format->Channels; i++ ){
            a_node->amp_next = analloc(  ) ;
            a_node = a_node->amp_next ;
            a_node->a_mod = NULL ;
            a_node->amp_next = NULL ;
        }
    }

    /*first pad { and } with white space to make them 
    come out separately from get_first_string_element*/
    pad_char_in_str_with_char( line, '{', ' ', MAX_LINE_LEN );
    pad_char_in_str_with_char( line, '}', ' ', MAX_LINE_LEN );
        
    /*determine the waveform*/        
    get_first_string_element( line, wfunc ) ;
        
    if( ! strcmp( wfunc, "sin" ) )
        node->func=&sin_wave;     
    else if( ! strcmp( wfunc, "sqx" ) )
        node->func=&sqx_wave;
    else if( ! strcmp( wfunc, "sqr" ) )
        node->func=&sqr_wave;
    else if( ! strcmp( wfunc, "tri" ) )
        node->func=&tri_wave;
    else if( ! strcmp( wfunc, "sn3" ) )
        node->func=&sn3_wave;
    else if( ! strcmp( wfunc, "sn5" ) )
        node->func=&sn5_wave;
    else if( ! strcmp( wfunc, "rnd" ) )
        node->func=&rnd_wave;
    else{
        fprintf( stderr, "Unknown wave function: %s, on line %ld\n", wfunc, *nlines);
        exit( EXIT_FAILURE );
    }

    /* get the frequency, phase and amplitudes */
    get_first_string_element( line, tmp2 ) ;
    node->frequency = (float) strtod( tmp2, (char **)NULL ) ;

    get_first_string_element( line, tmp2 ) ;
        
    /*if what comes back is a { then we expect 
    a frequency modulator to follow*/
        
    if( '{' == *tmp2 ){
        
        /*first put the { back on the front of the string*/
        sprintf( tmp1, "{ %s", line );
        strcpy( line, tmp1 );
        
        /*now get the frequency modulator from the first {} pair*/
        strcpy( modulator, line );
        extract_first_bracketed_from_string( modulator, '{', '}' );
        
        /*allocate memory for node*/
        if( ( node->f_mod = wnalloc( ) ) == NULL ){
            fprintf( stderr, "Failure to allocate memory for data structure\n" );
            exit( EXIT_FAILURE );
        }
                        
        /*parse the sub string*/
        parse_modulator( node->f_mod, modulator, depth+1, nlines, format );
                
        /*remove that modulator and get the phase 
        (which should be next)*/    
        chop_out_first_bracketed_from_string( line, '{', '}' );
        
        get_first_string_element( line, tmp2 );
        
    }
        
    node->phase = (float) strtod( tmp2, (char **)NULL ) ;
                
    get_first_string_element( line, tmp2 ) ;

    /*if what comes back is a { then we expect 
    a phase modulator to follow*/
        
    if( '{' == *tmp2 ){
        
        /*first put the { back on the front of the string*/
        sprintf( tmp1, "{ %s", line );
        strcpy( line, tmp1 );
        
        /*now get the modulator from the first {} pair*/
        strcpy( modulator, line );
        extract_first_bracketed_from_string( modulator, '{', '}' );
        
        /*allocate memory for node*/
        if( ( node->p_mod = wnalloc( ) ) == NULL ){
            fprintf( stderr, "Failure to allocate memory for data structure\n" );
            exit( EXIT_FAILURE );
        }
                        
        /*parse the sub string*/
        parse_modulator( node->p_mod, modulator, depth+1, nlines, format );
                
        /*remove that modulator*/
        chop_out_first_bracketed_from_string( line, '{', '}' );
        
        /*get the next element*/
        get_first_string_element( line, tmp2 );
        
    }

    /*Get channel 1 amplitude*/
    a_node = node->amp_list ;
    a_node->amplitude = (float) strtod( tmp2, (char **)NULL ) ;
    
    /*This is in place of the sanity check function*/
    if( depth == 0 && ( a_node->amplitude > 1 || a_node->amplitude < 0 ) ){
        fprintf( stderr,"amplitude at top level must be <1 and >0 (line %ld)\n", *nlines );
        exit( EXIT_FAILURE );
    }
        
    get_first_string_element( line, tmp2 );    
        
    /*if what comes back is a { then we expect 
    an amplitude modulator to follow*/
        
    if( '{' == *tmp2 ){
        
        /*first put the { back on the front of the string*/
        sprintf( tmp1, "{ %s", line );
        strcpy( line, tmp1 );
        
        /*now get the modulator from the first {} pair*/
        strcpy( modulator, line );
        extract_first_bracketed_from_string( modulator, '{', '}' );
        
        /*allocate memory for node*/
        if( ( a_node->a_mod = wnalloc( ) ) == NULL ){
            fprintf( stderr, "Failure to allocate memory for data structure\n" );
            exit( EXIT_FAILURE );
        }
                        
        /*parse the sub string*/
        parse_modulator( a_node->a_mod, modulator, depth+1, nlines, format );
                
        /*remove that modulator*/
        
        chop_out_first_bracketed_from_string( line, '{', '}' );
        
        get_first_string_element( line, tmp2 );
        
    }
    
        
    while( a_node->amp_next != NULL ){
        
        if( depth > 0 ){
            fprintf( stderr, "modulators must only have a single amplitude: %ld", *nlines );
            exit( EXIT_FAILURE );
        }
        
        a_node = a_node->amp_next ;
        a_node->amplitude = (float) strtod( tmp2, (char **)NULL ) ;
                    
        get_first_string_element( line, tmp2 ) ;            
                    
        /*if what comes back is a { then we expect 
        an amplitude modulator to follow*/
        
        if( '{' == *tmp2 ){
        
            /*first put the { back on the front of the string*/
            sprintf( tmp1, "{ %s", line );
            strcpy( line, tmp1 );
        
            /*now get the modulator from the first {} pair*/
            strcpy( modulator, line );
            extract_first_bracketed_from_string( modulator, '{', '}' );
        
            /*allocate memory for node*/
            if( ( a_node->a_mod = wnalloc( ) ) == NULL ){
                fprintf( stderr, "Failure to allocate memory for data structure\n" );
                exit( EXIT_FAILURE );
            }
                        
            /*parse the sub string*/
            parse_modulator( a_node->a_mod, modulator, depth+1, nlines, format );
                
            /*remove that modulator*/
        
            chop_out_first_bracketed_from_string( line, '{', '}' );
        
            if( a_node->amp_next != NULL ) get_first_string_element( line, tmp2 );
        }                
                    
        if( depth == 0 && ( a_node->amplitude > 1 || a_node->amplitude < 0 ) ){
            fprintf( stderr,"amplitude at top level must be <1 and >0\n" );
            exit( EXIT_FAILURE );
        }
            
    }


    return( 0 );

}


/*Allocate memory for a waveform node*/
struct wave_node *wnalloc( void )
{
    //return ( struct wave_node *) malloc( sizeof( struct wave_node ) );
    struct wave_node *node;
    node = ( struct wave_node *) malloc( sizeof( struct wave_node ) );
    node->rnd_mem=0.0;
    return node;
}

/*Allocate memory for an amplitude node*/
struct ampl_node *analloc( void )
{
    return ( struct ampl_node *) malloc( sizeof( struct ampl_node ) ) ; 
}




void calculate_data_value(  struct wave_node *node, PCM_fmt_chnk *fmt_chunk, 
                                    long int pos, long int nwaves, float *sample_value )
{

    float        tmp;
    long        i;
    
    struct ampl_node *a_node ;
    struct wave_node *local_node ;
    
    /*allocate local node*/    
    if( ( local_node = wnalloc(   ) ) == NULL ){
        fprintf( stderr, "Failure to allocate memory for data structure\n" );
        exit( EXIT_FAILURE );            
    }

    /*init sample array at 0*/
    for( i=0; i<fmt_chunk->Channels ; i++ )
        *(sample_value + i) = 0.00 ;
    
    tmp = 0.00 ;
    
    /*loop through the data structure*/
    
    while( node != NULL ){
        
        /*set up f, the value that */    
        node->f = node->frequency * pos * 2*M_PI / (float) fmt_chunk->SampleRate ;
        
        /*frequency modulation*/
        if( node->f_mod != NULL )
            local_node->f = node->f + node->f_mod->amp_list->amplitude * modulate_waveform( node->f_mod, fmt_chunk, pos ) ;
        else        
            local_node->f = node->f ;
                        
        /*phase modulation*/    
        if( node->p_mod != NULL )
            local_node->phase = node->phase + node->p_mod->amp_list->amplitude * modulate_waveform( node->p_mod, fmt_chunk, pos ) ;
        else        
            local_node->phase = node->phase ;
                
        tmp = node->func( local_node, fmt_chunk, pos );
                
        /*apply amplitudes for the individual channels*/    
        a_node=node->amp_list ;
        i=0;
        do{
            if(i>0) a_node = a_node->amp_next ;
            if( a_node->a_mod != NULL ){
                *(sample_value + i) += tmp * a_node->amplitude \
                                           * (1 -(node->amp_list->a_mod->amp_list->amplitude \
                                           * (1+modulate_waveform( a_node->a_mod, fmt_chunk, pos))/2.));
            }else{
                *(sample_value + i) += tmp * a_node->amplitude ;
            }
            i+=1;
        }while( a_node->amp_next != NULL );

        node=node->next;        
    }    

    /*scale the resulting values correctly*/    
    for( i=0; i<fmt_chunk->Channels ; i++ )
        *(sample_value + i) = *(sample_value + i)/(float)nwaves ;
    
    return;
    
}


float modulate_waveform( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos )
{

    float mod = 0.00 ;
    struct wave_node *local_node ;

    /*allocate local node*/
    
    if( ( local_node = wnalloc(   ) ) == NULL ){
        fprintf( stderr, "Failure to allocate memory for data structure\n" );
        exit( EXIT_FAILURE );            
    }

    node->f = node->frequency * pos * 2 * M_PI / (float) fmt_chunk->SampleRate  ;

    if( node->f_mod != NULL )
        local_node->f = node->f + node->f_mod->amp_list->amplitude * modulate_waveform( node->f_mod, fmt_chunk, pos ) ;
    else        
        local_node->f = node->f ;
        
    /*phase modulations*/    
    if( node->p_mod != NULL )
        local_node->phase = node->phase + node->p_mod->amp_list->amplitude * modulate_waveform( node->p_mod, fmt_chunk, pos ) ;
    else        
        local_node->phase = node->phase ;
            
    mod = node->func( local_node, fmt_chunk, pos );

    /*amplitude modulation*/
    if( node->amp_list->a_mod != NULL )
        mod *= ( 1. - node->amp_list->a_mod->amp_list->amplitude * 0.5 * ( 1 + modulate_waveform(node->amp_list->a_mod, fmt_chunk, pos) ) );
    
    return( mod ) ;

}



/*
   ======The oscillators=====
N.B. they must all have the same args 
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


/* --- a differentiable (smooth) version of the sqaure wave --- */

float sqx_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value;
    float    delta=0.005;
    sample_value = 2./M_PI*atan(sin( node->f + node->phase*2*M_PI )/delta);

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

/* --- tringular (positive gradient) wave --- */

float tri_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float        samples_per_cycle, sample_value;
    long int    ppos;

    
    node->frequency = fmt_chunk->SampleRate * node->f / ( pos * 2 * M_PI );

    samples_per_cycle = fmt_chunk->SampleRate / node->frequency;
    
    /*samples_per_cycle = pos * 2 * M_PI / node->f ;*/

    ppos = pos + node->phase * samples_per_cycle;
    sample_value = ( ( ( ( ( ppos % ( long ) samples_per_cycle ) ) / samples_per_cycle ) *2 ) - 1 ) ;


    return( sample_value );
}


/* --- random noise --- */

float rnd_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos ){

    float    sample_value ;
    
    sample_value = ( 2.0 * rand() / (float) RAND_MAX ) - 1.0;

    /* did not work as expexted... but shows how memory can be included:*/
    /*
    node->frequency = fmt_chunk->SampleRate * node->f / ( pos * 2 * M_PI );   
    sample_value = sample_value + node->frequency * node->rnd_mem;
    node->rnd_mem = sample_value;
    */
    
    return( sample_value );
}






/*Error in reading input line*/

void err_bad_line_format( long int l )
{

    fprintf( stderr, "Bad formatting on line %ld -- exiting\n", l );
    exit( EXIT_FAILURE );

}


/*functions for examing the data structure*/

void print_wnode( struct wave_node *wnode, PCM_fmt_chnk *format )
{
    
    /*long i;*/
    struct ampl_node *a_node;
    
    a_node = wnode->amp_list;

    fprintf( stderr, "w: %p\n", wnode->func );
    fprintf( stderr, "f: %f\n", wnode->frequency );
    fprintf( stderr, "p: %f\n", wnode->phase );
    fprintf( stderr, "a: %f\n",  a_node->amplitude );
    while( a_node->amp_next != NULL ){
        a_node = a_node->amp_next ;
        fprintf( stderr, "a: %f\n",  a_node->amplitude );
    }
    fprintf( stderr, "%p\n", wnode->next );

    return ;
}



/*
A recurseive function which prints an ascii representation
of the main data structure to the stderr. Only for testing
purposes.
*/

void print_data( struct wave_node *wnode, int depth )
{
    
    long i;
    struct ampl_node *a_node;
    
    a_node = wnode->amp_list;

    for( i=0; i<depth; i++ ) fprintf( stderr, "-" );
    fprintf( stderr, "w: %p\n", wnode->func );
    
    for( i=0; i<depth; i++ ) fprintf( stderr, "-" );
    fprintf( stderr, "f: %f\n", wnode->frequency );
    if( wnode->f_mod != NULL ) print_data( wnode->f_mod, depth + 1 );
    
    for( i=0; i<depth; i++ ) fprintf( stderr, "-" );
    fprintf( stderr, "p: %f\n", wnode->phase );
    if( wnode->p_mod != NULL ) print_data( wnode->p_mod, depth + 1 );
    
    for( i=0; i<depth; i++ ) fprintf( stderr, "-" );    
    fprintf( stderr, "a: %f\n",  a_node->amplitude );
    if( a_node->a_mod != NULL ) print_data( a_node->a_mod, depth + 1 );
    
    while( a_node->amp_next != NULL ){
        a_node = a_node->amp_next ;
        for( i=0; i<depth; i++ ) fprintf( stderr, "-" );
        fprintf( stderr, "a: %f\n",  a_node->amplitude );
        if( a_node->a_mod != NULL ) print_data( a_node->a_mod, depth + 1 );
    }
    
    if( wnode->next != NULL ) print_data( wnode->next, depth );
    
    return ;
}
