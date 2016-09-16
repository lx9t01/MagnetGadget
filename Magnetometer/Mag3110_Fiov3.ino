/*
  Code sample from MAG3110 Breakout Example Code
  by: Aaron Weiss, aaron at sparkfun dot com
      
  Original code reads the raw 16-bit x, y, and z values and prints them 
  out. INT1 pin left open. 

  Updated version reads magnet amplitude in uT from 3 axis and total value, 
  stores the data into a txt file with timestamp. 
  by Haixiang Xu
*/

#include <Wire.h>

#define MAG_ADDR  0x0E //7-bit address for the MAG3110, doesn't change
int RXLED = 17;
long t = 0;
int delay_ms = 20; // 50 Hz sampling rate
int bias_x = 0;
int bias_y = 0;
int bias_z = 0;

void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
  config();            // turn the MAG3110 on
  delay(5000);
  adjust();            // software calibrating
}


void loop()
{
  print_values(t);
  delay(delay_ms);
  t += delay_ms; // max 2,147,483,647 ms
  digitalWrite(RXLED, HIGH);
}

//----------------------------------------------------------------------------
void config(void)
{
  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x11);             // cntrl register2
  Wire.write(0x80);             // write 0x80, enable auto resets, write 0xA0, raw = 1
  Wire.endTransmission();       // stop transmitting
  
  delay(15);
  
  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(0x10);             // cntrl register1
  Wire.write(0x01);                // write 0x01, active mode
  Wire.endTransmission();       // stop transmitting
}

/*
 * first 5 seconds as adjusting period, sample 100 points 
 */
void adjust(void) {
  int sample_interval = 100;
  pinMode(RXLED, OUTPUT);
  long x_sum = 0;
  long y_sum = 0;
  long z_sum = 0;
  
  for (int i = 0; i < 10000 / sample_interval; ++i) { // 10s of calibrating period
    digitalWrite(RXLED, LOW);
    delay(sample_interval);
    x_sum += read_x();
    y_sum += read_y();
    z_sum += read_z();

    digitalWrite(RXLED, HIGH);
    delay(sample_interval);
    x_sum += read_x();
    y_sum += read_y();
    z_sum += read_z();
  }
  bias_x = (int)(x_sum / (20000 / sample_interval));
  bias_y = (int)(y_sum / (20000 / sample_interval));
  bias_z = (int)(z_sum / (20000 / sample_interval));
  Serial.println(bias_x);
  Serial.println(bias_y);
  Serial.println(bias_z);
}


void print_values(long t)
{
  Serial.print(t);
  Serial.print(" ");
  double vx = (double)(read_x() - bias_x) / 10.0; // 0.1 uT/LSB refer to data sheet for details
  Serial.print(vx);
  Serial.print(" ");  
  double vy = (double)(read_y() - bias_y) / 10.0;  
  Serial.print(vy);
  Serial.print(" ");       
  double vz = (double)(read_z() - bias_z) / 10.0;  
  Serial.print(vz);
  double total = sqrt(vx * vx + vy * vy + vz * vz);
  Serial.print(" ");
  Serial.print(total);
  Serial.println();
}

int mag_read_register(int reg)
{
  int reg_val;
  
  Wire.beginTransmission(MAG_ADDR); // transmit to device 0x0E
  Wire.write(reg);              // x MSB reg
  Wire.endTransmission();       // stop transmitting
 
  delayMicroseconds(2); //needs at least 1.3us free time between start and stop
  
  Wire.requestFrom(MAG_ADDR, 1); // request 1 byte
  while(Wire.available())    // slave may write less than requested
  { 
    reg_val = Wire.read(); // read the byte
  }
  
  return reg_val;
}

int mag_read_value(int msb_reg, int lsb_reg)
{
  int val_low, val_high;  //define the MSB and LSB
  
  val_high = mag_read_register(msb_reg);
  
  delayMicroseconds(2); //needs at least 1.3us free time between start and stop
  
  val_low = mag_read_register(lsb_reg);
  
  int out = (val_low|(val_high << 8)); //concatenate the MSB and LSB
  return out;
}

int read_x(void)
{
  return mag_read_value(0x01, 0x02);
}

int read_y(void)
{
  return mag_read_value(0x03, 0x04);
}

int read_z(void)
{
  return mag_read_value(0x05, 0x06);
}
