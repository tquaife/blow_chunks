#!/bin/gawk -f 

BEGIN{
    nwaves=20
    basef=220
    magnit=50
    durat=20
    step=1.0

    #sin gives a smooth but feeble detuning    
    wave="sin"

    #sqr is a bit bonkers    
    #wave="sqr"

    #sqr is a slightly smoother version
    #sqr (entirely expectedly)    
    #wave="sqx"

    
    printf("@sequence 0 %f\n",durat)
    for(i=1;i<=nwaves;i++){
        freq=basef+(rand()-0.5)*magnit
        phase=rand()
        printf("%s <",wave)
        for(t=0;t<durat; t+=step*3){
            printf("%f:%f %f:%f %f:%f ",t,basef,t+step,basef,t+step*2,freq)
        }
        printf("> %f 1\n",phase)
    
    }

}
