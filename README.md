# ADS123X
Library for reading from Texas Instruments ADS1232 and ADS1234 MultiChannel 24-bit ADC For Bridge Sensors.

Some settings and defines:

//======================== which one? ========================

//#define ADS1232
#define ADS1234
//============================================================

typedef enum ERROR_t {

	NoERROR,
	TIMEOUT_HIGH,     // Timeout waiting for HIGH
	TIMEOUT_LOW,      // Timeout waiting for LOW
	WOULD_BLOCK,      // weight not measured, measuring takes too long
	STABLE_TIMEOUT,   // weight not stable within timeout
	DIVIDED_by_ZERO   
	};

typedef enum Gain{

	GAIN1 = 1,
	GAIN2,
	GAIN64,
	GAIN128
	};

typedef enum Speed{

        SLOW = 0,
	FAST
	};

typedef enum Channel{

        AIN1 = 1,
	AIN2 = 2,
  #if defined ADS1232
	TEMP = 3,
  #else if defined ADS1234
        AIN3 = 3,
	AIN4 = 4
  #endif
};
=======================================================================================================

based on these great libraries:

HX711: by bodge -> https://github.com/bogde/HX711 

ADS1232: by Jeffrey M. Kubascik -> https://github.com/jeffkub/beer-gauge/tree/master/firmware/beer-gauge
