#!/usr/bin/gawk -f

BEGIN{

    ref=27.50 #A0

    notes["A"]=0
    notes["As"]=1
    notes["Bb"]=1
    notes["B"]=2
    notes["C"]=-9
    notes["Cs"]=-8
    notes["Db"]=-8
    notes["D"]=-7
    notes["Ds"]=-6
    notes["Eb"]=-6
    notes["E"]=-5
    notes["F"]=-4
    notes["Fs"]=-3
    notes["Gb"]=-3
    notes["G"]=-2
    notes["Gs"]=-1
    notes["Ab"]=-1
  

    for( oct=0;oct<=8;oct++){
        for( n in notes){
            print "{ ""\""n""oct"\", ","\""ref * 2 ^ ((notes[n]+oct*12) / 12)"\"},"
        }
    }
}
