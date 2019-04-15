/*

	Example of use of the FFT libray to compute FFT for a signal sampled through the ADC.
        Copyright (C) 2018 Enrique Condés and Ragnar Ranøyen Homb

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "arduinoFFT.h"

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
#define CHANNEL PA2
const uint16_t samples = 512; //This value MUST ALWAYS be a power of 2
const double samplingFrequency = 10000; //Hz, must be less than 10000 due to ADC

unsigned int sampling_period_us;
unsigned long microseconds;
uint8_t cnt = 0;
/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
double vReal[samples];
double vImag[samples];

double vMag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

void setup()
{
  pinMode(CHANNEL,INPUT_ANALOG);
  sampling_period_us = round(1000000*(1.0/samplingFrequency));
  Serial1.begin(57600);
  Serial1.println("Ready");
  for (int i=0;i!=samples;i++)
  {
    vMag[i]=0;
  }
}

void loop()
{
  /*SAMPLING*/
  for(int i=0; i<samples; i++)
  {
      microseconds = micros();    //Overflows after around 70 minutes!

      vReal[i] = analogRead(CHANNEL);
      vImag[i] = 0;
      while(micros() < (microseconds + sampling_period_us)){
        //empty loop
      }
  }
  
  FFT.Windowing(vReal, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, samples, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, samples);
  
  for (int i=0;i!=samples;i++)
  {
    vMag[i]+=vReal[i];
  }
  cnt++;
  
  if (cnt==32)
  {
    cnt=0;
    for (int i=0;i!=samples;i++)
      vMag[i]=vMag[i]/32;
    //Serial1.println("Computed magnitudes:");
    //PrintVector(vMag, (samples >> 1), SCL_FREQUENCY);
    Serial1.println("FREQ:");
    double x = FFT.MajorPeakPassFilter(vMag, samples, samplingFrequency,100.0,1000.0);
    int iix = x*samples/samplingFrequency;
    Serial1.print(vMag[iix]);
    Serial1.print("\t");
    Serial1.println(x, 6); //Print out what frequency is the most dominant.
    for (int i=0;i!=samples;i++)
    {
      vMag[i]=0;
    }
  }
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;
    /* Print abscissa value */
    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = (i * 1.0);
	break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / samplingFrequency);
	break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency) / samples);
	break;
    }
    Serial1.print(abscissa, 6);
    if(scaleType==SCL_FREQUENCY)
      Serial1.print("Hz");
    Serial1.print(" ");
    Serial1.println(vData[i], 4);
  }
  Serial1.println();
}
