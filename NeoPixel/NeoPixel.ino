/******************************************************************************
 *
 *	Module:			NeoPixel.ino
 *	Project Name:	Drone Illumination Project
 *	Designer:		Adam Johnson
 *	Language:		C++
 *	Environment:	Arduino 1.6.6
 *
 *	Description:	Lights up a ring of NeoPixels in the sky.
 *
 *	IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across 
 *	pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
 *	and minimize distance between Arduino and first pixel.  Avoid connecting
 *	on a live circuit...if you must, connect GND first.
 *
 *	NeoPixel type flags deserve some explanation.  Add together as needed, and
 *	use them when declaring the pixel strip:
 *	NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
 *	NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
 *	NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
 *	NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
 *
 ******************************************************************************/

#include <Adafruit_NeoPixel.h>			// driver for pixels
#ifdef __AVR__							// If we're running on an AVR...
#include <avr/power.h>					// AVR clock prescale functions
#endif

#define PIN_BUTTON			1			// pin with button attached
#define PIN_PIXELS			2			// pin with NeoPixels attached
#define NUM_PIXELS			12			// number of NeoPixels to control
#define PIXEL_BRIGHTNESS	80			// how bright to make the pixels (0-255)

//                            RRGGBB
#define COLOR_BLACK			0x000000
#define COLOR_RED			0xFF0000
#define COLOR_GREEN			0x00FF00
#define COLOR_BLUE			0x0000FF
#define COLOR_WHITE			0xFFFFFF
#define COLOR_LIGHT_WHITE	0x7F7F7F
#define COLOR_LIGHT_RED		0x7F0000
#define COLOR_LIGHT_BLUE	0x00007F
#define COLOR_CYAN			0x00FFFF
#define COLOR_MAGENTA		0xFF00FF
#define COLOR_YELLOW		0xFFFF00
#define COLOR_CHARTREUSE	0x7FFF00
#define COLOR_ORANGE		0xFF6000
#define COLOR_AQUAMARINE	0x7FFFD4
#define COLOR_PINK			0xFF5F5F
#define COLOR_TURQUOISE		0x3FE0C0
#define COLOR_REALWHITE		0xC8FFFF
#define COLOR_INDIGO		0x3F007F
#define COLOR_VIOLET		0xBF7FBF
#define COLOR_MAROON		0x320010
#define COLOR_BROWN			0x0E0600
#define COLOR_CRIMSON		0xDC283C
#define COLOR_PURPLE		0x8C00FF

enum									// modes of operation
{
	EFFECT_ALL = 0,						// rotate through all the effects
	EFFECT_BREATHE,						// one color; dim and brighten
	EFFECT_COLOR_WIPE,					// one color; update one pixel at a time
	EFFECT_THEATRE_CHASE,				// one color; chasing lights
	EFFECT_RAINBOW,						// many colors; all pixels the same
	EFFECT_RAINBOW_CYCLE,				// many colors; all pixels different
	EFFECT_THEATRE_CHASE_RAINBOW,		// many colors; chasing lights
	NUM_MODES
} ledEffect_t;

int g_buttonState;						// variable to hold the button state
int g_lightMode = EFFECT_BREATHE;		// what mode we're in
int g_lightModePrev = EFFECT_BREATHE;	// the last mode we were in

// Create a NeoPixel object.
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXEL, NEO_GRB + NEO_KHZ800);

// This code runs once, at startup.
void setup()
{
	// If this code is running on a 16 MHz Trinket, set clock divider.
#if (defined (__AVR_ATtiny85__) && (F_CPU == 16000000))
	clock_prescale_set(clock_div_1);
#endif

	// Set the button's pin as input with internal pull-up resistor.
	pinMode(PIN_BUTTON, INPUT_PULLUP);
	
	// Set the pixel pin as output.
	pinMode(PIN_PIXELS, OUTPUT);

	// Initialize the pixels.
	strip.begin();

	// If you want, set the brightness.  Else it will be really bright.
	strip.setBrightness(PIXEL_BRIGHTNESS);
	
	// Read the initial state of the switch.
	g_buttonState = digitalRead(PIN_BUTTON);

	// Initialize all pixels to 'off'.
	strip.show();
}

// This code runs over and over forever.
void loop()
{
	int val;	// variable for reading the button
	int val2;	// variable for debouncing the button
	
	// Read the button (twice, to debounce it).
	val = digitalRead(PIN_BUTTON);
	delay (20);
	val2 = digitalRead(PIN_BUTTON);
	
	// If the button has been pressed...
	if ((val == val2) && (val != g_buttonState) && (val == LOW))
	{
		// Change the mode.
		g_lightMode++;
		
		// If we've reached the last mode, start over.
		if (g_lightMode == NUM_MODES)
			g_lightMode = 0;
	}
	
	// Save the new state in our variable.
	g_buttonState = val;
	
	// If we've just entered a new light mode...
	if (g_lightModePrev != g_lightMode)
	{
		// Do anything required when a mode starts.
		if (g_lightMode == EFFECT_RAINBOW)
			RainbowInit();
		
		// Ensure that this stuff only happens once.
		g_lightModePrev = g_lightMode;
	}
	
	// Do any recurring actions associated with the light mode.
	switch (g_lightMode)
	{
	case EFFECT_ALL:
		break;
	
	case EFFECT_BREATHE:
		EffectBreathe(COLOR_RED);
		EffectBreathe(COLOR_BLUE);
		break;

	case EFFECT_COLOR_WIPE:
		EffectColorWipe(COLOR_RED, 50);
		EffectColorWipe(COLOR_GREEN, 50);
		EffectColorWipe(COLOR_BLUE, 50);
		break;
		
	case EFFECT_THEATRE_CHASE:
		EffectTheaterChase(COLOR_LIGHT_WHITE, 50);
		EffectTheaterChase(COLOR_LIGHT_RED, 50);
		EffectTheaterChase(COLOR_LIGHT_BLUE, 50);
		break;
		
	case EFFECT_RAINBOW:
		EffectRainbow(20);
		break;
		
	case EFFECT_RAINBOW_CYCLE:
		EffectRainbowCycle(20);
		break;
		
	case EFFECT_THEATRE_CHASE_RAINBOW:
		EffectTheaterChaseRainbow(50);
		break;

	default:
		g_effect = EFFECT_ALL;
	}
}

void EffectBreathe(uint32_t color)
{
	float MaximumBrightness = 255;
	float SpeedFactor = 0.008; // I don't actually know what would look good
	float StepDelay = 5; // ms for a step delay on the lights
	
	// Make the lights breathe
	for (int i = 0; i < 65535; i++)
	{
		// Intensity will go from 10 - MaximumBrightness in a "breathing" manner
		float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
		strip.setBrightness(intensity);
	
		// Now set every LED to that color
		for (int ledNumber = 0; ledNumber < PIXEL_NUM; ledNumber++)
			strip.setPixelColor(ledNumber, color);

		// Send data to the NeoPixels.
		strip.show();
		
		// Wait a bit before continuing to breathe.
		delay(StepDelay);
	}
}

// Fill the dots one after the other with a color
void EffectColorWipe(uint32_t c, uint8_t wait)
{
	for (uint16_t i = 0; i < strip.numPixels(); i++)
	{
		strip.setPixelColor(i, c);
		strip.show();
		delay(wait);
	}
}

void EffectRainbow(uint8_t wait)
{
	uint16_t i;		// counter for number of pixels
	uint16_t j;
	
	for (j = 0; j < 256; j++)
	{
		for (i = 0; i < strip.numPixels(); i++)
		{
			strip.setPixelColor(i, Wheel((i+j) & 255));
		}
		strip.show();
		delay(wait);
	}
}

// Slightly different, this makes the rainbow equally distributed throughout
void EffectRainbowCycle(uint8_t wait)
{
	uint16_t i;
	uint16_t j;

	// 5 cycles of all colors on wheel
	for (j = 0; j < 256 * 5; j++)
	{
		// For each pixel...
		for(i = 0; i < strip.numPixels(); i++)
		{
			// Set the pixel to the next color of the rainbow.
			strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
		}
		
		// Send the new colors to the pixels.
		strip.show();
		
		// Pause for effect (different values here change the rainbow's speed).
		delay(wait);
	}
}

// Theatre-style crawling lights.
void EffectTheaterChase(uint32_t c, uint8_t wait)
{
	// Do 10 cycles of chasing.
	for (int j = 0; j < 10; j++)
	{
		for (int q = 0; q < 3; q++)
		{
			// Turn every third pixel on.
			for (int i = 0; i < strip.numPixels(); i = i + 3)
			{
				strip.setPixelColor(i + q, c);
			}
			
			strip.show();
			delay(wait);

			// Turn every third pixel off.
			for (int i = 0; i < strip.numPixels(); i = i + 3)
			{
				strip.setPixelColor(i + q, 0);
			}
		}
	}
}

// Theatre-style crawling lights with rainbow effect.
void EffectTheaterChaseRainbow(uint8_t wait)
{
	// Cycle all 256 colors in the wheel.
	for (int j = 0; j < 256; j++)
	{
		for (int q = 0; q < 3; q++)
		{
			// Turn every third pixel on.
			for (int i = 0; i < strip.numPixels(); i = i + 3)
			{
				strip.setPixelColor(i+q, Wheel( (i+j) % 255));
			}
			
			strip.show();
			delay(wait);
	
			// Turn every third pixel off.
			for (int i = 0; i < strip.numPixels(); i = i + 3)
			{
				strip.setPixelColor(i+q, 0);
			}
		}
	}
}

/*******************************************************************************
*	Input a value 0 to 255 to get a color value.
*	The colours are a transition r - g - b - back to r.
*******************************************************************************/

uint32_t Wheel(byte wheelPos)
{
	if (WheelPos < 85)
	{
		return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	}
	else if (WheelPos < 170)
	{
		WheelPos -= 85;
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	else
	{
		WheelPos -= 170;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}
