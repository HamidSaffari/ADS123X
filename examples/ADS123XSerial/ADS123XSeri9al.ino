#include "ADS123X.h"

#define SCALE_DOUT   A0
#define SCALE_SCLK   A1
#define SCALE_PDWN   A2
#define SCALE_GAIN0  A3
#define SCALE_GAIN1  A4
#define SCALE_SPEED  A5
#define SCALE_A0     A6
#define SCALE_A1     A7

ADS123X scale;


void setup() {
	
  Serial.begin(38400);
  Serial.println("ADS123X Demo");

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  
  scale.begin(SCALE_DOUT, SCALE_SCLK, SCALE_PDWN, SCALE_GAIN0, SCALE_GAIN1, SCALE_SPEED, SCALE_A0, SCALE_A1);
  
  long value_long;
  scale.read(AIN1, value_long);
  Serial.println(value_long);			// print a raw reading from the ADC

  float value_double;
  Serial.print("read average: \t\t");
  scale.read_average(AIN1,value_double,20);
  Serial.println(value_long);  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  scale.get_value(AIN1,value_double,5);
  Serial.println(value_double);		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  scale.get_units(AIN1,value_double,5);
  Serial.println(value_double, 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided 
						// by the SCALE parameter (not set yet)  

  scale.set_scale(AIN1,2280.f);        // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare(AIN1);				        // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  scale.read(AIN1, value_long);
  Serial.println(value_long);                 // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  scale.read_average(AIN1,value_double,20);
  Serial.println(value_long);       // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  scale.get_value(AIN1,value_double,5);
  Serial.println(value_double);		// print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  scale.get_units(AIN1,value_double,5);
  Serial.println(value_double, 1);        // print the average of 5 readings from the ADC minus tare weight, divided 
						// by the SCALE parameter set with set_scale

  Serial.println("Readings:");
}

void loop() {
  float value;
  Serial.print("one reading:\t");
  scale.get_units(AIN1,value,1,true);
  Serial.print(value, 1);
  float value_avg;
  Serial.print("\t| average:\t");
  scale.get_units(AIN1,value_avg,10,true);
  Serial.println(value_avg, 1);

  //scale.power_down();			        // put the ADC in sleep mode
  delay(1000);
  //scale.power_up();
}
