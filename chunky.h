#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


#define MAX_LINE_LEN 400
#define WAVE_FORMAT_PCM 0x0001
#define FAST_FADE_MS 5

#define DEBUGPRINT fprintf(stderr,"here: %s %d\n",__FILE__,__LINE__);



/* ============================ */
/* Structures and other typedefs*/
/* ============================ */


/*At the top of any MS wav file*/
typedef struct  {
		char	 id[ 4 ];
		int32_t length;
		char	 type[ 4 ];
		} RIFF_hdr;


/*All chunks have this header*/
typedef struct  {
		char	 id[ 4 ];
		int32_t length;
		} chunk_hdr;


/*The format chunk for a PCM wav file*/
typedef struct  {
		int16_t	FormatTag;
		int16_t	Channels;
		int32_t	SampleRate;
		int32_t	AvgBytesPerSec;
		int16_t	BlockAlign;
		int16_t	PCM_bps;
		} PCM_fmt_chnk;
	
	
	
/*The self referential data structure used for
defining waveforms in the blow_chunks synth*/

/*Note that as of version 0.6 amplitude is a
pointer and so memory needs allocating to it*/
/*Note that as of version 0.7 amplitude is now 
it's own self referential data structure. 
This make assigning the modulators a lot less
messy even though it appears to be more so at
the first glance*/
/*The "f" member of the structure is to hold
and intermediate part of the processing for
frequency modulation (0.7b)*/
/*"rnd_mem" contains a memory term for the 
rnd generator*/


struct	wave_node {
		float				(*func)( );
		float				frequency;
		float				f;
		struct wave_node	*f_mod;	
		float				phase;
		struct wave_node	*p_mod;
		struct ampl_node	*amp_list;
		struct wave_node	*next;
		
		float               rnd_mem;
		};
		
/*
A structure to hold the amplitude info for
each channel.
*/

struct 	ampl_node {
		float				amplitude;
		struct	wave_node	*a_mod;
		struct	ampl_node	*amp_next;
		};
	
/*
The linked list to store the envelopes in
***** THIS IS NOT CURRENTLY USED ******
*/

struct envl_node  {
		float				samps ;
		float				value ;
		struct envl_node	*next ;
		} ;


/*
Data list for variables
*/

struct variable_node {
  char                   *key;
  char                   *value;
  struct variable_node   *next;
  };

		
/* ===================== */	
/* Function declarations */
/* ===================== */

/*General Writing functions*/
void setup_PCM_fmt_chunk( PCM_fmt_chnk *fmt_chunk );
void setup_chunk_headers( RIFF_hdr *wav_header, chunk_hdr *fmt_header, chunk_hdr *data_header,\
			      PCM_fmt_chnk *fmt_chunk, float duration );
void write_wav_header( RIFF_hdr *wav_header );
void write_PCM_fmt_chunk( PCM_fmt_chnk *fmt_chunk );
void write_chunk_hdr( chunk_hdr *header );
void write_pcm_data_sample( PCM_fmt_chnk *fmt_chunk, float *sample_value );

/*Blow chunks functions*/
void calculate_data_value( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, 
								long int pos, long int nwaves, float *sample_value );
struct wave_node *wnalloc( void );
struct ampl_node *analloc( void );
struct wave_node *setup_waveform_data_structures( long int *nlines, long int *nwaves, 
													PCM_fmt_chnk *fmt_chunk );
													
int parse_modulator( struct wave_node *node, char *line, unsigned long depth, 
							long int *nlines, PCM_fmt_chnk *format );

float modulate_waveform( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );


/*The oscillators*/

/*
NB - these need to all have the 
same arguments as each other
*/

float sin_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float sqr_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float sqx_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float tri_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float sn3_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float sn5_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );
float rnd_wave( struct wave_node *node, PCM_fmt_chnk *fmt_chunk, long int pos );


/*Error functions*/
void err_bad_line_format( long int l );

/*String functions*/
char is_string_blank( char *s );
char strip_comments( char *l, char c );
char get_first_string_element( char *line, char *element );
int	 pad_char_in_str_with_char( char *string, char target, char pad, int max_str_len );

/*variable parsing*/
struct variable_node *vnalloc( void );
struct variable_node * build_variable_list( void );
int substitute_variables( char *s,  struct variable_node *var_node );
char * get_var_value( char *key, struct variable_node *var_node );
int assign_variables( char *s,  struct variable_node *var_node );
void print_var_table( struct variable_node *var_node );


/*parsing tools*/
int count_open_close_pairs_in_string( char *string, char open_ch, char close_ch );
int chop_out_bracketed_from_string( char *string, char open_ch, char close_ch );
int chop_out_first_bracketed_from_string( char *string, char open_ch, char close_ch );
int extract_bracketed_from_string( char *string, char open_ch, char close_ch );
int extract_first_bracketed_from_string( char *string, char open_ch, char close_ch );

/*Decleration for debugging functions*/
void print_wnode( struct wave_node *wnode, PCM_fmt_chnk *format );
void print_data( struct wave_node *wnode, int depth );

