#!/bin/gawk -f 

BEGIN{
    nwaves=50
    basef=55
    detune=1.
    durat=7
    vol=0.8
    phase=1.0
    
    printf("@sequence 0 %f\n",durat)
    for(i=1;i<=nwaves;i++){
        freq=basef+(rand()-0.5)*detune
        panr=rand()*vol
        panl=1-panr
        
        #randomising the phase stops
        #initial transients. It is
        #cool with and without
        phase=rand()
        
        printf("sup %f %f <0:0 2:%f %f:%f %f:0> <0:0 2:%f %f:%f %f:0>\n",
                freq,phase,panr,durat-2,panr,durat,panl,durat-2,panl,durat)
    }

}
