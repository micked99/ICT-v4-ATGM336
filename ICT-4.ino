//#pragma GCC diagnostic error "-Wconversion"

/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
*/

//#include <avr/wdt.h>
#include <si5351.h>
#include <JTEncode.h>
#include <TimeLib.h>
#include <TinyGPS++.h>
#include <avr/power.h>
//#include <rs_common.h>
//#include <int.h>
//#include <string.h>



#define WSPR_CTC                2668 // CTC value for WSPR //10672 @ 16Mhz //5336 @ 8Mhz //2668 @ 4Mhz //1334 @ 2Mhz //667 @ 1Mhz
#define WSPR_TONE_SPACING       146           // 146 ~1.46 Hz


//#define WSPR_FREQ     7040185UL
//#define WSPR_FREQ     10140285UL          
#define WSPR_FREQ     14097192UL


// Enumerations
enum mode {MODE_WSPR};
TinyGPSPlus gps;
Si5351 si5351;
JTEncode jtencode;

// Global variables
unsigned long freq;
char call[] = "AA5ABC"; // WSPR Standard callsign
char call_telemetry[7]; // WSPR telemetry callsign
char loc_telemetry[5]; // WSPR telemetry locator
uint8_t dbm_telemetry; // WSPR telemetry dbm
char loc4[5]; // 4 digit gridsquare locator
byte Hour, Minute, Second; // Used for timing
long lat, lon, oldlat, oldlon; // Used for location
uint8_t tx_buffer[255]; // WSPR Tx buffer
uint8_t symbol_count; // JTencode
uint16_t tone_delay, tone_spacing; // JTencode
int alt_meters = 0;
bool telemetry_set = false;
int Sats = 0;
int gps_speed = 0;
volatile bool proceed = false;

void setModeWSPR()
{
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
 // tone_delay = WSPR_DELAY;
  memset(tx_buffer, 0, 255); // Clears Tx buffer from previous operation.
  jtencode.wspr_encode(call, loc4, 10, tx_buffer);
}

void setModeWSPR_telem()
{
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
//  tone_delay = WSPR_DELAY;
  memset(tx_buffer, 0, 255); // Clears Tx buffer from previous operation.
  jtencode.wspr_encode(call_telemetry, loc_telemetry, dbm_telemetry, tx_buffer);
}

void encode() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
{
    //si5351.output_enable(SI5351_CLK0, 1); // Turn off the CLK0 output
    //si5351.set_freq_manual((freq * 100) + (tx_buffer[i] * tone_spacing),87500000000ULL,SI5351_CLK0);
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing),SI5351_CLK0);
    proceed = false;
    while (!proceed);
    //delay(tone_delay);
}
  si5351.output_enable(SI5351_CLK0, 0); // Turn off the CLK0 output
  si5351.set_clock_pwr(SI5351_CLK0,0);  // Turn off the CLK0 clock
}

#include "TelemFunctions.h" // Various telemetry functions
#include "Timing.h" // Scheduling

// Timer interrupt vector.  This toggles the variable we use to gate
// each column of output to ensure accurate timing.  Called whenever
// Timer1 hits the count set below in setup().
ISR(TIMER1_COMPA_vect)
{
  proceed = true;
}

void setup()
{
  //clock_prescale_set(clock_div_2);
  //clock_prescale_set(clock_div_4);
  //clock_prescale_set(clock_div_8);
  
  pinMode(3, OUTPUT); digitalWrite(3, HIGH); //gps on
  
  pinMode(2, OUTPUT); digitalWrite(2, LOW); // Si5351 off
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);

  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);

  pinMode(A0, OUTPUT); // GPS VCC on
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH); 
  

  //delay(1000); // Allow GPS time to start
  Serial.begin(9600);
  //delay(1000); // Allow Serial time to start


  // Set up Timer1 for interrupts every symbol period.
  noInterrupts();          // Turn off interrupts.
  TCCR1A = 0;              // Set entire TCCR1A register to 0; disconnects
  //   interrupt output pins, sets normal waveform
  //   mode.  We're just using Timer1 as a counter.
  TCNT1  = 0;              // Initialize counter value to 0.
  TCCR1B = (1 << CS12) |   // Set CS12 and CS10 bit to set prescale
           (1 << CS10) |          //   to /1024
           (1 << WGM12);          //   turn on CTC   //   which gives, 64 us ticks
  TIMSK1 = (1 << OCIE1A);  // Enable timer compare interrupt.
  OCR1A = WSPR_CTC;        // Set up interrupt trigger count;
  interrupts();            // Re-enable interrupts.
  }

  void loop() 
  {
    loc4calc(); // Get position and update 4-char locator, 6-char locator and last 2 chars of 8-char locator
    call_telem(); // Update WSPR telemetry callsign based on previous information : position and altitude in meters
    while (Serial.available() > 0)
    if (gps.encode(Serial.read())) // GPS related functions need to be in here to work with tinyGPS Plus library
    if (timeStatus() == timeNotSet) // Only sets time if already not done previously
   
    setGPStime(); // Sets system time to GPS UTC time for sync
   
    if (gps.location.isValid()) TXtiming(); // Process timing 
   }
