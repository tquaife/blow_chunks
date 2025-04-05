#include<chunky.h>

float op_mul(float a, float b)
{return a*b;}

float op_div(float a, float b)
{return a/b;}

float op_sum(float a, float b)
{return a+b;}

float op_sub(float a, float b)
{return a-b;}

typedef struct operator
{
    char symbol;
    float (* func)(float, float);
} operator;

static const operator op_lut[] =
{
    { '*', op_mul},
    { '/', op_div},
    { '+', op_sum},
    { '-', op_sub},
    //terminate the list:
    { '\0', NULL},
};

float(* get_op_func(char key))(float, float){    
     int i=0;
     while(op_lut[i].symbol!='\0'){
         if(op_lut[i].symbol==key)
             return op_lut[i].func;
         i++;
     }
     return NULL;
}

int parse_maths(string)
char *string;
{
    char tmpstr[ MAX_LINE_LEN ];
    char tokens[ 100 ][ MAX_LINE_LEN ];
 
    /*extend if other chars get used in blow_chunks*/
    char pad_me[]="{}<>=";
    /*must be in operator order:*/
    char op_syms[]="*/+-";
    
    int i, j, k, ntokens, nops;
    float a, b;
    char *x;
    char symbol[]="x";
    
    float (* func)(float, float);

    strcpy( tmpstr, string ) ;

    /* First, pad out characters so we can 
    tokenise the string for processing*/
    i=0;
    while( *(pad_me+i) != '\0')
        pad_char_in_str_with_char( tmpstr, *(pad_me+i++), ' ', MAX_LINE_LEN );
    i=0;
    while( *(op_syms+i) != '\0')
        pad_char_in_str_with_char( tmpstr, *(op_syms+i++), ' ', MAX_LINE_LEN );
    
    /*tokenise the string*/
    ntokens=0;
    while( get_first_string_element(tmpstr,tokens[ntokens])) ntokens++;
    
    k=0;
    while( *(op_syms+k) != '\0'){
        symbol[0]=*(op_syms+k++);
        func=get_op_func(symbol[0]);
        do{
            nops=0;
            for(i=0;i<ntokens;i++){
               if(strcmp(tokens[i],symbol)==0){
                   if(i==0){
                       fprintf(stderr,"error: math operator found at start of string. exiting\n");
                       exit(EXIT_FAILURE);
                   }
                   a=(float)strtod(tokens[i-1], &x);
                   if(*x!=0){
                       fprintf(stderr, "error: could not read floating point number: %s\n", tokens[i-1]);
                        exit(EXIT_FAILURE);
                  } 
                   b=(float)strtod(tokens[i+1], &x);
                   if(*x!=0){
                       fprintf(stderr, "error: could not read floating point number: %s\n", tokens[i+1]);
                        exit(EXIT_FAILURE);
                  } 
                  sprintf(tokens[i-1],"%f",func(a,b));
                  for(j=i;j<(ntokens-1);j++) strcpy( tokens[j],tokens[j+2] );
                  nops++;
                  ntokens-=2;
               }
            }
        }while(nops>0);
    }

    /*Rebuild the input line */
    j=0;
    for(i=0;i<ntokens;i++){ 
        k=0;
        while(tokens[i][k]!='\0')
             string[j++]=tokens[i][k++];
        string[j++]=' ';
    }
    string[j]='\n';
    string[j+1]='\0';


    return 0;
}



