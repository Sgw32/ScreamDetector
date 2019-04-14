#include "fix_fft.h"
#define FFTLEN  128
#define log2FFT   7
#define N         (2 * FFT_SIZE)
#define log2N     (log2FFT + 1)

// Store our compass as a variable.
HMC5883L compass;
STM32ADC myADC(ADC1);
UBLOX gps(Serial3,115200);

//Channels to be acquired. 
uint8_t analogPins[] = {PA2,PA3,PA4,PA5,PA6,PA7,PB0,PB1};
#define numPins 8
#define maxSamples 1024

uint16_t buffer[maxSamples];
uint16_t *buffers[2];
uint8_t bufr;
uint8_t bufw;

//SETUP data
uint8_t gainCoeffs[8] = {1,1,1,1,1,1,1,1};
uint16_t thresholdFFT = 300;
uint8_t meanPower = 10;
uint8_t FFTdata[256];
uint16_t upperFreq = 1000;
uint16_t lowerFreq = 200;

uint8_t curChannel = 0;

#define pinLED  PC13
#define pinOUT  PB0

#define sampleFreqKhz       20
#define samplePeriodus      1000 / sampleFreqKhz
#define ticksPerSecond      2 * sampleFreqKhz * 1000 / maxSamples

long ticks;

uint32_t y[FFTLEN];
uint16_t hammingwindow[FFTLEN/2];
uint16_t bins = FFTLEN;

void TimerIRQ(void) {
  ticks++;
}
void DmaIRQ(void) {
  bufw = (bufw+1)%2;
}

void startupBlink()
{
  pinMode(PC13, OUTPUT);
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(100);          // Wait for 1 second (1000 milliseconds)
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(100);          // Wait for 1 second (1000 milliseconds)
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(100);          // Wait for 1 second (1000 milliseconds)
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(100);          // Wait for 1 second (1000 milliseconds)
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(100);          // Wait for 1 second (1000 milliseconds)
}

void init_hamming_window(uint16_t * windowtarget, int len){
  for(int i = 0;i<len/2; i++){ windowtarget[i] = (0.54 - 0.46 * cos((2 * i * 3.141592)/(len-1))) * 65536; }
}
  
void window(uint32_t * data, uint16_t * weights, int len, int scale){
  for(int i =0; i<len; i++){
    int weight_index = i;
    if( i > len/2 ) weight_index = len-i;
    data[i] = ((data[i] * scale * weights[weight_index]) >> 16) & 0xFFFF;
  }
}

uint16_t asqrt(uint32_t x) { //good enough precision, 10x faster than regular sqrt
  /*      From http://medialab.freaknet.org/martin/src/sqrt/sqrt.c
   *   Logically, these are unsigned. We need the sign bit to test
   *   whether (op - res - one) underflowed.
   */
  int32_t op, res, one;

  op = x;
  res = 0;
  /* "one" starts at the highest power of four <= than the argument. */
  one = 1 << 30;   /* second-to-top bit set */
  while (one > op) one >>= 2;
  while (one != 0) {
    if (op >= res + one) {
      op = op - (res + one);
      res = res +  2 * one;
    }
    res /= 2;
    one /= 4;
  }
  return (uint16_t) (res);
}

void inplace_magnitude(uint32_t * target, uint16_t len){
  uint16_t * p16;
  for (int i=0;i<len;i++){
     int16_t real = target[i] & 0xFFFF;
     int16_t imag = target[i] >> 16;
     uint32_t magnitude = asqrt(real*real + imag*imag);
     target[i] = magnitude; 
	}
}

float bin_frequency(uint32_t samplerate, uint32_t binnumber, uint32_t len){
   return (binnumber*samplerate)/((float)len);
  }
  
void fill(uint32_t * data, uint32_t value, int len){
  for (int i =0; i< len;i++) data[i]=value;
}

  

void perform_fft(uint32_t * indata, uint32_t * outdata, int len){
	//window(indata,hammingwindow,len,8); //scaling factor of 4 for 4095> 16 bits
	//cr4_fft_1024_stm32(outdata,indata,len);
	//inplace_magnitude(outdata,len);
}

void readSetup()
{
  if (Serial1.available() > 0) 
  {
    Serial1.println("Found data.");
    int data = Serial1.read();
    if (data==0xAA)
    {
      gainCoeffs[0] = Serial1.read();
      gainCoeffs[1] = Serial1.read();
      gainCoeffs[2] = Serial1.read();
      gainCoeffs[3] = Serial1.read();
      gainCoeffs[4] = Serial1.read();
      gainCoeffs[5] = Serial1.read();
      gainCoeffs[6] = Serial1.read();
      gainCoeffs[7] = Serial1.read();  
      lowerFreq = 0 | ((uint16_t)Serial1.read()<<8);
      lowerFreq |= Serial1.read();
      upperFreq = 0 | ((uint16_t)Serial1.read()<<8);
      upperFreq |= Serial1.read();
      thresholdFFT = 0 | ((uint16_t)Serial1.read()<<8);
      thresholdFFT |= Serial1.read();
      meanPower = Serial1.read();
      Serial1.println("Got new setup!");
      for (int i=0;i!=8;i++)
      {
        Serial1.print("gainCoeffs:");
        Serial1.print(gainCoeffs[i]);
        Serial1.print("\t");
      }
      Serial1.print("\n");
      Serial1.print("upperFreq:");
      Serial1.println(upperFreq);
      Serial1.print("lowerFreq:");
      Serial1.println(lowerFreq);
      Serial1.print("thresholdFFT:");
      Serial1.println(thresholdFFT);
      Serial1.print("meanPower:");
      Serial1.println(meanPower);
    }
  }
  else
  {
    Serial1.println("Using defaults.");
  }
}