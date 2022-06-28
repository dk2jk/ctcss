
#include "pinDefinitions.h"
#include "si5351_lite.h"

#include "SoftI2CMaster.h"
SoftI2CMaster myI2C(SCL,SDA);
/*
 * durch SoftI2CMaster kann der i2c bus auf beliebige pins
 * gelegt werden oder macht fuer cpu's ohne i2c-hardware
 * i2c erst moeglich.
 */

void si5351_lite::i2cWrite(unsigned char reg, unsigned char val) {   // write reg via i2c
  myI2C.beginTransmission(ADDR);
  myI2C.write(reg);
  myI2C.write(val);
  myI2C.endTransmission();
}

void si5351_lite::i2cWriten(unsigned char reg, unsigned char *vals, unsigned char vcnt) {  // write array
  myI2C.beginTransmission(ADDR);
  myI2C.write(reg);
  while (vcnt--) myI2C.write(*vals++);
  myI2C.endTransmission();
}


unsigned char   si5351_lite::i2cRead(int regaddr) {       // read one i2c reg
  unsigned char rval;
  myI2C.beginTransmission(ADDR);
  myI2C.write(regaddr);
  myI2C.endTransmission();
  myI2C.requestFrom(ADDR, 1);
  rval = myI2C.read();
  return rval;
}


void si5351_lite::init() {                  // Call once at power-up, start PLLA
  unsigned char reg;  unsigned long msxp1;
  myI2C.begin();
  i2cWrite(149, 0);                   // SpreadSpectrum off
  i2cWrite(3, clken);        // Disable all CLK output drivers
  i2cWrite(183, XTALPF << 6); // Set 25mhz crystal load capacitance
  msxp1 = 128 * MSA - 512;   // and msxp2=0, msxp3=1, not fractional
  unsigned char  vals[8] = {0, 1, BB2(msxp1), BB1(msxp1), BB0(msxp1), 0, 0, 0};
  i2cWriten(26, vals, 8);             // Write to 8 PLLA msynth regs
  // for (reg=16; reg<=23; reg++) i2cWrite(reg, 0x80);    // Powerdown CLK's
  // i2cWrite(187, 0);                // No fannout of clkin, xtal, ms0, ms4
  i2cWrite(165, 0);
  i2cWrite(166, 127);
  i2cWrite(177, 0x20);                // Reset PLLA  (0x80 resets PLLB)
}

void si5351_lite::setfreq(unsigned char clknum, unsigned long fout) {  // Set a CLK to fout Hz
  unsigned long  msa, msb, msc, msxp1, msxp2, msxp3p2top;
  if ((fout < 500000) || (fout > 109000000)) // If clock freq out of range
    clken |= 1 << clknum;      //  shut down the clock
  else {
    msa = vcoa / fout;     // Integer part of vco/fout
    msb = vcoa % fout;     // Fractional part of vco/fout
    msc = fout;             // Divide by 2 till fits in reg
    while (msc & 0xfff00000) {
      msb = msb >> 1;
      msc = msc >> 1;
    }
    msxp1 = (128 * msa + 128 * msb / msc - 512) | (((unsigned long)rdiv) << 20);
    msxp2 = 128 * msb - 128 * msb / msc * msc; // msxp3 == msc;
    msxp3p2top = (((msc & 0x0F0000) << 4) | msxp2);     // 2 top nibbles
    unsigned char vals[8] = { BB1(msc), BB0(msc), BB2(msxp1), BB1(msxp1),
                              BB0(msxp1), BB2(msxp3p2top), BB1(msxp2), BB0(msxp2)
                            };
    i2cWriten(42 + (clknum * 8), vals, 8); // Write to 8 msynth regs
    i2cWrite(16 + clknum, 0x0C | drive[clknum]); // use local msynth
    //					// drive strength :0x0c 2ma , 0x0d 4ma, 0x0e 6ma, 0x0f 8ma
    clken &= ~(1 << clknum);   // Clear bit to enable clock
  }
  i2cWrite(3, clken);        // Enable/disable clock
}

 void si5351_lite::setDrive( unsigned char clknum, unsigned char driveIndex)
 {
   drive[clknum] = driveIndex;
 }

