#define LIBCHUNKY_VERSION 0.0.1

/*
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
*/

#include<chunky.h>

/*
** ------------------------------------------
** Calculate the ABPS and the block alignment
** This requires that values have been assigned
** to the sampling frequency, bps and no. of
** channels already. 
** ------------------------------------------
*/

void setup_PCM_fmt_chunk( PCM_fmt_chnk *fmt_chunk )
{        
    /* - Calculate block align and avg bytes per second - */                       
    fmt_chunk->AvgBytesPerSec=fmt_chunk->Channels\
                             *fmt_chunk->SampleRate\
                             *(float)fmt_chunk->PCM_bps/8.0;
                                     
    fmt_chunk->BlockAlign=fmt_chunk->Channels\
                             *(float)fmt_chunk->PCM_bps/8.0;

    fmt_chunk->FormatTag=WAVE_FORMAT_PCM;

	return;
}


void setup_chunk_headers( RIFF_hdr *wav_header, chunk_hdr *fmt_header, chunk_hdr *data_header,\
				PCM_fmt_chnk *fmt_chunk, float duration )
{

	strncpy( wav_header->id,   "RIFF", 4 );
	strncpy( wav_header->type, "WAVE", 4 );
	strncpy( fmt_header->id,   "fmt ", 4 );
	strncpy( data_header->id,  "data", 4 );
	
	fmt_header->length=16; /*PCM wav file fmt chunk length*/

	data_header->length=fmt_chunk->Channels\
			  *fmt_chunk->SampleRate\
			  *( float )fmt_chunk->PCM_bps/8.0\
			  *duration;
				     
	wav_header->length=sizeof( wav_header->type )\
			 +sizeof( chunk_hdr )\
			 +sizeof( PCM_fmt_chnk )\
			 +sizeof( chunk_hdr )\
			 +data_header->length ;
			
	return;
}

void write_wav_header( FILE *outfp, RIFF_hdr *wav_header )
{
	fwrite( wav_header, sizeof( RIFF_hdr ), 1, outfp );
	return;
}


void write_PCM_fmt_chunk( FILE *outfp, PCM_fmt_chnk *fmt_chunk )
{
	fwrite( fmt_chunk, sizeof( PCM_fmt_chnk ), 1, outfp );
	return;
}


void write_chunk_hdr( FILE *outfp, chunk_hdr *header )
{
	fwrite( header, sizeof( chunk_hdr ), 1, outfp );
	return;
}


void write_pcm_data_sample( FILE *outfp, PCM_fmt_chnk *fmt_chunk, float *sample_value )
{
	short int 	bps16_value;
	char	 	bps8_value;
	long		i ;

	if( fmt_chunk->PCM_bps == 16 ){	
		for( i=0; i<fmt_chunk->Channels; i++ ){
			bps16_value = *(sample_value + i) * 32767;
			fwrite( &bps16_value , sizeof( short int ), 1, outfp );
		}
	
	}else if( fmt_chunk->PCM_bps == 8 ){
		for( i=0; i<fmt_chunk->Channels; i++ ){
			bps8_value = *(sample_value + i) * 127;
			fwrite( &bps8_value , sizeof( char ), 1, outfp );
		}

	}else{	
		fprintf( stderr, "Unsupported bits per sample: %d\n", fmt_chunk->PCM_bps );
		exit( EXIT_FAILURE );
	}

	return;
}


