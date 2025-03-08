#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chunky.h>


int	main( int argc, char **argv )
{

	long int		i=0, j,num_samples;
	long int		nlines=0, nwaves=0;
	int             verbose=FALSE ;
	float			*sample_value, duration=1.0;
	float			num_fade_samples;
	
	RIFF_hdr		wav_header;
	chunk_hdr		fmt_header, data_header;
	PCM_fmt_chnk		fmt_chunk;
	
	struct	wave_node	*top_node;
	
	void 			parser(  );
	
	
	/* -- Setup defaults -- */
	fmt_chunk.Channels=1;
	fmt_chunk.SampleRate=44100;
	fmt_chunk.AvgBytesPerSec=88200;
	fmt_chunk.BlockAlign=2;
	fmt_chunk.PCM_bps=16;
	
	/*parse command line*/
	parser( argc, argv, &duration, &fmt_chunk );

	/* Set up the data structures */	
	if ( ( top_node = setup_waveform_data_structures( &nlines, &nwaves, &fmt_chunk ) ) == NULL ){
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
			
	/*allocate memeory for the samples*/	
	if( ( sample_value = (float *) malloc( sizeof(float) *  fmt_chunk.Channels ) ) == NULL ){
		fprintf( stderr, "Failure to allocate memory for sample array\n" );
		exit( EXIT_FAILURE );		
	}

	/* Setup header information and format chunk */
	setup_PCM_fmt_chunk( &fmt_chunk );
	setup_chunk_headers( &wav_header, &fmt_header, &data_header, &fmt_chunk, duration );
	
	/* Write .wav file up to start of data */
	write_wav_header( &wav_header );
	write_chunk_hdr( &fmt_header );	
	write_PCM_fmt_chunk( &fmt_chunk );
	write_chunk_hdr( &data_header );
	
	
	/* Calculate and write data values */	
	
	num_samples = duration * fmt_chunk.SampleRate ; 
	num_fade_samples = ( FAST_FADE_MS * fmt_chunk.SampleRate ) / 1000.0 ;
	
	/*Fast fade in*/
	while( i < num_fade_samples ){
		calculate_data_value( top_node, &fmt_chunk, i, nwaves, sample_value );	
		for( j=0; j<fmt_chunk.Channels; j++ ) *( sample_value + j ) = \
					*( sample_value + j ) * ( i / (float)num_fade_samples ) ;
		write_pcm_data_sample( &fmt_chunk, sample_value );
		
		++i;		
	}
	
	/*Non-faded data*/
	while( i < ( num_samples - num_fade_samples ) ){		
		calculate_data_value( top_node, &fmt_chunk, i, nwaves, sample_value );
		write_pcm_data_sample( &fmt_chunk, sample_value );
		++i;
	}	
		
	/*Fast fade out*/
	while( i < num_samples ){
		calculate_data_value( top_node, &fmt_chunk, i, nwaves, sample_value );
		for( j=0; j<fmt_chunk.Channels; j++ ) *( sample_value + j ) = \
					*( sample_value + j ) * ( (num_samples-i-1) / (float)num_fade_samples );
		write_pcm_data_sample( &fmt_chunk, sample_value );
		++i ;
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

