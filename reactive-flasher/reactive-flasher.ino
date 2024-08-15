#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/power.h>    
#include <avr/wdt.h> 
#include <EEPROM.h>

const int sensorPin                     = 3;
const int statusLED                     = 2;
const int powerPin                      = 1;
long counter = 0;
const char* txt = "Geocaching cache by PanJanek123. Check https://www.geocaching.com/ for more information and join the fun!";
int val = 0;
int previousVal = 0;
int analog = 0;
int val_diff;
int val_abs_diff;
byte old_ADCSRA = 0;

void setup() {
    old_ADCSRA = ADCSRA;
    pinMode(0, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(5, INPUT_PULLUP);

    //write 0 to minimise current leak
    pinMode(sensorPin, OUTPUT);
    digitalWrite(sensorPin, LOW);

    pinMode(statusLED, OUTPUT);
    digitalWrite(statusLED, HIGH);
    delay(125);
    digitalWrite(statusLED, LOW);
    delay(125);
    digitalWrite(statusLED, HIGH);
    delay(125);
    digitalWrite(statusLED, LOW);

    pinMode(powerPin, OUTPUT);
    digitalWrite(powerPin, LOW);

    for(int i=0; i<strlen(txt); i++) { EEPROM.put(4+i, txt[i]); }
    } // setup

ISR (PCINT0_vect)
 {
 // do something interesting here
 }  // end of PCINT0_vect
 
// watchdog interrupt
ISR (WDT_vect)
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR = 0;  
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}

void sleepForOneSecond ()
  {
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  noInterrupts ();      // timed sequence coming up: 1s
  setup_watchdog (5);      // 5 = get watchdog ready : 0.5s
  sleep_enable ();       // ready to sleep
  interrupts ();         // interrupts are required now
  sleep_cpu ();          // sleep                
  sleep_disable ();      // precaution
  power_all_enable ();   // power everything back on
  }  // end

void loop() {
    ADCSRA = old_ADCSRA; //128+1+2+4;
    pinMode(sensorPin, INPUT);
    digitalWrite(powerPin, HIGH);
    analog = analogRead(sensorPin);
    pinMode(sensorPin, OUTPUT);
    digitalWrite(sensorPin, LOW);
    digitalWrite(powerPin, LOW);

    val = analog;
    val_diff = previousVal - val;
    val_abs_diff = abs(val_diff);

    if ((val_abs_diff > 25) && (val > 700 || previousVal > 700) && (val < 990 || previousVal < 990)) {

      digitalWrite(statusLED, HIGH);
      EEPROM.get(0, counter);
      counter++;
      EEPROM.put(0, counter);
      delay(125);
      digitalWrite(statusLED, LOW);
    }      
    
    previousVal = val;
    sleepForOneSecond();
  }