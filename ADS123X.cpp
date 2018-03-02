/*
  ADS123X.h - Library for reading from a ADS1232 and ADS1234 24-bit ADC.
  Created by Hamid Saffari @ Jan 2018.
  Released into the public domain.
  
  based on these great libraries:
  HX711: by bodge -> https://github.com/bogde/HX711
  ADS1232: by Jeffrey M. Kubascik -> https://github.com/jeffkub/beer-gauge/tree/master/firmware/beer-gauge
   
*/
#include "ADS123X.h"

#if ARDUINO_VERSION <= 106
    // "yield" is not implemented as noop in older Arduino Core releases, so let's define it.
    // See also: https://stackoverflow.com/questions/34497758/what-is-the-secret-of-the-arduino-yieldfunction/34498165#34498165
    void yield(void) {};
#endif

ADS123X::ADS123X(byte pin_DOUT, byte pin_SCLK, byte pin_PDWN, byte pin_GAIN0, byte pin_GAIN1, byte pin_SPEED, byte pin_A0, byte pin_A1_or_TEMP) {
  _pin_DOUT = pin_DOUT;
  _pin_SCLK = pin_SCLK;
  _pin_PDWN = pin_PDWN;
  _pin_GAIN0 = pin_GAIN0;
  _pin_GAIN1 = pin_GAIN1;
  _pin_SPEED = pin_SPEED;
  _pin_A0 = pin_A0;
  _pin_A1_or_TEMP = pin_A1_or_TEMP;
  

  pinMode(_pin_DOUT,  INPUT_PULLUP);
  pinMode(_pin_SCLK, OUTPUT);
  pinMode(_pin_PDWN, OUTPUT);
  pinMode(_pin_GAIN0, OUTPUT);
  pinMode(_pin_GAIN1, OUTPUT);
  pinMode(_pin_SPEED, OUTPUT);
  pinMode(_pin_A0, OUTPUT);
  pinMode(_pin_A1_or_TEMP, OUTPUT);
  
  setGain(GAIN128);
  setSpeed(FAST);
  
  power_up();

}
	

ADS123X::ADS123X() {
}

ADS123X::~ADS123X() {
}

bool ADS123X::is_ready(void)
{
  return digitalRead(_pin_DOUT) == LOW;
}

void ADS123X::setGain(Gain gain)
{
  switch(gain)
  {
    case GAIN1:
    {
      digitalWrite(_pin_GAIN1, LOW);
      digitalWrite(_pin_GAIN0, LOW);
      break;
    }
    case GAIN2:
    {
      digitalWrite(_pin_GAIN1, LOW);
      digitalWrite(_pin_GAIN0, HIGH);
      break;
    }
    case GAIN64:
    {
      digitalWrite(_pin_GAIN1, HIGH);
      digitalWrite(_pin_GAIN0, LOW);
      break;
    }
    case GAIN128:
    {
      digitalWrite(_pin_GAIN1, HIGH);
      digitalWrite(_pin_GAIN0, HIGH);
      break;
    }
  }
}

void ADS123X::power_up(void)
{
  digitalWrite(_pin_PDWN, HIGH);
  
  // Set CLK low to get the ADS1231 out of suspend
  digitalWrite(_pin_SCLK, LOW);
}

void ADS123X::power_down(void)
{
  digitalWrite(_pin_PDWN, LOW);
  digitalWrite(_pin_SCLK, HIGH);
}


void ADS123X::setSpeed(Speed speed)
{
  _speed = speed;
  switch(speed)
  {
    case SLOW:
    {
      digitalWrite(_pin_SPEED, LOW);
      break;
    }
    case FAST:
    {
      digitalWrite(_pin_SPEED, HIGH);
      break;
    }
  }
 
}

void ADS123X::setChannel(Channel channel)
{
  switch(channel)
  {
    case AIN1:
    {
      digitalWrite(_pin_A1_or_TEMP, LOW);
      digitalWrite(_pin_A0, LOW);
      break;
    }
    case AIN2:
    {
      digitalWrite(_pin_A1_or_TEMP, LOW);
      digitalWrite(_pin_A0, HIGH);
      break;
    }
  #if defined ADS1232
    case TEMP:
    {
      digitalWrite(_pin_A1_or_TEMP, HIGH);
      digitalWrite(_pin_A0, LOW);
      break;
    }
  #elif defined ADS1234
	case AIN3:
    {
      digitalWrite(_pin_A1_or_TEMP, HIGH);
      digitalWrite(_pin_A0, LOW);
      break;
    }
	case AIN4:
    {
      digitalWrite(_pin_A1_or_TEMP, HIGH);
      digitalWrite(_pin_A0, HIGH);
      break;
    }
  #endif

  }
}


/*
 * Get the raw ADC value. Can block up to 100ms in normal operation.
 * Returns 0 on success, an error code otherwise.
 */
ERROR_t ADS123X::read(Channel channel,long& value, bool Calibrating)
{
    int i=0;
    unsigned long start;
	unsigned int waitingTime;
	unsigned int SettlingTimeAfterChangeChannel=0;
	
	if(channel!=lastChannel){
		setChannel(channel);
		
		if(_speed==FAST) SettlingTimeAfterChangeChannel=55;
		else SettlingTimeAfterChangeChannel=405;
		lastChannel=channel;
	}
	
    /* A high to low transition on the data pin means that the ADS1231
     * has finished a measurement (see datasheet page 13).
     * This can take up to 100ms (the ADS1231 runs at 10 samples per
     * second!).
     * Note that just testing for the state of the pin is unsafe.
     */
	 
	if(Calibrating){
		if(_speed==FAST) waitingTime=150;
		else waitingTime=850;
	}
	else{
		if(_speed==FAST) waitingTime=20;
		else waitingTime=150;
	}
	waitingTime+=SettlingTimeAfterChangeChannel;
	
    start=millis();
    while(digitalRead(_pin_DOUT) != HIGH)
    {
        if(millis() > start+waitingTime)
            return TIMEOUT_HIGH; // Timeout waiting for HIGH
    }
    start=millis();
    while(digitalRead(_pin_DOUT) != LOW)
    {
        if(millis() > start+waitingTime)
            return TIMEOUT_LOW; // Timeout waiting for LOW
    }
    //ads1231_last_millis = millis();

    // Read 24 bits
    for(i=23 ; i >= 0; i--) {
        digitalWrite(_pin_SCLK, HIGH);
        value = (value << 1) + digitalRead(_pin_DOUT);
        digitalWrite(_pin_SCLK, LOW);
    }

	
	if(Calibrating){
	// 2 extra bits for calibrating
		for(i=1 ; i >= 0; i--) {
			digitalWrite(_pin_SCLK, HIGH);
			digitalWrite(_pin_SCLK, LOW); 
		}
	}
	
    /* Bit 23 is acutally the sign bit. Shift by 8 to get it to the
     * right position (31), divide by 256 to restore the correct value.
     */
    value = (value << 8) / 256;

	if(!Calibrating){
		/* The data pin now is high or low depending on the last bit that
		 * was read.
		 * To get it to the default state (high) we toggle the clock one
		 * more time (see datasheet).
		 */
		digitalWrite(_pin_SCLK, HIGH);
		digitalWrite(_pin_SCLK, LOW);
	}

    return NoERROR; // Success
}

ERROR_t ADS123X::read_average(Channel channel, float& value, byte times, bool Calibrating) {
	long val = 0;
	long sum = 0;
	ERROR_t err;
	for (byte i = 0; i < times; i++) {
		err = read(channel, val, Calibrating);
		if(err!=NoERROR) return err;
		
		sum += val;
		yield();

	}
	if(times==0) return DIVIDED_by_ZERO;
	value = sum / times;
	return NoERROR;
}

ERROR_t ADS123X::get_value(Channel channel, float& value, byte times, bool Calibrating) {
	float val = 0;
	ERROR_t err;
	err = read_average(channel, val, times, Calibrating);
	if(err!=NoERROR) return err;
	value = val - OFFSET[channel-1];
	return NoERROR;
}

ERROR_t ADS123X::get_units(Channel channel, float& value, byte times, bool Calibrating) {
	float val = 0;
	ERROR_t err;
	err = get_value(channel, val, times, Calibrating);
	if(err!=NoERROR) return err;
	if(SCALE[channel-1]==0) return DIVIDED_by_ZERO;
	value = val / SCALE[channel-1];
	return NoERROR;
}

ERROR_t ADS123X::tare(Channel channel, byte times, bool Calibrating) {
	ERROR_t err;
	float sum = 0;
	err = read_average(channel, sum, times, Calibrating);
	if(err!=NoERROR) return err;
	set_offset(channel, sum);
	return NoERROR;
}

void ADS123X::set_scale(Channel channel, float scale) {
	SCALE[channel-1] = scale;
}

float ADS123X::get_scale(Channel channel) {
	return SCALE[channel-1];
}

void ADS123X::set_offset(Channel channel, float offset) {
	OFFSET[channel-1] = offset;
}

float ADS123X::get_offset(Channel channel) {
	return OFFSET[channel-1];
}
