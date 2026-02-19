from scipy.io import wavfile
import subprocess as subproc
import io
import sys

import matplotlib.pyplot as plt

def mk_fig(bc_command,bc_command2=None,freq=200,n_show=5):

    #set up the blow_chunks command
    args=[]
    args.append("../blow_chunks")
    args.append("-d")
    args.append("0.5")

    #use subprocess to generate the wav date
    p = subproc.Popen(args, stdin=subproc.PIPE, stdout=subproc.PIPE)
    p.stdin.write(bc_command.encode("ascii"))
    wav = io.BytesIO(p.communicate()[0])
    samplerate, data = wavfile.read(wav)

    if bc_command2 is not None:
        #use subprocess to generate the wav date
        p = subproc.Popen(args, stdin=subproc.PIPE, stdout=subproc.PIPE)
        p.stdin.write(bc_command2.encode("ascii"))
        wav2 = io.BytesIO(p.communicate()[0])
        _, data2 = wavfile.read(wav2)


    #plotting
    samps_per_wave=samplerate/freq
    n_skip=3 #avoid the fast faders
    
    plt.figure(figsize=(10, 4), dpi=100)
    plt.plot(data[int(samps_per_wave*n_skip):int(samps_per_wave*(n_skip+n_show))],'k-')
    if bc_command2 is not None:
        plt.plot(data2[int(samps_per_wave*n_skip):int(samps_per_wave*(n_skip+n_show))],'r--')

    plt.title(bc_command)
    plt.xlabel("samples")
    plt.savefig(bc_command.replace(" ","_").replace(".","p").replace("{","-").replace("}","-")+".png")
    
if __name__=="__main__":

     freq="220"
     oscils=["sin","sup","sdn", "sn3","sn5","sqr","sqx","sut","tri"]     
     oscils=["tri","trx"]     

     #basic oscillator plots
     for osc in oscils:
         bc_command="%s %s 1 1"%(osc,freq)
         mk_fig(bc_command,freq=float(freq))
             

     #amp mod plots
     for osc in oscils:
         bc_command="sin 220 1 1 {%s 5 1 1}"%(osc)
         bc_command2="%s 5 1 1"%(osc)
         mk_fig(bc_command,bc_command2=bc_command2,freq=float(freq),n_show=200)
             




