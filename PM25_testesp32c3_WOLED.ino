/* Test sketch for Adafruit PM2.5 sensor with UART or I2C */
#include <Wire.h>
#include "Adafruit_PM25AQI.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>



// If your PM2.5 is UART only, for UNO and others (without hardware serial)
// we must use software serial...
// pin #2 is IN from sensor (TX pin on sensor), leave pin #3 disconnected
// comment these two lines if using hardware serial
//#include <SoftwareSerial.h>
//SoftwareSerial pmSerial(2, 3);


#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define SSD1306_NO_SPLASH

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

void setup() {
  // Wait for serial monitor to open
  Serial.begin(115200);
  // while (!Serial) delay(10);
  delay(6000);
  Serial.println(F("Testing Adafruit PMSA003I Air Quality Sensor"));
  Wire.setPins(D4,D5);
  Wire.begin();
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  delay(1000);
  // Clear the buffer
  display.clearDisplay();

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.

  display.setTextSize(1);  // Normal 1:1 pixel scale
  // display.setRotation(2);
  display.setTextColor(SSD1306_WHITE);  // Draw white text
  display.setCursor(0, 0);              // Start at top-left corner
  display.println(F("Hello, world!"));
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);


  // If using serial, initialize it and set baudrate before starting!
  // Uncomment one of the following
  Serial1.begin(9600,SERIAL_8N1,D7,D6);
  //pmSerial.begin(9600);

  // There are 3 options for connectivity!
  //if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
  if (!aqi.begin_UART(&Serial1)) {  // connect to the sensor over hardware serial
                                    //if (! aqi.begin_UART(&pmSerial)) { // connect to the sensor over software serial
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }

  Serial.println("PM25 found!");
}

void loop() {
  PM25_AQI_Data data;

  if (!aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
                 //Serial.println("Plowing ahead anyway");
    return;
  }
  Serial.println("AQI reading success");

  Serial.println();
  Serial.println(F("---------------------------------------"));
  Serial.println(F("Concentration Units (standard)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: "));
  Serial.print(data.pm10_standard);
  Serial.print(F("\t\tPM 2.5: "));
  Serial.print(data.pm25_standard);
  Serial.print(F("\t\tPM 10: "));
  Serial.println(data.pm100_standard);
  Serial.println(F("Concentration Units (environmental)"));
  Serial.println(F("---------------------------------------"));
  Serial.print(F("PM 1.0: "));
  Serial.print(data.pm10_env);
  Serial.print(F("\t\tPM 2.5: "));
  Serial.print(data.pm25_env);
  Serial.print(F("\t\tPM 10: "));
  Serial.println(data.pm100_env);
  Serial.println(F("---------------------------------------"));
  Serial.print(F("Particles > 0.3um / 0.1L air:"));
  Serial.println(data.particles_03um);
  Serial.print(F("Particles > 0.5um / 0.1L air:"));
  Serial.println(data.particles_05um);
  Serial.print(F("Particles > 1.0um / 0.1L air:"));
  Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:"));
  Serial.println(data.particles_25um);
  Serial.print(F("Particles > 5.0um / 0.1L air:"));
  Serial.println(data.particles_50um);
  Serial.print(F("Particles > 10 um / 0.1L air:"));
  Serial.println(data.particles_100um);
  Serial.println(F("---------------------------------------"));
  display.clearDisplay();
  //display.setTextSize(1); // Draw 1X-scale text
  display.setCursor(0, 0);
  display.print("P > 0.3 ");
  display.println(data.particles_03um);
  display.print("P > 0.5 ");
  display.println(data.particles_05um);
  display.print("P > 1.0 ");
  display.println(data.particles_10um);
  display.print("P > 2.5 ");
  display.println(data.particles_25um);
  display.print("P > 5.0 ");
  display.println(data.particles_50um);
  display.print("P > 10.0");
  display.println(data.particles_100um);
  display.display();
  delay(3000);
  Serial1.flush();
}
