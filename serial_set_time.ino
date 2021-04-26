/*
   TimeSerial.pde
   example code illustrating Time library set through serial port messages.

   Messages consist of the letter T followed by ten digit time (as seconds since Jan 1 1970)
   you can send the text on the next line using Serial Monitor to set the clock to noon Jan 1 2013
  T1357041600

   A Processing example sketch to automatically send the messages is included in the download
   On Linux, you can use "date +T%s\n > /dev/ttyACM0" (UTC time zone)
*/
#include <stdio.h>
#include <TimeLib.h>
#include <TimeAlarms.h>  // https://github.com/PaulStoffregen/TimeAlarms
AlarmId id;
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 
long adjusttime;


uint16_t hh;
uint16_t mm;
uint16_t s;
uint16_t mn;
uint16_t dyy;
uint16_t yr;

char buffer[32];

float sectoadjustperday;




void setup()  {
  Serial.begin(9600);
  while (!Serial) ; // Needed for Leonardo only
  pinMode(13, OUTPUT);
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Waiting for sync message");
  setTime(8, 38, 00, 24, 4, 21);
  set_clock();
  // setDateTime(__DATE__,__TIME__);


  // timer to adjust the clock + or - a second every ?? seconds
  // 8.5 seconds a day for my clock to adjust
  sectoadjustperday = 8.5; // + or -
  adjusttime = (24 * 60 * 60L) / fabs(sectoadjustperday); // hour*Minutes*Seconds / secondstoadjustperday

  Serial.println(adjusttime);
  Alarm.timerRepeat(adjusttime, Repeats_adjusttime);           // timer for every ?? seconds
}

void loop() {
  if (Serial.available()) {
    //processSyncMessage();
    set_clock();
  }
  if (timeStatus() != timeNotSet) {
    digitalClockDisplay();
  }
  if (timeStatus() == timeSet) {
    digitalWrite(13, HIGH); // LED on if synced
  } else {
    digitalWrite(13, LOW);  // LED off if needs refresh
  }
  //delay(1000);
  
  Alarm.delay(1000); // wait one second between clock display
}




void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(adjusttime);
  Serial.print(" ");
  Serial.println(now());
  //Serial.println(Repeats_adjusttime());
  //Serial.println(__DATE__);
  //Serial.println(__TIME__);

}

void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}


void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if (Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
    }

  }
  setTime(8, 04, 00, day(), month(), year());
}

time_t requestSync()
{
  Serial.write(TIME_REQUEST);
  return 0; // the time will be sent later in response to serial mesg
}

void Repeats_adjusttime() {
  if (sectoadjustperday < 0) {
    Serial.println("adjust time - 1 second");
    adjustTime(-1); // clock is to fast minus a second
  }
  if (sectoadjustperday > 0) {
    Serial.println("adjust time + 1 second");
    adjustTime(+1); // clock is to slow add a second
  }
}


// get user input, set clock https://forum.arduino.cc/t/how-to-set-ds3231-by-serial-monitor/205193/8
void set_clock (void)
{
    hh = 0; mm = 0; s = 0;
    mn = 0; dyy = 0; yr = 0;
    Serial.print ("Enter the time:\r\n");
    Serial.print ("HH MM SS DD MM YYYY\r\n");
    readline (buffer, 200);
   Serial.print ("1buffer=");Serial.println (buffer);
    sscanf (buffer, "%2u %2u %2u %2u %2u %4u", &hh, &mm, &s, &dyy, &mn,  &yr);
 Serial.print ("2buffer=");Serial.println (buffer);
    if (hh + mm + s +  dyy + mn  + yr) {
       // RTC.writeTime (h, m, s, mn, dy, yr); // "write time" or whatever the function is that sets the RTC
             Serial.println ("\r\nClock is set!\r\n");

Serial.println (hh);
Serial.println (mm);
Serial.println (s);
Serial.println (mn);
Serial.println (dyy);
Serial.println (yr);        
        setTime(hh, mm, s, dyy, mn, yr);
//setTime(10, 10, 10, 10, 10, 2021);
        

    } else {
        Serial.print ("\r\nClock has not been changed\r\n");
    }
}

//https://forum.arduino.cc/t/how-to-set-ds3231-by-serial-monitor/205193/8
uint16_t readline (char *buffer, int limit)
{
    char c;
    int ptr = 0;

    while (1) {

        if (Serial.available()) {

            c = Serial.read();

            if (c == 0x0D) { // cr == end of line
                *(buffer + ptr) = 0; // mark end of line
                break; // return char count
            }

            if (c == 0x08) { // backspace

                if (ptr) { // if not at the beginning

                    *(buffer + --ptr) = 0;
                    Serial.print ( (char) 0x08); // erase backspaced char
                    Serial.print ( (char) 0x20);
                    Serial.print ( (char) 0x08);

                } else {
                    Serial.print ( (char) 0x07); // ascii bel (beep) if terminal supports it
                }

            } else { // not a backspace, handle it

                if (ptr < (limit - 1)) { // if not at the end of the line

                    Serial.print ( (char) c); // echo char
                    *(buffer + ptr++) = c; // put char into buffer
                    Serial.print("ptr=");
                    Serial.println(ptr);

                } else { // at end of line, can't add any more
                    Serial.print ( (char) 0x07); // ascii bel (beep) if terminal supports it
                    Serial.print("end ");
                }
            }
        }
    }
    

    return ptr;
}
