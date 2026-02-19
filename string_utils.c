#include<chunky.h>

/*Strip comments from input data*/
char    strip_comments( char *l, char c ){

    while( *l != '\0' ){
        if( *l == c ){
             *l = '\0' ;
            return 0 ;
        }
        l++ ;    
    }    
    return 1 ;
}

/*
** -------------------------
** pad_char_in_str_with_char
** -------------------------
*//*
Surrounds any occurance of a given char in string 
with a second specified char. The max string length
is required to try and stop accidental overflows.
v1.0 (13/10/02)
*/

int   pad_char_in_str_with_char( string, target, pad, max_str_len )
char  *string;
char  target;
char  pad;
int   max_str_len;
{

    char *tmp_str;
    int  ret_val=0;
    int  i, j=0;
            
    if( ( tmp_str = (char *) malloc( sizeof( char ) * max_str_len ) ) == NULL ){
        fprintf( stderr, "unable to allocate memory in function pad_char_in_str_with_char\n" );
        exit( EXIT_FAILURE );
    }
    
    strcpy( tmp_str, string );
        
    for( i=0; i<max_str_len; i++ ){
        
        /*if we hit the end of the string exit*/
        if( *( tmp_str + i ) == '\0') {
            *( string + j ) = '\0';
            break ;
        }
                    
        /*if we hit the max lentgh exit with -1*/
        if( j++ >= max_str_len ){
            ret_val = -1 ;
            break ;
        }  
                        
        /*if we find a match with target do padding*/
        /*if not just copy over the current char*/
        if( *( tmp_str + i ) == target ){
                
            if( (j+=2 ) >= max_str_len ){
                ret_val = -1 ;
                break ;
            } 
                        
            ret_val++ ;
            
            *( string + j - 3 ) = pad ;
            *( string + j - 2 ) = target ;
            *( string + j - 1 ) = pad ;
    
        }else{
                
            *( string + j - 1 ) = *( tmp_str + i ) ;
        
        }
    
    }

    free( tmp_str );
    return( ret_val );

}     

/*
** --------------------------------
** count_open_close_pairs_in_string
** --------------------------------
*//*
This function is a TEMPORARY *tool* and will
change in the final release of version 0.7

exit codes:
0: none of the specified chars were found
>1: this number of pairs were found
-1: a close was found before a matching open
-2: more opens than closes were found


*/
int count_open_close_pairs_in_string( string, open_ch, close_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
{

    int      exit_code=0;
    int        n_open=0;
    int        n_close=0;
    
    char     tmp[ MAX_LINE_LEN ], str[ MAX_LINE_LEN ];
    
    /*copy string so we don't destroy it*/
    
    strcpy( str, string );
    
    
    pad_char_in_str_with_char( str, open_ch, ' ', MAX_LINE_LEN-100 );
    pad_char_in_str_with_char( str, close_ch, ' ', MAX_LINE_LEN-100 );
    
    while( get_first_string_element( str, tmp ) ){
    
        if( open_ch == *tmp )
            n_open++;
        else if( close_ch == *tmp )
            n_close++;
            
        if( n_close > n_open )
            return( -1 ) ;
            
    }

    if( n_open > n_close ) exit_code=-2 ;
    else exit_code=n_open ;

    return( exit_code );
}


/*
** -------------------------------------
** replace_bracketed_in_string_with_char
** -------------------------------------
*//*
Chops out any thing from a string which is contained between
an opening and a closing char and then replace it with a
specified character.

Return values:
>=0: the return value represents the number of bracket pairs
-1: badly placed or too many closing brackets
-2: more open brackets than close brackets

*/
int     replace_bracketed_in_string_with_char( string, open_ch, close_ch, repl_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
char    repl_ch ;
{

    int      exit_code=0;
    int      n_open=0;
    int      n_close=0;
    int      j=0, i=0;
    char     tmp[ MAX_LINE_LEN ]; 
    
    strcpy( tmp, string );
    
    while( *( tmp + j ) != '\0' ){
    
        if( *( tmp + j ) == open_ch ){ 
            *( string + i++ ) = repl_ch;
            n_open++ ; 
            j++;
        }
    
        /*copy string if outside brackets*/
        if( n_open == n_close )
            *( string + i++ ) = *( tmp + j ) ;
            
        if( *( tmp + j ) == close_ch ) n_close++ ; 
    
        /*if n_close is greater than n_open
        the input line is badly formed*/
        if( n_close > n_open ){
            exit_code = -1 ;
            i--;
            break ;
        }             
        j++;    
    }
    
    /*NULL terminate string*/
    *( string + i ) = '\0' ;
    
    if( n_open == n_close ) exit_code = n_open ;
    else if( n_open > n_close ) exit_code = -2 ;
    
    return( exit_code );
}






/*
** ------------------------------
** chop_out_bracketed_from_string
** ------------------------------
*//*
Chops out any thing from a string which is contained between
an opening and a closing char. This is useful for parsing
text with brackets in it.

Return values:
>=0: the return value represents the number of bracket pairs
-1: badly placed or too many closing brackets
-2: more open brackets than close brackets

*/
int     chop_out_bracketed_from_string( string, open_ch, close_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
{

    int      exit_code=0;
    int      n_open=0;
    int      n_close=0;
    int      j=0, i=0;
    char     tmp[ MAX_LINE_LEN ]; 
    
    strcpy( tmp, string );
    
    while( *( tmp + j ) != '\0' ){
    
        if( *( tmp + j ) == open_ch ) n_open++ ; 
    
        /*copy string if outside brackets*/
        if( n_open == n_close )
            *( string + i++ ) = *( tmp + j ) ;
            
        if( *( tmp + j ) == close_ch ) n_close++ ; 
    
        /*if n_close is greater than n_open
        the input line is badly formed*/
        if( n_close > n_open ){
            exit_code = -1 ;
            i--;
            break ;
        }             
        j++;    
    }
    
    /*NULL terminate string*/
    *( string + i ) = '\0' ;
    
    if( n_open == n_close ) exit_code = n_open ;
    else if( n_open > n_close ) exit_code = -2 ;
    
    return( exit_code );
}


/*
** ------------------------------------
** chop_out_first_bracketed_from_string
** ------------------------------------
*//*
Chops out the first element from a string which is contained between
an opening and a closing char. This is useful for parsing
text with brackets in it.

Return values:
>=0: the return value represents the number of bracket pairs
-1: badly placed or too many closing brackets
-2: more open brackets than close brackets

*/
int     chop_out_first_bracketed_from_string( string, open_ch, close_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
{

    int      exit_code=0;
    int        n_open=0;
    int        n_close=0;
    int        j=0, i=0;
    char     tmp[ MAX_LINE_LEN ]; 
    
    
    strcpy( tmp, string );
    
    while( *( tmp + j ) != '\0' ){
    
        if( *( tmp + j ) == open_ch ) n_open++ ; 

        /*copy string if outside brackets*/
        if( n_open == n_close )
            *( string + i++ ) = *( tmp + j ) ;
            
        if( *( tmp + j ) == close_ch ) n_close++ ; 
    
        /*if n_open > 0 and n_open==n_close
        then we have passed the first set*/
        
        if( ( n_open > 0 ) && ( n_open == n_close ) ){
            j++ ;
            break;
        }
    
        /*if n_close is greater than n_open
        the input line is badly formed*/
        
        if( n_close > n_open ){
            exit_code = -1 ;
            i--;
            break ;
        } 
            
            
        j++;
    
    }
    
    /*NULL terminate string*/
    /* *( string + i ) = '\0' ;*/
    
    while( ( *( string + i++ ) = *( tmp + j++ ) ) != '\0');
    
    if( n_open == n_close ) exit_code = n_open ;
    else if( n_open > n_close ) exit_code = -2 ;
    
    return( exit_code );
}




/*
** ------------------------------
** extract_bracketed_from_string
** ------------------------------
*//*
Extracys anything from a string which is contained between
an opening and a closing char. This is useful for parsing
text with brackets in it.

Return values:
>=0: the return value represents the number of bracket pairs
-1: badly placed or too many closing brackets
-2: more open brackets than close brackets

*/
int     extract_bracketed_from_string( string, open_ch, close_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
{

    int      exit_code=0;
    int        n_open=0;
    int        n_close=0;
    int        j=0, i=0;
    char     tmp[ MAX_LINE_LEN ]; 
    
    
    strcpy( tmp, string );
    
    while( *( tmp + j ) != '\0' ){

        if( *( tmp + j ) == close_ch ) n_close++ ; 
    
        /*if n_close is greater than n_open
        the input line is badly formed*/
        
        if( n_close > n_open ){
            exit_code = -1 ;
            break ;
        } 
        
        /*copy string if inside brackets*/
        if( n_open != n_close )
            *( string + i++ ) = *( tmp + j ) ;
            
        if( *( tmp + j ) == open_ch ) n_open++ ; 
    
                        
        j++;
    
    }
    
    /*NULL terminate string*/
    *( string + i ) = '\0' ;
    
    if( n_open == n_close ) exit_code = n_open ;
    else if( n_open > n_close ) exit_code = -2 ;
    
    return( exit_code );
}

/*
** -----------------------------------
** extract_first_bracketed_from_string
** -----------------------------------
*//*
Extracys anything from a string which is contained between
an opening and a closing char. This is useful for parsing
text with brackets in it.

Return values:
>=0: the return value represents the number of bracket pairs
-1: badly placed or too many closing brackets
-2: more open brackets than close brackets

*/
int     extract_first_bracketed_from_string( string, open_ch, close_ch )
char    *string ;
char    open_ch ;
char    close_ch ;
{

    int      exit_code=0;
    int        n_open=0;
    int        n_close=0;
    int        j=0, i=0;
    char     tmp[ MAX_LINE_LEN ]; 
    
    
    strcpy( tmp, string );
    
    while( *( tmp + j ) != '\0' ){

        if( *( tmp + j ) == close_ch ) n_close++ ; 
    
        /*if n_close is greater than n_open
        the input line is badly formed*/
        
        if( n_close > n_open ){
            exit_code = -1 ;
            break ;
        } 
        
        if( ( n_open > 0 ) && ( n_open == n_close ) ){
            break ;
        }
        
        /*copy string if inside brackets*/
        if( n_open != n_close )
            *( string + i++ ) = *( tmp + j ) ;
            
        if( *( tmp + j ) == open_ch ) n_open++ ; 
    
                        
        j++;
    
    }
    
    /*NULL terminate string*/
    *( string + i ) = '\0' ;
    
    if( n_open == n_close ) exit_code = n_open ;
    else if( n_open > n_close ) exit_code = -2 ;
    
    return( exit_code );
}



/*
** ------------------------
** get_first_string_element
** ------------------------
*//*
Gets the first group of non whitespace characters,
puts them into element and removes them from the
original string.

version 1.1 bug fixed by adding null termination 
see (*) below.

[NB - this was written for an app at work which
is why it probably looks a damn sight smarter
then a lot of the rest of blow chunks!]
*/

char    get_first_string_element( line, element )
char    *line;
char    *element;
{

    int    i=0, j=0;

    /* Check that we are not already */
    /* at the end of the line        */

    if( *line == '\0' ) return( 0 );
    
    /* Skip leading white space and */
    /* return zero if end of string */
    /* is encountered               */
    
    while( isspace( *(line+i) ) ) 
        if( *(line+( ++i )) == '\0' ) return( 0 );
    
    
    /* Put the first block of non white 
       space characters into element */
    
    while( ! isspace( *(line+i) ) ){
        if( ( element[j++] = *(line+(i++) ) ) == '\0' ){
        
            *line = '\0';
            return( 1 );
        
        }
    }
    
    
    /*put a NULL terminator in at j - (*) */
    
    element[ j ] = '\0' ;
    
    /* Copy remainder of line into the beggining */
    /* of itself */
 
    for( j=0; ( *(line+j)=*(line+i) ) != '\0' ; ++j, ++i );
        
    return( 1 );

}



/*Find if string is empty*/
/*Read K&R section 5.5*/

char is_string_blank( char *s )
{
    
    while( *s != '\0' )
        if( ! isspace( *s++ ) ) 
            return 0 ;
                
    return 1 ;

}



int substitute_variables( char *s,  struct variable_node *var_node )
{

    int n_subs=0, i=0, j=0, k=0;
    char *var_value;
    char *s_orig=s;
    char var_name[ MAX_LINE_LEN ];
    char new_s[ MAX_LINE_LEN ];

    while( *s!='\0' ){
        if( *s=='$' ){
            while( isalnum( *++s ) )
                var_name[i++]=*s;
            
            var_name[i]='\0';
            n_subs++;
            i=0;
            var_value=get_var_value( var_name, var_node );
            if( var_value==NULL){
                fprintf(stderr,"exiting: undefined variable: %s\n", var_name);
                exit(1);
            }
            while( var_value[k] != '\0')
                new_s[j++]=var_value[k++];
            k=0;
        }else{
            new_s[j++]=*s++;
        }
    }      
    new_s[j]='\0';
    strcpy(s_orig,new_s);
    return n_subs ;
}


char * get_var_value( char *key,  struct variable_node *var_node ){    
    while( var_node!=NULL ){
        if (strcmp(var_node->key, key) == 0) 
            return var_node->value;
        var_node=var_node->next;
    }
    return NULL;
}


void print_var_table( struct variable_node *var_node ){    
    while( var_node!=NULL ){
        fprintf(stderr,"%s %s\n",var_node->key,var_node->value);
        var_node=var_node->next;
    }
}



int assign_variables( char *string,  struct variable_node *var_node )
{

    char token1[ MAX_LINE_LEN ];
    char token2[ MAX_LINE_LEN ];
    char tmp[ MAX_LINE_LEN ];
    int  i, k;
    
    struct variable_node *last_node;
        
    pad_char_in_str_with_char( string, '=', ' ', MAX_LINE_LEN-100 );
    
    if( get_first_string_element( string, token1 )==0 ) return 0;
    if( get_first_string_element( string, token2 )==0 ) return 0;

    if( strcmp(token2,"=") != 0) return 0;
    
    /*check here whether token1 is valid var name*/
    strcpy(tmp, token1);
    i=0;
    while( tmp[i] != '\0' ){
        if(isalnum( tmp[i++] )==0){
            fprintf(stderr,"exiting. badly formed variable name: %s\n",token1);
        }
    }

    /*created a left trimmed version of the remainder of the string*/
    strcpy(tmp, string);
    i=k=0;
    while( isspace(tmp[i++]) ) continue;
    while( tmp[i-1] != '\0' ){
        string[k++]=tmp[i-1];  
        i++;
    }

    fprintf(stderr,"possible problem in variable parsing in %s at line %d\n",__FILE__,__LINE__); 
    string[k-2]='\0';
    
    /*check if var exists*/
    /*and, if so, replace*/
    while( var_node!=NULL ){
        if (strcmp(var_node->key, token1) == 0){ 
            strcpy(var_node->value, string);
            return 1;
        }
        last_node=var_node;
        var_node=var_node->next;
    }

    /*if here then add new var*/
    last_node->next=vnalloc();
    var_node=last_node->next;
    var_node->next=NULL;   
    var_node->key=(char*) malloc(MAX_LINE_LEN*sizeof(char));
    var_node->value=(char*) malloc(MAX_LINE_LEN*sizeof(char));
    strcpy(var_node->key, token1);
    strcpy(var_node->value, string);

    //fprintf(stderr,"%s %d check node: |%s| |%s|\n",__FILE__,__LINE__,var_node->key,var_node->value);        

    return 1;
}



