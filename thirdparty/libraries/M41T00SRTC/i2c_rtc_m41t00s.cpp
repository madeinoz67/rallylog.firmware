/*
  
  i2c_rtc_m41t00s.cpp Arduino class library for ST M41T00S real-time clock
  
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
  Copyright � 2009, 2010 Doc Walker <dfwmountaineers at gmail dot com>
  $Id: i2c_rtc_m41t00s.cpp 2 2010-02-12 17:48:16Z dfwmountaineers $
  
*/


/* _____PROJECT INCLUDES_____________________________________________________ */
#include "i2c_rtc_m41t00s.h"


/* _____PUBLIC FUNCTIONS_____________________________________________________ */
/**
Create class object.
*/
i2c_rtc_m41t00s::i2c_rtc_m41t00s()
{
}


/**
Initialize class object.

Sets up the TWI/I2C interface.
Call once class has been instantiated, typically within setup().
*/
void i2c_rtc_m41t00s::begin()
{
  Wire.begin();
}


/**
Retrieve time from RTC and populate time structure.

@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::get()
{
  uint8_t u8Status;
  
  Wire.beginTransmission(_ku8BaseAddress);
  Wire.send(_ku8Sec);
  u8Status = Wire.endTransmission();
  
  if (u8Status == ku8TWISuccess)
  {
    // request the _ku8Length bytes of data (sec, min, hour, dow, day, month, year, cal)
    if (Wire.requestFrom(_ku8BaseAddress, _ku8Length) == _ku8Length)
    {
      for (uint8_t i = 0; i < _ku8Length; i++)
      {
        _rtc_bcd[i] = Wire.receive();
      }
      
      time.sec = bcd2dec(_rtc_bcd[_ku8Sec] & 0x7f);		
      time.min = bcd2dec(_rtc_bcd[_ku8Min] & 0x7f);		
      time.hour = bcd2dec(_rtc_bcd[_ku8Hour] & 0x3f);
        
      time.dow = bcd2dec(_rtc_bcd[_ku8DOW]);
      time.day = bcd2dec(_rtc_bcd[_ku8Day]);
      time.month = bcd2dec(_rtc_bcd[_ku8Month]);
      time.year = bcd2dec(_rtc_bcd[_ku8Year]);
      time.cal = _rtc_bcd[_ku8Calibration];
    }
  }
  
  return u8Status;
}


/**
Set time from time structure.

@param tNewTime pointer to time structure
@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::set(time_t *tNewTime)
{
  _rtc_bcd[_ku8Sec] = (_rtc_bcd[_ku8Sec] & bit(_ku8Bit_ST)) |
    dec2bcd(constrain(tNewTime->sec, 0, 59));
  _rtc_bcd[_ku8Min] = dec2bcd(constrain(tNewTime->min, 0, 59));
  _rtc_bcd[_ku8Hour] = dec2bcd(constrain(tNewTime->hour, 0, 23));
  _rtc_bcd[_ku8DOW] = dec2bcd(constrain(tNewTime->dow, 1, 7));
  _rtc_bcd[_ku8Day] = dec2bcd(constrain(tNewTime->day, 1, 31));
  _rtc_bcd[_ku8Month] = dec2bcd(constrain(tNewTime->month, 1, 12));
  _rtc_bcd[_ku8Year] = dec2bcd(constrain(tNewTime->year, 0, 99));

  Wire.beginTransmission(_ku8BaseAddress);
  Wire.send(_ku8Sec);
  
  for (uint8_t i = 0; i < (_ku8Length - 1); i++)
  {
    Wire.send(_rtc_bcd[i]);
  }
  
  return Wire.endTransmission();
}


/**
Indicate status of oscillator stop bit.

@return status of oscillator (false=stop bit is clear, true=stop bit is set)
*/
uint8_t i2c_rtc_m41t00s::isStopped()
{
  if (read(_ku8Sec) == ku8TWISuccess)
  {
    return bitRead(_rtc_bcd[_ku8Sec], _ku8Bit_ST);
  }
  else
  {
    return true;
  }
}


/**
Indicate whether oscillator is running.

@return status of oscillator (false=stopped, true=running)
*/
uint8_t i2c_rtc_m41t00s::isRunning()
{
  return (!isStopped());
}


/**
Indicate status of oscillator fail bit.

@return status of oscillator fail bit (false=fail bit is clear, true=fail bit is set)
*/
uint8_t i2c_rtc_m41t00s::isFailed()
{
  if (read(_ku8Min) == ku8TWISuccess)
  {
    return bitRead(_rtc_bcd[_ku8Min], _ku8Bit_OF);
  }
  else
  {
    return true;
  }
}


/**
Set calibration byte.

@param u8Value calibration value (0x00..0xFF)
@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::setCalibration(uint8_t u8Value)
{
  _rtc_bcd[_ku8Calibration] = u8Value;
  
  return write(_ku8Calibration);
}


/**
Stop the clock.

@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::stop()
{
  // set the stop bit to stop the rtc
  bitSet(_rtc_bcd[_ku8Sec], _ku8Bit_ST);
  
  return write(_ku8Sec);
}


/**
Start the clock.

@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::start()
{
  bitClear(_rtc_bcd[_ku8Sec], _ku8Bit_ST);
  
  return write(_ku8Sec);
}


/**
Clear oscillator fail bit (OF).

@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::clearFault()
{
  uint8_t u8Status = read(_ku8Min);
  
  if (u8Status != ku8TWISuccess)
  {
    return u8Status;
  }
  
  bitClear(_rtc_bcd[_ku8Min], _ku8Bit_OF);
  
  return write(_ku8Min);
}


/* _____PRIVATE FUNCTIONS____________________________________________________ */
/**
Convert BCD value into decimal.

@param u8BcdValue BCD value (0x00..0xFF)
@return decimal value (0x00..0xFF)
*/
uint8_t i2c_rtc_m41t00s::bcd2dec(uint8_t u8BcdValue)
{
  //return (u8BcdValue & 0xF) + ((u8BcdValue & 0xF0) >> 4) * 10;
  return ( (u8BcdValue/16*10) + (u8BcdValue%16) );

}


/**
Convert decimal value into BCD.

@param u8ByteValue decimal value (0x00..0xFF)
@return BCD value (0x00..0xFF)
*/
uint8_t i2c_rtc_m41t00s::dec2bcd(uint8_t u8ByteValue)
{
  //return (u8ByteValue % 10) + ((u8ByteValue / 10) << 4);
  return ( (u8ByteValue/10*16) + (u8ByteValue%10) );

}


/**
Read a single byte from the rtc.

@param u8Register register from which to read (0.._ku8Length)
@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::read(uint8_t u8Register)
{
  uint8_t u8Status;
  
  Wire.beginTransmission(_ku8BaseAddress);
  Wire.send(u8Register);
  u8Status = Wire.endTransmission();
  
  if (u8Status == ku8TWISuccess)
  {
    if (Wire.requestFrom((uint8_t)_ku8BaseAddress, (uint8_t)1) == 1)
    {
      _rtc_bcd[u8Register] = Wire.receive();
    }
    else
    {
      return ku8TWIError;
    }
  }
  
  return u8Status;
}


/**
Write a single byte to the rtc.

@param u8Register register to be written (0.._ku8Length)
@return 0 on success; exception number on failure
*/
uint8_t i2c_rtc_m41t00s::write(uint8_t u8Register)
{
  Wire.beginTransmission(_ku8BaseAddress);
  Wire.send(u8Register);
  Wire.send(_rtc_bcd[u8Register]);
  
  return Wire.endTransmission();
}
