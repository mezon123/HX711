#ifndef HX711_h
#define HX711_h

#include <stdlib.h>
#include <SoftwareSerial.h>
#include <Arduino.h>




#define ROZMIAR_TABLICY		32


class HX711
{
	private:
		enum GAIN_HX711 { CH_A_GAIN_128 = 1, CH_B_GAIN_32 = 2, CH_A_GAIN_64 = 3 };			
			
		//T_HX711_WEIGTH hx711_weigth;
		
		byte PD_SCK;	// Power Down and Serial Clock Input Pin
		byte DOUT;		// Serial Data Output Pin
		
		//enum GAIN_HX711 GAIN;		// amplification factor
		byte GAIN;
		
		long OFFSET;	// used for tare weight
		long SCALE;	// used to return weight in grams, kg, ounces, whatever
		

	public:	
	
		// define clock and data pin, channel, and gain factor
		// channel selection is made by passing the appropriate gain: 128 or 64 for channel A, 32 for channel B
		// gain: 128 or 64 for channel A; channel B works with 32 gain factor only
		HX711(byte dout, byte pd_sck, enum GAIN_HX711 gain = CH_A_GAIN_128);

		virtual ~HX711();


		// set the gain factor; takes effect only after a call to read()
		// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
		// depending on the parameter, the channel is also set to either A or B
		void set_gain(enum GAIN_HX711 gain = CH_A_GAIN_128);

		// waits for the chip to be ready and returns a reading
		uint32_t read(void);
		
		
		
		int32_t Bubble_Sort(int32_t *tab, uint8_t n);
		
		int32_t Simple_average(void);
		int32_t HX711::LPF_average(void);		// Low Pass Filter average 
		int32_t read_hx711_average(void);      // returns (read_average() - OFFSET), that is the current value without the tare weight; times = how many readings to do
		
		int32_t get_value(void);

		// returns get_value() divided by SCALE, that is the raw value divided by a value obtained via calibration
		// times = how many readings to do
		long get_units(void);
		
		void show_result(void);

		// set the OFFSET value for tare weight; times = how many times to read the tare value
		void tare(void);

		// set the SCALE value; this value is used to convert the raw data to "human readable" data (measure units)
		void set_scale(long scale = 1);

		// get the current SCALE
		long get_scale();

		// set OFFSET, the value that's subtracted from the actual reading (tare weight)
		void set_offset(long offset = 0);

		// get the current OFFSET
		long get_offset();

		// puts the chip into power down mode
		void power_down();

		// wakes up the chip after power down mode
		void power_up();
};

#endif /* HX711_h */
