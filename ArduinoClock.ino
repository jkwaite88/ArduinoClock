#include "LedControl.h"

#define ledPin 13
#define NUM_LED_DISPLAYS 4

//each number is 3x5 pixels, each character is represented by 5 rows (or bytes)
//and 3 columns represented by the least significant 3 bits
byte Numbers[10][5] = 
{
  {B00000111, B00000101, B00000101, B00000101, B00000111},
  {B00000001, B00000001, B00000001, B00000001, B00000001},
  {B00000111, B00000001, B00000111, B00000100, B00000111},
  {B00000111, B00000001, B00000111, B00000001, B00000111},
  {B00000101, B00000101, B00000111, B00000001, B00000001},
  {B00000111, B00000100, B00000111, B00000001, B00000111},
  {B00000111, B00000100, B00000111, B00000101, B00000111},
  {B00000111, B00000001, B00000001, B00000001, B00000001},
  {B00000111, B00000101, B00000111, B00000101, B00000111},
  {B00000111, B00000101, B00000111, B00000001, B00000001}
};

unsigned long delaytime1=500;
unsigned long delaytime2=50;

//LedControl lc=LedControl(12,10,11,1);
LedControl lc=LedControl(12,10,11,NUM_LED_DISPLAYS);

struct clock_t
{
  int hours;
  int minutes;
  int seconds;
  bool display24hr;
  bool updated;
};

volatile clock_t Clock;

void setup()
{
  int i;
  clockInit();
  
  pinMode(ledPin, OUTPUT);

  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = 15625;            // compare match register 16MHz/1024 - 1 second
 // OCR1A = 150;            // compare match register 16MHz/1024
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10);   //1024 // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt

  interrupts();             // enable all interrupts

  Serial.begin(115200);

 /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */

  for(i=0; i<NUM_LED_DISPLAYS; i++)
  {
      lc.shutdown(i,false);
      /* Set the brightness to a medium values */
      lc.setIntensity(i,2);
      /* and clear the display */
      lc.clearDisplay(i);
  }
}


ISR(TIMER1_COMPA_vect)          // timer compare interrupt service routine
{

  digitalWrite(ledPin, digitalRead(ledPin) ^ 1);   // toggle LED pin
  Clock.seconds += 1;
  if(Clock.seconds >= 60)
  {
    Clock.seconds = 0;
    Clock.minutes += 1;
  }
  if(Clock.minutes >= 60)
  {
    Clock.minutes = 0;
    Clock.hours += 1;
  }
  if(Clock.hours >= 24)
  {
    Clock.hours = 0;
  }
  Clock.updated = true;
}
  

void loop()
{

  if(Clock.updated == true)
  {
    
    
    writeTwoDigitNumber(Clock.seconds, 0);
    writeTwoDigitNumber(Clock.minutes, 1);
    writeTwoDigitNumber(Clock.hours, 2);
    WriteSerialTime();

    Clock.updated = false;
  }

}

void writeTwoDigitNumber(int number, int displayNum)
{
  byte dataRow;
  int row;
  int digits[2];
  int num;
  //int digit;
  
  num = number % 100;
  digits[0] = num % 10;
  digits[1] = (num-digits[0])/10;
  
  
  //create number data, 7x5 pixels - two numbers with a space in between
  for(row = 0; row<5; row++)
  {
    dataRow =(Numbers[digits[1]][row]<<4) | Numbers[digits[0]][row];
   lc.setRow(displayNum,row,dataRow);
  }
}


void clockInit()
{
  Clock.hours = 0;
  Clock.minutes = 0;
  Clock.seconds = 0;
  Clock.display24hr = true;
  Clock.updated = true;
}

void WriteSerialTime()
{
 char timeString[20];
 int offset;

  if(Clock.display24hr == true)
  {
    sprintf(timeString, "%02d:%02d:%02d\n", Clock.hours, Clock.minutes, Clock.seconds);
  }
  else
  {
    offset = 0;
    if(Clock.hours > 12)
    {
      offset = 12;
    }
    if(Clock.hours < 1)
    {
      offset = -12;
    }
    sprintf(timeString, "%02d:%02d:%02d", Clock.hours-offset, Clock.minutes, Clock.seconds);
    if(Clock.hours >= 12)
    {
      strcat(timeString, " pm\n");
    }
    else
    {
      strcat(timeString, " am\n");
    }
    
  }
  Serial.print(timeString);
  
}
