/* SCREAMDETECTOR by Sgw32 */
/* 2k19 
for Aeroxo */
#include <HardwareTimer.h>
#include "Wire.h"
#include "UBLOX.h"
#include <STM32ADC.h>
#include <HMC5883L.h>
#include "arduinoFFT.h"
#include "mainutils.h"

void setupADC(uint8_t ch)
{
  ticks = 0;
  bufr = 0;
  bufw = 0;
  buffers[0] = &buffer[0];
  buffers[1] = &buffer[maxSamples/2];

  Timer3.setPeriod(samplePeriodus);
  Timer3.setMasterModeTrGo(TIMER_CR2_MMS_UPDATE);

  myADC.calibrate();

  // Set up our analog pin(s)
  for (unsigned int j = 0; j <8; j++) 
    pinMode(analogPins[j], INPUT_ANALOG);
  
  myADC.setSampleRate(ADC_SMPR_1_5); // ?
  myADC.setPins(&analogPins[ch], 1);
  myADC.setDMA(buffer, maxSamples, (DMA_MINC_MODE | DMA_CIRC_MODE | DMA_HALF_TRNS | DMA_TRNS_CMPLT), DmaIRQ);
  myADC.setTrigger(ADC_EXT_EV_TIM3_TRGO);
  myADC.startConversion(); 
}

void chADC(uint8_t ch)
{
  myADC.setPins(&analogPins[ch], 1);
  myADC.startConversion(); 
}

void setupGPS()
{
  gps.begin();  
}

void startupSequence()
{
  init_hamming_window(hammingwindow,FFTLEN);
  delay(1000);
  startupBlink();
  Serial1.begin(57600); // Ignored by Maple. But needed by boards using hardware serial via a USB to Serial adaptor
  Serial1.println("ScreamDetector started!");
  Serial1.println("Waiting 5s for setup, or using defaults.");
  delay(5000); 
  curChannel=0;
  readSetup();
}

void setup() {
  setupGPS();
  setupADC(0);
  Wire.begin(); // Start the I2C interface. 
  startupSequence();
}

void checkGPS()
{
  if(gps.readSensor()) {
    Serial1.print(gps.getLatitude_deg(),10);     ///< [deg], Latitude
    Serial1.print("\t");
    Serial1.print(gps.getLongitude_deg(),10);    ///< [deg], Longitude
    Serial1.print("\n");
  }
}


// Output the data down the serial port.
void magOutput(float headingDegrees)
{
   Serial1.print("   \tHeading:\t");
   Serial1.print(headingDegrees);
   Serial1.println(" Degrees   \t");
}

// Our main program loop.
void magRead()
{
  // Retrive the raw values from the compass (not scaled).
  MagnetometerRaw raw = compass.ReadRawAxis();
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  
  // Values are accessed like so:
  int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)

  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: 2� 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.0457;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 

  // Output the data via the serial port.
  magOutput(headingDegrees);

  // Normally we would delay the application by 66ms to allow the loop
  // to run at 15Hz (default bandwidth for the HMC5883L).
  // However since we have a long serial out (104ms at 9600) we will let
  // it run at its natural speed.
  delay(66);
}

void processADC()
{
  if (bufr!=bufw) {
    // process data 
    Serial1.println(curChannel&0x7);
    double data[maxSamples];
    double datai[maxSamples];
    double mean=0;
    data[0]=0.0;
    for (int i=0;i!=maxSamples;i++)
    {
      data[i]=(double)buffers[bufr][i];
      datai[i]=0.0;
      mean+=data[i];
    }
    /*mean/=maxSamples;
    for (int i=0;i!=maxSamples;i++)
    {
      data[i]-=mean;
    }*/
    
    FFT.Windowing(data, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(data, datai, samples, FFT_FORWARD); 
    FFT.ComplexToMagnitude(data, datai, samples);
    
    bufr = (bufr+1)%2;
    Serial1.println(curChannel&0x7);
    chADC((++curChannel)&0x7);
    
  }
}

void loop() {
  checkGPS();
  processADC();
  
  digitalWrite(PC13,!digitalRead(PC13));// Turn the LED from off to on, or on to off
  delay(1000);
}
