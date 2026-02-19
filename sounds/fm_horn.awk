#!/bin/gawk -f 

function abs(x){
    if(x<0) return x*-1.
    return x
}

BEGIN{
    note=220
    detune=0.1
    nwaves=10

    for(i=1;i<=nwaves;i++){
        a=(rand()-0.5)
        f=note+a*detune
        phase=rand()

        printf("trx %f %f %f \n",f,phase,1-abs(a))
    
    }

}
