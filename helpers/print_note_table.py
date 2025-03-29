

C1_frq=32.70319568

note_order=["C","D","E","F","G","A","B"]

semitones={}

semitones["C"]=0
semitones["D"]=2
semitones["E"]=4
semitones["F"]=5
semitones["G"]=7
semitones["A"]=9
semitones["B"]=11

for octave in range(1,9):
    for note in note_order:
        for semi in [-1,0,1]:
            x=""
            if semi==-1:
                x="b"
            elif semi==1:
                x="s"
            note_str=note+x+"%d"%octave
            note_frq=C1_frq*2**((semitones[note]+semi+(octave-1)*12.)/12.)
            print(" {\"%s\",\"%0.5f\"},"%(note_str,note_frq))
