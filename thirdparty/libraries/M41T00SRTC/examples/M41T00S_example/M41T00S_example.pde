/*

  M41T00S_example.pde - example using i2c_rtc_m41t00s library
  
  This file is part of i2c_rtc_m41t00s.
  
  i2c_rtc_m41t00s is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  i2c_rtc_m41t00s is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with i2c_rtc_m41t00s.  If not, see <http://www.gnu.org/licenses/>.
  
  Written by Doc Walker (Rx)
  Copyright © 2009, 2010 Doc Walker <dfwmountaineers at gmail dot com>
  $Id: M41T00S_example.pde 3 2010-02-12 17:57:17Z dfwmountaineers $
  
*/


#include <Wire.h>
#include <i2c_rtc_m41t00s.h>


// instantiate i2c_rtc_m41t00s object
i2c_rtc_m41t00s rtc;

// string to store current time/date
char dateString[50];


void setup()
{
  // initialize serial interface
  Serial.begin(19200);
  
  // initialize i2c interface
  rtc.begin();
}


void loop()
{
  uint8_t k, u8Status;
  
  if (rtc.isStopped())
  {
    Serial.println("RTC is stopped.");
    Serial.print("Recent: ");
  }
  else
  {
    if (rtc.isFailed())
    {
      Serial.println("RTC is running (fail bit is set).");
    }
    else
    {
      Serial.println("RTC is running.");
    }
    Serial.print("Currently: ");
  }
  
  printTime();
  Serial.println("");
  Serial.println("S - [S]tart RTC");
  Serial.println("P - sto[P] RTC");
  Serial.println("F - clear [F]ault");
  Serial.println("T - set [T]ime");
  Serial.println("C - set [C]al");
  Serial.println("D - [D]isplay current time");
  Serial.println("");
  Serial.println("Choose a menu item:");
  
  while (!Serial.available())
  {
  }
  
  Serial.println("----------------------------------------");
  
  k = Serial.read();
  switch(k)
  {
    case 'S':
    case 's':
      u8Status = rtc.start();
      if (u8Status)
      {
        Serial.print("Unable to start RTC due to I2C error 0x");
        Serial.print(u8Status, HEX);
        Serial.println(".");
      }
      break;
      
    case 'P':
    case 'p':
      u8Status = rtc.stop();
      if (u8Status)
      {
        Serial.print("Unable to stop RTC due to I2C error 0x");
        Serial.print(u8Status, HEX);
        Serial.println(".");
      }
      break;
      
    case 'F':
    case 'f':
      u8Status = rtc.clearFault();
      if (u8Status)
      {
        Serial.print("Unable to clear RTC fault due to I2C error 0x");
        Serial.print(u8Status, HEX);
        Serial.println(".");
      }
      break;
      
    case 'T':
    case 't':
      setTime();
      break;
      
    case 'C':
    case 'c':
      setCal();
      break;
      
    case 'D':
    case 'd':
    default:
      break;
  }
  
  Serial.flush();
}


// print date/time to serial interface
void printTime()
{
  uint8_t u8Status = rtc.get();
  
  if (!u8Status)
  {
    sprintf(dateString, "%4u-%02u-%02u %02u:%02u:%02u (DOW: %u, CAL: 0x%02x).",
      2000 + rtc.time.year, rtc.time.month, rtc.time.day, rtc.time.hour,
        rtc.time.min, rtc.time.sec, rtc.time.dow, rtc.time.cal);
    Serial.println(dateString);
  }
  else
  {
    Serial.print("Unable to get current time due to I2C error 0x");
    Serial.print(u8Status, HEX);
    Serial.println(".");
  }
}


void setTime()
{
  uint8_t x, y, u8Status;
  i2c_rtc_m41t00s::time_t newtime;
  
  Serial.println("Enter new time in format (dow is Sun, Mon, ... Sat):");
  Serial.println("yyyy-mm-dd hh:mm:ss dow");
  while (Serial.available() < 22)
  {
  }
  
  x = Serial.read(); // discard digit
  x = Serial.read(); // discard digit
  x = Serial.read(); // year: tens digit
  y = Serial.read(); // year: ones digit
  newtime.year = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // month: tens digit
  y = Serial.read(); // month: ones digit
  newtime.month = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // day: tens digit
  y = Serial.read(); // day: ones digit
  newtime.day = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // hour: tens digit
  y = Serial.read(); // hour: ones digit
  newtime.hour = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // min: tens digit
  y = Serial.read(); // min: ones digit
  newtime.min = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // sec: tens digit
  y = Serial.read(); // sec: ones digit
  newtime.sec = 10 * (x - '0') + (y - '0');
  
  x = Serial.read(); // discard spacer
  x = Serial.read(); // dow: 1st character
  y = Serial.read(); // dow: 2nd character
  
  switch(x)
  {
    case 'S':
    case 's':
      if (y == 'U' or y == 'u')
      {
        newtime.dow = 1;
      }
      
      if (y == 'A' or y == 'a')
      {
        newtime.dow = 7;
      }
      break;
      
    case 'M':
    case 'm':
      newtime.dow = 2;
      break;
      
    case 'T':
    case 't':
      if (y == 'U' or y == 'u')
      {
        newtime.dow = 3;
      }
      
      if (y == 'H' or y == 'h')
      {
        newtime.dow = 5;
      }
      break;
      
    case 'W':
    case 'w':
      newtime.dow = 4;
      break;
      
    case 'F':
    case 'f':
      newtime.dow = 6;
      break;
  }
  
  Serial.flush();
  
  u8Status = rtc.set(&newtime);
  if (u8Status)
  {
    Serial.print("Unable to set time due to I2C error 0x");
    Serial.print(u8Status, HEX);
    Serial.println(".");
  }
}


void setCal()
{
  uint8_t x, y, u8Status;
  
  Serial.println("Enter new calibration value in hex (skip 0x):");
  while (Serial.available() < 2)
  {
  }
  
  x = Serial.read(); // cal: upper nibble
  if (BOUND(x, '0', '9'))
  {
    x -= '0';
  }
  else if (BOUND(x, 'A', 'F'))
  {
    x -= ('A' - 10);
  }
  else if (BOUND(x, 'a', 'f'))
  {
    x -= ('a' - 10);
  }
  else
  {
    x = 0;
  }
  
  y = Serial.read(); // cal: lower nibble
  if (BOUND(y, '0', '9'))
  {
    y -= '0';
  }
  else if (BOUND(y, 'A', 'F'))
  {
    y -= ('A' - 10);
  }
  else if (BOUND(y, 'a', 'f'))
  {
    y -= ('a' - 10);
  }
  else
  {
    y = 0;
  }
  
  u8Status = rtc.setCalibration((x << 4) + y);
  if (u8Status)
  {
    Serial.print("Unable to set calibration value due to I2C error 0x");
    Serial.print(u8Status, HEX);
    Serial.println(".");
  }
}
