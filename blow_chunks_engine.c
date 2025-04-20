#include<chunky.h>
#include<note_lookup.h>

/*
Build the initial variable list of notes names and values
from the  note_lookup  header file
*/
struct variable_node *build_variable_list(  )
{
    struct variable_node *var_node;
    struct variable_node *top_node;
    size_t num_vars = sizeof(note_lookup) / sizeof(NOTE);

    var_node=vnalloc();
    top_node=var_node;
        
    for (size_t i=0; i<num_vars; i++) {
        if(i>0){
            var_node->next=vnalloc();
            var_node=var_node->next;
            var_node->next=NULL;
        }
        
        var_node->key=(char*) malloc(MAX_LINE_LEN*sizeof(char));
        var_node->value=(char*) malloc(MAX_LINE_LEN*sizeof(char));
        strcpy(var_node->key, note_lookup[i].key);
        strcpy(var_node->value, note_lookup[i].value);

    }

    return top_node;
}


int proc_commands( char *string,  struct control *ctrl ){

    char token1[ MAX_LINE_LEN ];
    char tmp1[ MAX_LINE_LEN ];
    int  i, k;
    char *x;

    get_first_string_element( string, token1 );

    /**************************
    ********* @Print **********
    ***************************/
    if (strcmp(token1, "@print") == 0){
        /*created a left trimmed version of the remainder of the string*/
        strcpy(tmp1, string);
        i=k=0;
        while( isspace(tmp1[i++]) ) continue;
        while( tmp1[i-1] != '\n' ){
            string[k++]=tmp1[i-1];  
            i++;
        }
        string[k]='\0';
        fprintf(stderr, "%s\n", string);
        fflush(stderr);
        
    /**************************
    ********* @Volume **********
    ***************************/
    }else if (strcmp(token1, "@volume") == 0){
        if( get_first_string_element( string, token1 )==0 ){
            fprintf(stderr, "exiting: expected argument to @volume command: %s\n", token1);
            exit(1);
        }
        ctrl->master_volume=(float)strtod(token1, &x);
        if(*x!=0){
            fprintf(stderr, "exiting: could not read argument to @volume command: %s\n", token1);
            exit(1);
        }else if(ctrl->master_volume<0.0){
            fprintf(stderr, "exiting: @volume should not be less than 0.0: %s\n", token1);
            exit(1);
        }else if(ctrl->master_volume>1.0){
            fprintf(stderr, "warning: setting @volume greater than 1.0 will end in tears!: %s\n", token1);
        }        
        
    /**************************
    ******* @Sequence *********
    ***************************/
    }else if (strcmp(token1, "@sequence") == 0){
        /*get start time*/
        if( get_first_string_element( string, token1 )==0 ){
            fprintf(stderr, "exiting: expected argument to @sequence command: %s\n", token1);
            exit(1);
        }
        ctrl->seq_start=(float)strtod(token1, &x);
        if(*x!=0){
            fprintf(stderr, "exiting: could not read 1st argument to @sequence command: %s\n", token1);
            exit(1);
        }else if(ctrl->seq_start<0.0){
            fprintf(stderr, "exiting: @sequence start should not be less than 0.0: %s\n", token1);
            exit(1);
        }
        /*get duration*/
        if( get_first_string_element( string, token1 )==0 ){
            fprintf(stderr, "exiting: expected argument to @sequence command: %s\n", token1);
            exit(1);
        }
        ctrl->seq_duration=(float)strtod(token1, &x);
        if(*x!=0){
            fprintf(stderr, "exiting: could not read 2nd argument to @sequence command: %s\n", token1);
            exit(1);
        }else if(ctrl->seq_duration<0.0){
            fprintf(stderr, "exiting: @sequence duration should not be less than 0.0: %s\n", token1);
            exit(1);
        }
        /*reset the total duration of the sound if needed*/
        if((ctrl->seq_start+ctrl->seq_duration)>ctrl->total_length)
            ctrl->total_length=ctrl->seq_start+ctrl->seq_duration;
        
    /**************************
    ********* unknown *********
    ***************************/                
    }else if (strncmp(token1, "@", 1) == 0){
        /*this one must come last, just before 
        the else statement*/
        fprintf(stderr, "exiting: unknown command: %s\n", token1);
        exit(1);
    }else{
        /*if we got here it shouldn't be a 
        command line so return 0 to keep 
        processing the input file*/
        return 0;
    }
    return 1; 
}


/*
Reads the data in line by line.
Comments are removed and blank lines are ignored.
variables and maths are then parsed.
What remains is then fed into a parser, line by line, to
set up the core data structure. 

This function also checks to see if the number of channels
at the top level is constant on every line of the input
data. This could now be done in the text parsing function
instead.

05/04/25 - re-written to make it not a recursive
function as this was causing stack overflows for 
very big input files.

*/
struct wave_node *setup_waveform_data_structures( long int *nlines, long int *nwaves, 
                                  PCM_fmt_chnk *format, struct variable_node *var_node,
                                  struct control *ctrl )
{

    char    line[ MAX_LINE_LEN ];
    char    tmp1[ MAX_LINE_LEN ], tmp2[ MAX_LINE_LEN ];    
    long    counter=0;

    struct wave_node *node, *top_node ;    
    node = NULL ;        
    top_node = NULL ;        
        
    while( fgets( line, MAX_LINE_LEN, stdin ) != NULL ){
    
        (*nlines)++;

        /*=============================================================
        Pre-processing stuff. Strip comments, ignore blank lines,
        assign and substitute variables. Execute commands.
        
        n.b. be very careful with the ordering of these actions.
        ==============================================================*/

        /*strip comments and ignore blank lines*/
        strip_comments( line, ';' );
        if( is_string_blank( line ) ) continue;

        /*substitute variables*/
        substitute_variables( line, var_node );

        /*do any maths*/
        parse_maths( line );
        
        /*commands*/
        strcpy( tmp1, line ) ;
        if( proc_commands( tmp1, ctrl ) ) continue ;
        
        /*assign variables*/
        strcpy( tmp1, line ) ;
        if( assign_variables( tmp1, var_node ) ) continue ;
                
        
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

        if( *nwaves==0 ){
            if( ( node = wnalloc(  ) ) == NULL ){        
                fprintf( stderr, "Failure to allocate memory for data structure\n" );
                exit( EXIT_FAILURE );
            }        
            top_node=node;
        }else{
            if( ( node->next = wnalloc(  ) ) == NULL ){        
                fprintf( stderr, "Failure to allocate memory for data structure\n" );
                exit( EXIT_FAILURE );
            }        
            node=node->next;
        }
        
        /*copy over relevant variables from the control structure*/
        node->master_volume=ctrl->master_volume;    
        node->start_time=ctrl->seq_start;    
        node->duration=ctrl->seq_duration;    
                
        /* parse the string and set up the data structure*/
        parse_modulator( node, line, 0, nlines, format );    
         
        /*track the total number of lines*/        
        (*nwaves)++;

    }
    return( top_node );
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
should also be implemented.

*/
int parse_modulator( struct wave_node *node, char *line, unsigned long depth, long int *nlines, PCM_fmt_chnk *format )
{
    
    char    wfunc[ 20 ];
    char    tmp1[ MAX_LINE_LEN ], tmp2[ MAX_LINE_LEN ];
    
    long      i, counter=0;

    struct ampl_node *a_node ;

    /*
    Perform a couple of checks of the string that has been passed
    N.B. Something similar is done in the calling function too
    and could be placed here too.
    */

    /*check {} are balanced at current depth*/
    strcpy( tmp1, line ) ;                
    if( chop_out_bracketed_from_string( tmp1, '{', '}' ) < 0 ){
        fprintf( stderr, "ill formed {} on line %ld\n", *nlines );
        exit( EXIT_FAILURE );
    }            
                
    /*modulators only have a single amplitude*/
    while( get_first_string_element( tmp1, tmp2 ) ) counter++ ;
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
        
    /*determine the waveform and get function*/        
    get_first_string_element( line, wfunc ) ;
    if ((node->func=assign_oscillator_function(wfunc))==NULL){
        fprintf( stderr, "Unknown wave function: %s, on line %ld\n", wfunc, *nlines);
        exit( EXIT_FAILURE );
    }

    /* get the frequency*/
    get_first_string_element( line, tmp2 ) ;
    node->frequency = (float) strtod( tmp2, (char **)NULL ) ;

    /*get the frequency modulator*/
    node->f_mod=setup_modulator(line,depth,nlines,format);

    /* get the phase*/
    get_first_string_element( line, tmp2 );    
    node->phase = (float) strtod( tmp2, (char **)NULL ) ;
                
    /*get the phase modulator*/                     
    node->p_mod=setup_modulator(line,depth,nlines,format);

    /*Get channel 1 amplitude*/
    get_first_string_element( line, tmp2 );    
    a_node = node->amp_list ;
    a_node->amplitude = (float) strtod( tmp2, (char **)NULL ) ;
    
    /*This is in place of the sanity check function*/
    if( depth == 0 && ( a_node->amplitude > 1 || a_node->amplitude < 0 ) ){
        fprintf( stderr,"amplitude at top level must be <1 and >0 (line %ld)\n", *nlines );
        exit( EXIT_FAILURE );
    }
        
    /*get the amplitude modulator for channel 1)*/                     
    a_node->a_mod=setup_modulator(line,depth,nlines,format);

    /*now loop over the other channels*/
    while( a_node->amp_next != NULL ){
        
        /*get amplitude value*/
        get_first_string_element( line, tmp2 ) ;                        
        a_node = a_node->amp_next ;
        a_node->amplitude = (float) strtod( tmp2, (char **)NULL ) ;

        /*get amplitude modulator*/
        a_node->a_mod=setup_modulator(line,depth,nlines,format);
                    
        if( depth == 0 && ( a_node->amplitude > 1 || a_node->amplitude < 0 ) ){
            fprintf( stderr,"amplitude at top level must be <1 and >0\n" );
            exit( EXIT_FAILURE );
        }
            
    }

    return( 0 );
}


/*
Determines whether there is a modulator in the input
string and, if so, allocates memory for it and calls
parse_modulator() to do further processing.

Is called from, and calls, parse_modulator(), which
makes for a slightly complex recursion, but simplifies 
the code in parse_modulator significantly.

Should only be called from parse_modulator().
*/
struct wave_node * setup_modulator(line, depth, nlines, format)
char *line;
unsigned long depth;
long int *nlines; 
PCM_fmt_chnk *format;
{

    char    tmp[ 20 ];
    char    tmp2[ MAX_LINE_LEN + 20 ];
    char    modulator[ MAX_LINE_LEN ];
    struct wave_node *node;

    get_first_string_element( line, tmp ) ;
        
    /*if what comes back is a { then we expect 
    a frequency modulator to follow*/        
    if( '{' == *tmp ){
        
        /*first put the { back on the front of the string*/
        sprintf( tmp, "{ %s", line );
        strcpy( line, tmp );
        
        /*now get the frequency modulator from the first {} pair*/
        strcpy( modulator, line );
        extract_first_bracketed_from_string( modulator, '{', '}' );
        
        /*allocate memory for node*/
        if( ( node = wnalloc( ) ) == NULL ){
            fprintf( stderr, "Failure to allocate memory for data structure\n" );
            exit( EXIT_FAILURE );
        }
                        
        /*parse the sub string*/
        parse_modulator( node, modulator, depth+1, nlines, format );
                
        /*remove that modulator and get the phase 
        (which should be next)*/    
        chop_out_first_bracketed_from_string( line, '{', '}' );
        
        /*return the address of the modulator*/
        return node;
        
    }

    /*if we get here, we didn't find a 
    modulator so put the line back together 
    and return NULL*/
    sprintf( tmp2, "%s %s",tmp, line );
    strcpy( line, tmp2 );
    return NULL;

}

void calculate_data_value(  struct wave_node *node, PCM_fmt_chnk *fmt_chunk, 
                                    long int pos, long int nwaves, float *sample_value )
{

    float       tmp;
    long        i;
    long int    pos_start;
    long int    pos_end;
    long int    pos_local;
    
    struct ampl_node *a_node ;
    struct wave_node *local_node ;
    
    float num_fade_samples = ( FAST_FADE_MS * fmt_chunk->SampleRate ) / 1000.0 ;
    
    
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
        
        /*could do this when the initial data structures
        are set up - will save some comp time*/
        pos_start=node->start_time*fmt_chunk->SampleRate;
        pos_end=pos_start+node->duration*fmt_chunk->SampleRate;
        pos_local=pos-pos_start;
        
        if( (pos<pos_start)||(pos>pos_end)){
            node=node->next;
            continue ;
        }
        /*set up f, the value that */    
        node->f = node->frequency * pos_local * 2*M_PI / (float) fmt_chunk->SampleRate ;
        
        /*frequency modulation*/
        if( node->f_mod != NULL )
            local_node->f = node->f + node->f_mod->amp_list->amplitude * modulate_waveform( node->f_mod, fmt_chunk, pos_local ) ;
        else        
            local_node->f = node->f ;
                        
        /*phase modulation*/    
        if( node->p_mod != NULL )
            local_node->phase = node->phase + node->p_mod->amp_list->amplitude * modulate_waveform( node->p_mod, fmt_chunk, pos_local ) ;
        else        
            local_node->phase = node->phase ;
                
        /*data value prior to any modification
        for its amplitude*/        
        tmp = node->func( local_node, fmt_chunk, pos_local );

        /*apply master volume here*/
        tmp*=node->master_volume;        
        
        /*apply fast faders here*/ 
        if( pos<(pos_start+num_fade_samples ) )
            tmp*=pos_local/(float)num_fade_samples;  
        if( pos>(pos_end-num_fade_samples ) )
            tmp*=(pos_end-pos)/(float)num_fade_samples;  
                
        /*apply amplitudes for the individual channels*/    
        a_node=node->amp_list ;
        i=0;
        do{
            if(i>0) a_node = a_node->amp_next ;
            if( a_node->a_mod != NULL ){
                *(sample_value + i) += tmp * a_node->amplitude \
                                           * ((a_node->a_mod->amp_list->amplitude \
                                           * (1+modulate_waveform( a_node->a_mod, fmt_chunk, pos_local))/2.));
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
        mod *= ( node->amp_list->a_mod->amp_list->amplitude * 0.5 * ( 1 + modulate_waveform(node->amp_list->a_mod, fmt_chunk, pos) ) );
    
    return( mod ) ;

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


/*Allocate memory for an variable node*/
struct variable_node *vnalloc( void )
{
    return ( struct variable_node *) malloc( sizeof( struct variable_node ) ) ; 
}


/*Error in reading input line*/
void err_bad_line_format( long int l )
{
    fprintf( stderr, "Bad formatting on line %ld -- exiting\n", l );
    exit( EXIT_FAILURE );
}

/*functions for examiinng the data structure*/
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
