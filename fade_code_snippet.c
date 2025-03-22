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

