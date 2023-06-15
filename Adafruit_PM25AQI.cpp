/*!
 * @file Adafruit_PM25AQI.cpp
 *
 * @mainpage Adafruit PM2.5 air quality sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's PM2.5 AQI driver for the
 * Arduino platform.  It is designed specifically to work with the
 * Adafruit PM2.5 Air quality sensors: http://www.adafruit.com/products/4632
 *
 * These sensors use I2C or UART to communicate.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 *
 * @section author Author
 * Written by Ladyada for Adafruit Industries.
 *
 * @section license License
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "Adafruit_PM25AQI.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
extern Adafruit_SSD1306 display;
/*!
 *  @brief  Instantiates a new PM25AQI class
 */
Adafruit_PM25AQI::Adafruit_PM25AQI() {}

/*!
 *  @brief  Setups the hardware and detects a valid PMSA003I. Initializes I2C.
 *  @param  theWire
 *          Optional pointer to I2C interface, otherwise use Wire
 *  @return True if PMSA003I found on I2C, False if something went wrong!
 */
bool Adafruit_PM25AQI::begin_I2C(TwoWire *theWire) {
  if (!i2c_dev) {
    i2c_dev = new Adafruit_I2CDevice(PMSA003I_I2CADDR_DEFAULT, theWire);
  }

  if (!i2c_dev->begin()) {
    return false;
  }

  return true;
}

/*!
 *  @brief  Setups the hardware and detects a valid UART PM2.5
 *  @param  theSerial
 *          Pointer to Stream (HardwareSerial/SoftwareSerial) interface
 *  @return True
 */
bool Adafruit_PM25AQI::begin_UART(Stream *theSerial) {
  serial_dev = theSerial;

  return true;
}

/*!
 *  @brief  Setups the hardware and detects a valid UART PM2.5
 *  @param  data
 *          Pointer to PM25_AQI_Data that will be filled by read()ing
 *  @return True on successful read, false if timed out or bad data
 */
bool Adafruit_PM25AQI::read(PM25_AQI_Data *data) {
  uint8_t buffer[32];
  uint16_t sum = 0;
  uint16_t bytes_read;
  if (!data) {
    Serial.println("No place to put data");
    return false;
  }

  if (i2c_dev) { // ok using i2c?
    if (!i2c_dev->read(buffer, 32)) {
      return false;
    }
  } else if (serial_dev) { // ok using uart
    if (!serial_dev->available()) {
      Serial.println("No serial data available");
      display.drawChar(110,10,'*',SSD1306_WHITE,SSD1306_BLACK,1);	// put something on the display to show we got an error
      display.display();
      // return false;
    }
    int skipped = 0;
    while ((skipped < 32) && (serial_dev->peek() != 0x42)) {
      serial_dev->read();
      skipped++;
      if (!serial_dev->available()) {
        Serial.println("skipped until no Serial data available");
        display.drawChar(110,10,'S',SSD1306_WHITE,SSD1306_BLACK,1);	// put something on the display to show we got an error
        display.display();  
        bytes_read = serial_dev->readBytes(buffer, 32);   
        goto read_anyway;   
        // return false;
      }
    }
    if (serial_dev->peek() != 0x42) {
      serial_dev->read();
      Serial.println("Serial data not 0x42");
      display.drawChar(110,10,'4',SSD1306_WHITE,SSD1306_BLACK,1);	// put something on the display to show we got an error
      display.display();      
      return false;
    }
    // Now read all 32 bytes   
    /* I had to bypass this test as at this point, the error was always "Not 32 bytes of Serial data"  
      I wanted to see what would happen if we just went ahead and read the serial port  */
    //if (serial_dev->available() < 32) {  
    //  Serial.println(" Not 32 bytes of Serial data, but still going to proceed");
    // return false;
    //}
    bytes_read = serial_dev->readBytes(buffer, 32);
  read_anyway:
    Serial.print(" bytes actually read = "); Serial.println( bytes_read );
  } else {
    Serial.println("Why am I here?????");
    return false;
  }

  // Check that start bytes is correct!
  if ((buffer[0] != 0x42) || (buffer[1] != 0x4d)) {
    Serial.println(" first byte not 0x42 or second byte not 0x4d");
    display.drawChar(110,10,'4',SSD1306_WHITE,SSD1306_BLACK,1);	// put something on the display to show we got an error
    display.display();
    return false;
  }

  // get checksum ready
  for (uint8_t i = 0; i < 30; i++) {
    sum += buffer[i];
  }

  // The data comes in endian'd, this solves it so it works on all platforms
  uint16_t buffer_u16[15];
  for (uint8_t i = 0; i < 15; i++) {
    buffer_u16[i] = buffer[2 + i * 2 + 1];
    buffer_u16[i] += (buffer[2 + i * 2] << 8);
  }

  // put it into a nice struct :)
  memcpy((void *)data, (void *)buffer_u16, 30);

  if (sum != data->checksum) {
    Serial.println(" failed checksum");
      display.drawChar(110,10,'C',SSD1306_WHITE,SSD1306_BLACK,1);	// put something on the display to show we got an error
      display.display();    
    return false;
  }

  // success!
  return true;
}
