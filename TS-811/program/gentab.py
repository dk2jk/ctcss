import os
from time import ctime

    
def generateTabelle( tabellenname ='name', filename="filename",array=[1,2,3], typ="const unsigned char", memory='PROGMEM'):
    myname = os.path.basename(__file__)
    f = open(filename,'w')
    Header =  "//**** "+ filename +" ****\n" + "//**** generiert durch '"+ myname +"\n//**** "+ctime() +"'\n"
    size = len(array)
    sizeName = tabellenname.upper()+'_SIZE '
    Size = "#define "+ sizeName  + str(size) +'\n' 
    f.write( Header + Size)
    f.write(typ + ' '+ tabellenname + "[ "+ sizeName +"] " + memory +" = {\n")
    for i in range (0,size):
        f.write( str(array[i]) + ',')
        rest = i % 16
        if rest == 15:
            f.write('\n')
    f.write(" };\n\n")
    f.close()
                
