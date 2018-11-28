#include <Arduino.h>
#include <stdlib.h>
#include <HX711.h>

#define portOfPin(P)\
  (((P)>=0&&(P)<8)?&PORTD:(((P)>7&&(P)<14)?&PORTB:&PORTC))
#define ddrOfPin(P)\
  (((P)>=0&&(P)<8)?&DDRD:(((P)>7&&(P)<14)?&DDRB:&DDRC))
#define pinOfPin(P)\
  (((P)>=0&&(P)<8)?&PIND:(((P)>7&&(P)<14)?&PINB:&PINC))
#define pinIndex(P)((uint8_t)(P>13?P-14:P&7))
#define pinMask(P)((uint8_t)(1<<pinIndex(P)))

#define pinAsInput(P) *(ddrOfPin(P))&=~pinMask(P)
#define pinAsInputPullUp(P) *(ddrOfPin(P))&=~pinMask(P);digitalHigh(P)
#define pinAsOutput(P) *(ddrOfPin(P))|=pinMask(P)
#define digitalLow(P) *(portOfPin(P))&=~pinMask(P)
#define digitalHigh(P) *(portOfPin(P))|=pinMask(P)
#define isHigh(P)((*(pinOfPin(P))& pinMask(P))>0)
#define isLow(P)((*(pinOfPin(P))& pinMask(P))==0)
#define digitalState(P)((uint8_t)isHigh(P))

/*
Thus, you can save valuable code space and get dramatically faster execution by changing:

    pinMode( pin, INPUT ); with pinAsInput( pin );
    pinMode( pin, OUTPUT ); with pinAsOutput( pin );
    pinMode( pin, INPUT_PULLUP); with pinAsInputPullUp( pin );

    digitalWrite( pin, LOW ); with digitalLow( pin );
    digitalWrite( pin, HIGH ); with digitalHigh( pin );

    digitalRead( pin ) with digitalState( pin )


*/


HX711::HX711(byte dout, byte pd_sck, enum GAIN_HX711 gain) {
	PD_SCK 	= pd_sck;
	DOUT 	= dout;

	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);

	set_gain(gain);
}

HX711::~HX711() {

}


/*********************************************************
CH_A_GAIN_128 = 1, CH_A_GAIN_64 = 3, CH_B_GAIN_32 = 2
**********************************************************/
void HX711::set_gain(enum GAIN_HX711 gain)
 {	
	 GAIN = gain;
	 
	digitalWrite(PD_SCK, LOW);
	read();
}

//*********************************************************************

//*********************************************************************
uint32_t HX711::read(void) 
{
	//uint8_t i = 0;
	
	union T_HX711_WEIGTH{
		uint32_t data = 0;
		struct{
				uint8_t d3;
				uint8_t d2;
				uint8_t d1;
				uint8_t d0;
			};
	}hx711_weigth;
	
	
	// check if HX711 is ready
	// from the datasheet: When output data is not ready for retrieval, digital output pin DOUT is high. Serial clock
	// input PD_SCK should be low. When DOUT goes to low, it indicates data is ready for retrieval.		

	while (! (digitalRead(DOUT) == LOW) );

	// pulse the clock pin 24 times to read the data
	hx711_weigth.d0 = 0x00;
    hx711_weigth.d1 = shiftIn(DOUT, PD_SCK, MSBFIRST);
    hx711_weigth.d2 = shiftIn(DOUT, PD_SCK, MSBFIRST);
    hx711_weigth.d3 = shiftIn(DOUT, PD_SCK, MSBFIRST);
	
	// set the channel and the gain factor for the next reading using the clock pin
	do 
	{
		digitalWrite(PD_SCK, HIGH);
		digitalWrite(PD_SCK, LOW);
		
	}while(--GAIN);  //while(++i < GAIN);
	
	return  (hx711_weigth.data ^ 0x00800000);
}

//******************************************************************************************************

int32_t HX711::Bubble_Sort(int32_t *tab, uint8_t n)
{
	uint32_t pom =0;
	uint8_t  i =0, j =0;
	uint8_t min, max;

	for(j = 0; j < n - 1; j++)
	{
		for(i = 0; i < n - 1-j; i++)
			if( *(tab+i) > *(tab +(i + 1)) )
			{
				pom = *(tab +i+1);
				tab[i+1] = *(tab +i);
				*(tab +i) = pom;
			};
	};
	
	// srednia arytmetyczna kilku srodkowych pomiarow.
	pom =0;
	min = (ROZMIAR_TABLICY/2) -4;
	max = (ROZMIAR_TABLICY/2) +3;

	for(i = min; i <max; i++)
	{
		pom += tab[i];
	};

	//pom = (pom + (7/2))/7;    //(a + b/2)/b
	pom = (pom + ((max - min)/2))/(max - min);
	
	return pom;
}


int32_t HX711::Simple_average(void)
{
	const uint8_t LIMIT =16;
	
	uint8_t i = 0;
	uint32_t sum = 0;
	
	for(i=0; i < LIMIT; i++)
		sum += read();
	
	//sum /= LIMIT;
	sum >>= 4;  //2^4 = LIMIT
	
	return (long)sum;
}


int32_t HX711::read_hx711_average(void)
{
	uint8_t i =0;
	int32_t sum = 0;
	int32_t tablica[ROZMIAR_TABLICY];
	
	
	for(i=0; i< ROZMIAR_TABLICY; i++)
	{
		tablica[i] = 0;
		tablica[i] = read();
	};
	
	sum = Bubble_Sort(tablica, ROZMIAR_TABLICY);
	
	return sum;
}

//******************************************************************************************************

int32_t HX711::get_value(void) 
{
	long pom; 
	
	pom = Simple_average() - OFFSET;
	
	if( pom > 0)
		return pom;
	else
		return 0;		
}


 long HX711::get_units(void)
{
	unsigned long pom;
	
	pom = (unsigned long)get_value();
		
	return (100UL * pom )/SCALE;
}


void HX711::show_result(void)
{
	
	ldiv_t masaa;
	

	masaa = ldiv( get_units(), 100L);
	
	Serial.print("one reading:\t");
	Serial.print(masaa.quot );
	//Serial.print(",");
	Serial.write(',');
	Serial.print(masaa.rem );
	Serial.println("kg");
	/*
	Serial.print("\t| average:\t");
	Serial.print(masaa.quot );
	Serial.print(",");
	Serial.print(masaa.rem/10 );
	Serial.println("kg");
	*/  
}


void HX711::tare(void)
{
	OFFSET = Simple_average();

	Serial.print("\tOFFSET: \t");
	Serial.println(OFFSET);
}

void HX711::set_scale(long scale) {
	SCALE = scale;
}

long HX711::get_scale() {
	return SCALE;
}

void HX711::set_offset(long offset) {
	OFFSET = offset;
}

long HX711::get_offset() {
	return OFFSET;
}

void HX711::power_down() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);
}

void HX711::power_up() {
	digitalWrite(PD_SCK, LOW);
}
