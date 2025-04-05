from scipy.io import wavfile
import subprocess as subproc
import io

import matplotlib.pyplot as plt

args=[]
args.append("../blow_chunks")
args.append("-d")
args.append("0.024")

with open("x.txt", "r") as f, open("x.wav", "wb") as g:
    p = subproc.Popen(args, stdin=f, stdout=subproc.PIPE)
    output=p.communicate()[0]
    g.write(output)

samplerate, data = wavfile.read("x.wav")


plt.plot(data)
plt.show()
