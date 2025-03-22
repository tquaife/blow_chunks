#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chunky.h>


int	main( int argc, char **argv )
{

	long int		i=0, num_samples;
	long int		nlines=0, nwaves=0;
	int             verbose=FALSE ;
	float			*sample_value, duration=1.0;
	
	RIFF_hdr		wav_header;
	chunk_hdr		fmt_header, data_header;
	PCM_fmt_chnk		fmt_chunk;
	
	struct	wave_node	*top_node;
    struct variable_node *var_node ;
    struct control ctrl; //not a pointer!
	
	void 			parser(  );
	
	
	/* -- Setup defaults -- */
	fmt_chunk.Channels=1;
	fmt_chunk.SampleRate=44100;
	fmt_chunk.AvgBytesPerSec=88200;
	fmt_chunk.BlockAlign=2;
	fmt_chunk.PCM_bps=16;
	
	/*parse command line*/
	parser( argc, argv, &duration, &fmt_chunk );

    /*set up in-built variables*/
    var_node = NULL ;
    var_node=build_variable_list(  );

    /*set up the control structure*/
    ctrl.master_volume=1.0;
    ctrl.seq_start=0.0;
    ctrl.seq_duration=duration;
    ctrl.total_length=duration;

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
	
	
	while( i < num_samples ){		
		calculate_data_value( top_node, &fmt_chunk, i, nwaves, sample_value );
		write_pcm_data_sample( &fmt_chunk, sample_value );
		++i;
	}	
		

	return( EXIT_SUCCESS );	
}


void    parser( argc, argv, seconds, format )
int     argc;
char    **argv;
float   *seconds;
PCM_fmt_chnk *format;
        {
        int     i = 1;
        void    usage( );

/*
**      Derive options specified on the command-line.
*/
        for( i = 1; i < argc; i++ )
        {       if( *argv[ i ] == '-' )
                {       switch( *( argv[ i ] + 1 ) )
						{      
				case 'd':
                                        *seconds = atof( argv[ ++i ] );
					break; 
										
				/*	As of version 6 the number of channels will
				be determined by the format of the input file */
				
				/*	
                                case 'c':
                                	format->Channels = ( short )atoi( argv[ ++i ] );
                                	break;
				*/
                                
				case 'b':
                                        format->PCM_bps = ( short )atoi( argv[ ++i ] );
                                        break;
				case 'f':
                                        format->SampleRate = ( long )atoi( argv[ ++i ] );
                                        break;
                                case 'u':
                                        usage( argv[ 0 ] );
                                        break;
                                default:
                                        usage( argv[ 0 ] );
                                        break;                                  
                        }
		}
	}
	
}

void usage( char *binary_name )
{
        
        fprintf(stderr, "usage: %s -d [duration] -b [bps] \
			\n-f [sampling freq] -u < instrument.dat > out.wav \n", binary_name);
        
        exit( EXIT_FAILURE );
        
}

