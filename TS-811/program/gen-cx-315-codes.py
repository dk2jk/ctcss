# cx-315 codes


file="cx-315.csv"
file2="cx_315_v1.h"

#datei lesen
f= open (file)
text= f.readlines()
f.close()

# datei schreiben
f= open (file2, "w")

#1. zeile kopftext
f.write("/********** "+file2+" ****/\n\n" )



def gen_data(s="671.0,1,1,1,1,1,1"):
    x= s.split(',')
    y1= float(x[0])
    c=''
    for i in range (1,7):
        c=c+x[i]
    c= int(c,2)

    return y1,c

def gen_bin(n=61):
    s= "{:b}".format(n).rjust(6,'0')
    return s
    
def gen_zeile(s=(671.0,1)):
    y1=str(s[0])
    y2=str(s[1])
    return '{'+y1+',\t'+ y2 + '},\t// '+ gen_bin(s[1])  +'\n' 

#liste mit sortierten daten
data=[]
for j in text[1:]:
    data.append( gen_data(j))
sorted_by_second = data #sorted(data, key=lambda tup: tup[1])

kopf= "struct s_frequenztabelle{\n float fq; byte code; };\n"
f.write(kopf)
len=str(len(data))
f.write("#define TABELLENLAENGE " + len +'\n')
f.write("s_frequenztabelle frequenztabelle[TABELLENLAENGE]= {\n")
f.writelines('//fq,\ti, \t    //[5....0] bin\n')

for j in sorted_by_second:
    f.writelines(gen_zeile(j))
f.write("};\n" )
f.close()




