# blow_chunks

A simple, non-realtime, command line driven synthesiser. Makes nice noises.

On a Linux system with C development tools installed (gcc, make, etc) it should compile simply by typing "make".

A simple example is: 

`echo "sin 220 1 1" | ./blow_chunks > a220.wav`

A not much more complex, but much more dynamic example: 

`echo "sin 220 {sin 55 1 <0:1 1:15>} 1 1" | ./blow_chunks > a220_fsweep.wav`

More in the manual.



