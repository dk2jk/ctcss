#ifndef __si5351_lite__H
#define __si5351_lite__H

class si5351_lite
{
#define BB0(x) ((unsigned char )x)             // Bust int32 into Bytes
#define BB1(x) ((unsigned char )(x>>8))
#define BB2(x) ((unsigned char )(x>>16))

#define ADDR 0x60              // I2C address of Si5351   (typical)
#define XTALPF 3               // 1:6pf  2:8pf  3:10pf

    // If using 27mhz crystal, set XTAL=27000000, MSA=33.  Then vco=891mhz
#define XTAL 27005000          // Crystal freq in Hz
#define MSA  33                // VCOA is at 25mhz*35 = 875mhz

  public:

    void init();
    void setfreq(unsigned char clknum, unsigned long fout);
    void setDrive( unsigned char clknum, unsigned char driveIndex); 
  private:
    unsigned long vcoa = (XTAL*MSA);  // 25mhzXtal calibrate
    unsigned char   rdiv = 0;             // 0-7, CLK pin sees fout/(2**rdiv)
    unsigned char   drive[3] = {0, 0, 0}; // 0=2ma 1=4ma 2=6ma 3=8ma for CLK 0,1,2
    unsigned char   clken = 0xFF;         // Private, all CLK output drivers off
    void i2cWrite(unsigned char reg, unsigned char val) ;
    void i2cWriten(unsigned char reg, unsigned char *vals, unsigned char vcnt);
    unsigned char   i2cRead(int regaddr);
 
};

#endif //__si5351_lite__H 
