# sinustabelle generieren

import math
from gentab import generateTabelle


sinustabelle= []

'''
als verstaerker wird ein lm324 mit nur 5v- versorgung als spannungsfolger verwendet.
der lm324 kann bei 5v nur bis ca. 3.4 Volt ( v_max ) nach oben und bis 0.6 Volt
( v_min ) nach unten ausgesteuert werden ( gemessene werte). daraus ergeben sich maximum und
minimum fuer die pwm-werte.
'''
v_ref = 5.0

v_max = 3.4
v_min =.6
pwm_ref = 256
pwm_max = v_max * pwm_ref/v_ref
pwm_min = v_min * pwm_ref/v_ref
pwm_delta = pwm_max - pwm_min
pwm_amplitude = (pwm_delta)/2
pwm_offset = pwm_min + pwm_amplitude
offset = int(pwm_offset)   # igentwo in der mitte vom bereich 0.. 255
faktor=1.0
amplitude = int( faktor * pwm_amplitude )

#sinus liste berechnen
phi = 0
n_sinuswerte=256
step= 2 * math.pi / n_sinuswerte  
for i in range ( 0, n_sinuswerte ):     
    y= math.sin( phi )
    phi = phi + step
    sinustabelle.append ( offset + int( pwm_amplitude * y ))

#output
generateTabelle("sinus","sinus.h", sinustabelle)
# generateTabelle( 'tabellenname', "filename",array, typ="const unsigned char", memory='PROGMEM')

