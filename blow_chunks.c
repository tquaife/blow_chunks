#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chunky.h>


int	main( int argc, char **argv )
{

	long int		i=0, num_samples;
	long int		nlines=0, nwaves=0;
	int             verbose=FALSE ;
	float			*sample_value; 
	float           default_duration=1.0;
	
	RIFF_hdr		wav_header;
	chunk_hdr		fmt_header, data_header;
	PCM_fmt_chnk    fmt_chunk;
	
	struct	wave_node	*top_node;
    struct variable_node *var_node ;
    struct control ctrl; //not a pointer!
	
	void 			clparser(  );	
	
	/* -- Setup defaults -- */
	fmt_chunk.Channels=1;
	fmt_chunk.SampleRate=44100;
	fmt_chunk.AvgBytesPerSec=88200;
	fmt_chunk.BlockAlign=2;
	fmt_chunk.PCM_bps=16;
    ctrl.seq_duration=default_duration;
    ctrl.total_length=default_duration;
	
	/*parse command line*/
	clparser( argc, argv, &ctrl, &fmt_chunk );

    /*set up in-built variables*/
    var_node = NULL ;
    var_node=build_variable_list(  );

    /*set up the control structure*/
    ctrl.master_volume=1.0;
    ctrl.seq_start=0.0;
    //ctrl.seq_duration=duration;
    //ctrl.total_length=duration;

	/* Set up the data structures */	
	if ( ( top_node = setup_waveform_data_structures( &nlines, &nwaves, &fmt_chunk, var_node, &ctrl ) ) == NULL ){
		fprintf(stderr, "%s: no data found at stdin\n", argv[ 0 ] );
		exit( EXIT_FAILURE );
	} 

    /*print the data structure for checking*/
    if(verbose)print_data(top_node,0);

	/* - Check output is being redirected - */
	if( isatty( fileno( stdout ) ) ){
		fprintf( stderr, "%s: output must be redirected\nexiting\n", argv[ 0 ] );
		exit( EXIT_FAILURE );                
    }		
			
	/*allocate memory for the samples*/	
	if( ( sample_value = (float *) malloc( sizeof(float) *  fmt_chunk.Channels ) ) == NULL ){
		fprintf( stderr, "Failure to allocate memory for sample array\n" );
		exit( EXIT_FAILURE );		
	}

	/* Setup header information and format chunk */
	setup_PCM_fmt_chunk( &fmt_chunk );
	setup_chunk_headers( &wav_header, &fmt_header, &data_header, &fmt_chunk, ctrl.total_length );
	
	/* Write .wav file up to start of data */
	write_wav_header( &wav_header );
	write_chunk_hdr( &fmt_header );	
	write_PCM_fmt_chunk( &fmt_chunk );
	write_chunk_hdr( &data_header );
	
	
	/* Calculate and write data values */		
	num_samples = ctrl.total_length * fmt_chunk.SampleRate ; 
	
	/*loop over time (samples) and write out*/
	while( i < num_samples ){		
		calculate_data_value( top_node, &fmt_chunk, i, nwaves, sample_value );
		write_pcm_data_sample( &fmt_chunk, sample_value );
		++i;
	}	
		

	return( EXIT_SUCCESS );	
}


void    clparser( argc, argv, ctrl, format )
int     argc;
char    **argv;
struct control *ctrl ;
PCM_fmt_chnk *format;
{
    int     i = 1;
    void    usage( );

    for( i=1; i<argc; i++ ){

        if( *argv[ i ] == '-' ){
                
            /**/ if( !strncasecmp(argv[ i ],"--duration",5) || !strcasecmp(argv[ i ],"-d") )   
            {       
                     ctrl->seq_duration = (float)atof( argv[ ++i ] );
                     ctrl->total_length = ctrl->seq_duration ;
            }
            else if( !strncasecmp( argv[ i ],"--bits_per_sample",5 ) || !strcasecmp(argv[ i ],"-b") )                 
                    format->PCM_bps = (short)atoi( argv[ ++i ] );

            else if( !strncasecmp( argv[ i ],"--sample_rate",5 ) || !strcasecmp(argv[ i ],"-s") )                 
                    format->SampleRate = (long)atoi( argv[ ++i ] );                    

            else if( !strncasecmp( argv[ i ],"--usage",5 ) || !strcasecmp(argv[ i ],"-u") )                 
                    usage(argv[0]);                    
            else{
                    fprintf(stderr,"%s: unknown argument on command line: %s\n",argv[0],argv[i]);
                    usage(argv[0]);
           }
        }
    }
}

void usage(char *binary_name)
{
        fprintf(stderr, "\nusage: %s [options] < input.txt > output.wav\n", binary_name); 
        fprintf(stderr, "\nwhere [options] are:\n\n" );

        fprintf(stderr, "--duration|-d arg\t\twhere arg is the duration in seconds [float]\n" );
        fprintf(stderr, "--bits_per_sample|-b arg\twhere arg is the bits per sample of the wav file [int]\n" );
        fprintf(stderr, "--sample_rate|-s arg\t\twhere arg is the sample rate of the wav file in Hertz [int]\n" );

        fprintf(stderr, "\n\n");
        exit( EXIT_FAILURE );        
}

