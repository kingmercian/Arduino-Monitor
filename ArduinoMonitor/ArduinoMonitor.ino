/* Arduino Monitor
  |-------------------------------------------------------------BUGS-----------------------------------------------------------------|
  | *Once the serial input string buffer has incorrect data (i.e. an unknown command) the buffer has no function to clear and so does|
  |  not accept any new commands thus, a reset is required. This should be easy to resolve or can be ignored as only commands used   |
  |  will be sent by the ATR.                                                                                                        |
  |                                                                                                                                  |
  |------------------------------------------------------------ISSUES----------------------------------------------------------------|
  | * Due to limitations with Arduino currently (using serial comms) only "SLEEP_MODE_IDLE" can be implemented, while this does offer|
  |   power consumption reduction by disabling individual modules it is not as efficient as "SLEEP_MODE_PWR_DOWN". The reason for    |
  |   restriction is due to the board shutting down serial comms in every power mode but "SLEEP_MODE_PWR_DOWN" and require hardware  |
  |   interrupts. "SLEEP_MODE_PWR_DOWN" does still halt the program and await serial input before continueing the loop. Another      |
  |   caveat of this is that "SLEEP_MODE_PWR_DOWN" will resume on ANY serial input meaning it does not wait specifically for "PM1"  |
  |   and that any serial command will wake the device. NOTE: Try implementing a "if" just after sleep mode to perhaps fall back into|
  |   returning to sleep if not "PM1".                                                                                               |
  ------------------------------------------------------------------------------------------------------------------------------------
/**************************************************************************/
/*
    Libraries / definitions
*/
/**************************************************************************/
// Stardard libraries
#include <stdlib.h>     //C standard function library
#include <stdio.h>      //C IO functions
//FFT
#define LOG_OUT 1       // use the log output function
#define FFT_N 256       // set to 256 point fft
#include <FFT.h>        // FFT library
//Sleep
#include <avr/power.h>      // sleep mode functions
#include <avr/sleep.h>      // sleep mode functions
//Light Sensor
#include <Wire.h>                   //I2C comms library
#include <Adafruit_Sensor.h>        //adafruits unified sensor driver
#include <Adafruit_TSL2561_U.h>     //adafruits TSL2561 driver
//#include <SoftwareSerial.h>       //used for RS232 Comms

/**************************************************************************/
/*
    constants/variables
*/
/**************************************************************************/
// Misc variables
String version = "v1.0.1";
String incomingString;                // Declare incoming st ring
int incomingByte;                     // Declare incoming byte
float sleepStatus = 0;                // variable to store a request for sleep

// Constants / variables for voltage / current
const int SENSOR_PIN_V = A11;         // Input pin for measuring voltage
const int SENSOR_PIN_C = A1;          // Input pin for measuring current
const int RS = 10;                    // Value of shunt resistor, default 10kÎ©
const int RL = 10;                    // Value of the output resistor, default 10kÎ©
const int VOLTAGE_REF = 5;            // Reference voltage for analog read
const int voltReadings = 5;           // Number of voltage readings to keep
float sensorValueVolt;                // Variable to store value from analog read
float voltage;                        // Calculated voltage value
long intervalVolt = 100;              // Interval for voltage read
float voltreadings[voltReadings];     // the readings voltage
int readVoltIndex = 0;                // the index of the current volt reading
float totalVolt = 0;                  // the running total of volt readings
float maxVolt = 0;                    // the maximum volt reading
float minVolt = 1023;                 // the minimum volt readings
float averageVolt = 0;                // the average from volt readings
long previousMillisVolt = 0;          // Storage for voltage time
const int currReadings = 5;           // Number of current readings to keep
float sensorValueCurr;                // Variable to store value from analog read
float current;                        // Calculated current value
long intervalCurr = 100;              // Interval for current read
long previousMillisCurr = 0;          // Storage for current time
float currreadings[currReadings];     // the readings voltage
int readCurrIndex = 0;                // the index of the current volt reading
float totalCurr = 0;                  // the running total of volt readings
float maxCurr = 0;                    // the maximum current reading
float minCurr = 1023;                 // the minimum current readings
float averageCurr = 0;                // the average from volt readings

// Variables for FFT
int Result_1[128];
int Result_2[128];
int Result_3[128];
long temp_answer = 0;
int Answer[128];
int Attack = 0;
int Decay = 0;
int Sustain = 0;
int Release = 0;
long temp_volume = 0;
int Volume[1000];

// Constants / variables for light sensor
// The address will be different depending on whether you leave
// the ADDR pin float (addr 0x39), or tie it to ground or vcc. In those cases
// use TSL2561_ADDR_LOW (0x29) or TSL2561_ADDR_HIGH (0x49) respectively
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345); //TSL2561_ADDR_FLOAT, 1 == a unique ID and I2C address
const int lightReadings = 5;            // light readinsg to loop through
float sensorValueLight;                 // float for light sensor value
long intervalLight = 100;               // Interval for light read
long previousMillisLight = 0;           // Storage for light time
float lightreadings[lightReadings];     // the light sensor readings
int readLightIndex = 0;                 // the index of the current volt reading
float totalLight = 0;                   // the running total of volt readings
float maxLight = 0;                     // the maximum light reading
float minLight = 1023;                  // the minimum light readings
float averageLight = 0;                 // the average from light readings

/**************************************************************************/
/*
    Setup function
*/
/**************************************************************************/
void setup() {
  Serial.begin(9600);                         //Init serial
  Serial.print("Arduino Monitor ");  // print program start to console
  Serial.print(version);  // print program start to console
  Serial.print('\n');

  //Light sensor setup
  /* You can manually set the gain or enable auto-gain support for the light sensor */
   //tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
   //tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
   tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */
  /* Changing the integration time gives you better light sensor resolution (402ms = 16-bit data) */
   tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
   // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
   // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  // FFT setup
  //  TIMSK0 = 0; // turn off timer0 for lower jitter  //Do we really want to turn off the timer?
  //  ADCSRA = 0xe5; // set the adc to free running mode
    ADCSRB = 0x08; // sets second register to be used
    ADMUX = 0x40; // use adc8
    DIDR0 = 0x01; // turn off the digital input for adc8
}

/**************************************************************************/
/*
    reset arduino
*/
/**************************************************************************/
void resetArduino() {
  if (incomingString == "RESET") {
    Serial.print("Resetting");
    Serial.print('\n');
    delay(50);
    asm volatile ("  jmp 0");
    incomingString = "";
  }
}

/**************************************************************************/
/*
    sleep mode
*/
/**************************************************************************/
void sleepMode() //power comsumption reduction, this is due to any sleep mode requiring either hardware input interrupts or disabling any wake method via serial.
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();          // enables the sleep bit in the mcucr register
  //---------------------------//|--------------DISABLED MODULES START--------------|//    All modules applicable to the ATmega2560
  power_adc_disable();     //| Disable the Analog to Digital Converter module.  |//
  power_spi_disable();     //| Disable the Serial Peripheral Interface module.  |//
  power_timer0_disable();  //| Disable the Timer 0 module.                      |//    http://www.nongnu.org/avr-libc/user-manual/group__avr__power.html
  power_timer1_disable();  //| Disable the Timer 1 module.                      |//
  power_timer2_disable();  //| Disable the Timer 2 module.                      |//
  power_timer3_disable();  //| Disable the Timer 3 module.                      |//
  power_timer4_disable();  //| Disable the Timer 4 module.                      |//
  power_timer5_disable();  //| Disable the Timer 5 module.                      |//
  power_usart1_disable();  //| Disable the USART 1 module.                      |//
  power_usart2_disable();  //| Disable the USART 2 module.                      |//
  power_usart3_disable();  //| Disable the USART 3 module.                      |//
  power_twi_disable();     //| Disable the Two Wire Interface module.           |//
  //---------------------------//|---------------DISABLED MODULES END---------------|//
  sleep_mode();            // device is put to sleep
  sleepStatus = 1;         // update sleep status variable
  sleep_disable();         // disables the sleep bit in the mcucr register
  power_all_enable();      // enables all modules

}

void sleepCheck() {
  if (incomingString == "PM0") {
    Serial.print("Entering Sleep mode");
    Serial.print('\n');
    delay(100);     // this delay is needed, the sleep, function will produce a serial error otherwise
    sleepMode();     // sleep function
    incomingString = "";
  }
  if (incomingString == "PM1" && sleepStatus != 0) {
    Serial.print("Waking Up");
    Serial.print('\n');
    incomingString = "";
  }
  else if (sleepStatus == 0) {
    Serial.print("Entering Sleep mode");
    Serial.print('\n');
    delay(100);     // this delay is needed, the sleep, function will produce a serial error otherwise
    sleepMode();     // sleep function
    incomingString = "";
  }
}

/**************************************************************************/
/*
    VOLTAGE read every 100MS
*/
/**************************************************************************/
// Reads voltage every 100MS
void checkVoltageSensors() {
  //reads voltage
  unsigned long currentMillisVolt = millis();
  if (currentMillisVolt - previousMillisVolt > intervalVolt) {
    previousMillisVolt = currentMillisVolt;
    // Read a value from the INA169 board
    sensorValueVolt = analogRead(SENSOR_PIN_V);

    // Remap the ADC value into a voltage number (5V reference)
    voltage = (sensorValueVolt * VOLTAGE_REF) / 1023;

    // subtract the last reading:
    totalVolt = totalVolt - voltreadings[readVoltIndex];
    // read from the sensor:
    voltreadings[readVoltIndex] = (voltage);
    // add the reading to the total:
    totalVolt = totalVolt + voltreadings[readVoltIndex];

    // calc min/max volt
    if (voltreadings[readVoltIndex] > maxVolt) maxVolt = voltreadings[readVoltIndex];
    if (voltreadings[readVoltIndex] < minVolt) minVolt = voltreadings[readVoltIndex];

    // advance to the next position in the array:
    readVoltIndex = readVoltIndex + 1;
    // if we're at the end of the array...
    if (readVoltIndex >= voltReadings) {
      // ...wrap around to the beginning:
      readVoltIndex = 0;
    }
    averageVolt = totalVolt / voltReadings;
    //Serial.print("MINVOLT: "); Serial.print(minVolt); Serial.print("  ||  ");
    //Serial.print("MAXVOLT: ");Serial.print(maxVolt); Serial.println("");
    //Serial.print("Volt readings: "); Serial.print(voltReadings); Serial.print("  ||  ");
    //Serial.print("Total volt: "); Serial.print(totalVolt); Serial.print("  ||  ");
    //Serial.print("Average volt: "); Serial.print(averageVolt);Serial.println("");
  }
}

/**************************************************************************/
/*
    VOLTAGE serial read
*/
/**************************************************************************/
// VOLTAGE readOnce
// Waits for "MVM,1" over serial then reads voltage converts to "MVMR1,xx,yy"
// where xxyy = voltage in 0.1v per bit
void checkVoltageSerial() {
  if (incomingString == "MVM,1") // await serial string == MVM,1
  {
    // Output value to the serial monitor to 2 decimal
    // places and check length of string and formats
    char voltageBuffer[7];
    String voltageString = dtostrf(voltage, 7, 2, voltageBuffer);
    voltageString.trim();
    int voltageLength = voltageString.length();
    if (voltageLength < 5)
    {
      Serial.print("MVMR1,0");
    }
    else if (voltageLength >= 5)
    {
      Serial.print("MVMR1,");
    }
    voltageString.replace('.', ',');
    Serial.print(voltageString);
    Serial.print('\n');
    incomingString = "";
    voltageString = "";
  }


// VOLTAGE average
// Waits for "MVM,2" over serial then reads voltage and calculates average then
// converts to "MVMR2,xx,yy" where xxyy = voltage in 0.1v per bit
  else if (incomingString == "MVM,2") // await serial string "MVM,2"
  {
    char voltageBuffer[7];
    String voltageString = dtostrf(averageVolt, 7, 2, voltageBuffer);
    voltageString.trim();
    int voltageLength = voltageString.length();
    if (voltageLength < 5)
    {
      Serial.print("MVMR2,0");
    }
    else if (voltageLength >= 5)
    {
      Serial.print("MVMR2,");
    }
    voltageString.replace('.', ',');
    Serial.print(voltageString);
    Serial.print('\n');
    incomingString = "";
    voltageString = "";
  }
  
// VOLTAGE min
// Waits for "MVM,3" over serial then reads minimum voltage and
// converts to "MVMR3,xx,yy" where xxyy = voltage in 0.1v per bit
  else if (incomingString == "MVM,3") // await serial string "MVM,3"
  {
    char voltageBuffer[7];
    String voltageString = dtostrf(minVolt, 7, 2, voltageBuffer);
    voltageString.trim();
    int voltageLength = voltageString.length();
    if (voltageLength < 5)
    {
      Serial.print("MVMR3,0");
    }
    else if (voltageLength >= 5)
    {
      Serial.print("MVMR3,");
    }
    voltageString.replace('.', ',');
    Serial.print(voltageString);
    Serial.print('\n');
    incomingString = "";
    voltageString = "";
  }

// VOLTAGE max
// Waits for "MVM,4" over serial then reads maximum voltage and
// converts to "MVMR4,xx,yy" where xxyy = voltage in 0.1v per bit
  else if (incomingString == "MVM,4") // await serial string "MVM,4"
  {
    char voltageBuffer[7];
    String voltageString = dtostrf(maxVolt, 7, 2, voltageBuffer);
    voltageString.trim();
    int voltageLength = voltageString.length();
    if (voltageLength < 5)
    {
      Serial.print("MVMR4,0");
    }
    else if (voltageLength >= 5)
    {
      Serial.print("MVMR4,");
    }
    voltageString.replace('.', ',');
    Serial.print(voltageString);
    Serial.print('\n');
    incomingString = "";
    voltageString = "";
  }
}

//reset VOLTAGE stores
void resetVoltStores() {
  if (incomingString == "MVM,5") {
    memset(voltreadings, 0, sizeof(voltreadings));
    averageVolt = 0;
    minVolt = 1023;
    maxVolt = 0;
    totalVolt =  0;

    if (voltreadings > 0 && averageVolt == 0 && minVolt == 1023 && maxVolt == 0 
    && totalVolt ==  0) {
      Serial.print("MVMR5,01");
      Serial.print('\n');
      incomingString = "";
    }
    else if (voltreadings > 0) {
      Serial.print("MVMR5,00");
      Serial.print('\n');
      incomingString = "";
    }
  }
}

/**************************************************************************/
/*
    CURRENT read every 100MS
*/
/**************************************************************************/
  //reads current every 100ms
  void checkCurrentSensors() {
  unsigned long currentMillisCurr = millis();
  if (currentMillisCurr - previousMillisCurr > intervalCurr) {
    previousMillisCurr = currentMillisCurr;
    // Read a value from the INA169 board
    // Remap the ADC value into a voltage number (5V reference)
    sensorValueCurr = analogRead(SENSOR_PIN_C);
    sensorValueCurr = (sensorValueCurr * VOLTAGE_REF) / 1023;

    // Follow the equation given by the INA169 datasheet to
    // determine the current flowing through RS. Assume RL = 10k
    // Is = (Vout x 1k) / (RS x RL)
    current = sensorValueCurr / (RL * RS);

    // subtract the last reading:
    totalCurr = totalCurr - currreadings[readCurrIndex];
    // read from the sensor:
    currreadings[readCurrIndex] = (current);
    // add the reading to the total:
    totalCurr = totalCurr + currreadings[readCurrIndex];

    // calc min/max current
    if (currreadings[readCurrIndex] > maxCurr) maxCurr = currreadings[readCurrIndex];
    if (currreadings[readCurrIndex] < minCurr) minCurr = currreadings[readCurrIndex];


    // advance to the next position in the array:
    readCurrIndex = readCurrIndex + 1;
    // if we're at the end of the array...
    if (readCurrIndex >= currReadings) {
      // ...wrap around to the beginning:
      readCurrIndex = 0;
    }
    averageCurr = totalCurr / currReadings;
    //Serial.print("MINCURR: "); Serial.print(minCurr); Serial.print("  ||  ");
    //Serial.print("MAXCURR: ");Serial.print(maxCurr); Serial.println("");
    //Serial.print("Current readings: "); Serial.print(currReadings); Serial.print("  ||  ");
    //Serial.print("Total curr: "); Serial.print(totalCurr); Serial.print("  ||  ");
    //Serial.print("Average curr: "); Serial.print(averageCurr);Serial.println("");
  }
}

/**************************************************************************/
/*
    CURRENT serial read
*/
/**************************************************************************/
// Waits for "MCM,1" over serial then reads voltage converts to "MCMR1,xx,yy"
// where xxyy = voltage in 0.1ma per bit
void checkCurrentSerial() {
  if (incomingString == "MCM,1") // await serial string == MCM,1
  {
    // Output value to the serial monitor to 2 decimal
    // places and check length of string and formats
    char CurrentBuffer[7];
    String currentString = dtostrf(current, 7, 2, CurrentBuffer);
    currentString.trim();
    int currentLength = currentString.length();
    if (currentLength < 5)
    {
      Serial.print("MCMR1,0");
    }
    else if (currentLength >= 5)
    {
      Serial.print("MCMR1,");
    }
    currentString.replace('.', ',');
    Serial.print(currentString);
    Serial.print('\n');
    incomingString = "";
    currentString = "";
  }

//CURRENT average
// Waits for "MCM,2" over serial then reads current and calculates average then
// converts to "MCMR2,xx,yy" where xxyy = current in 0.1mA per bit
  else if (incomingString == "MCM,2") // await serial string "MCM,2"
  {
    char currentBuffer[7];
    String currentString = dtostrf(averageCurr, 7, 2, currentBuffer);
    currentString.trim();
    int currentLength = currentString.length();
    if (currentLength < 5)
    {
      Serial.print("MCMR2,0");
    }
    else if (currentLength >= 5)
    {
      Serial.print("MVMR2,");
    }
    currentString.replace('.', ',');
    Serial.print(currentString);
    Serial.print('\n');
    incomingString = "";
    currentString = "";
  }

// CURRENT min
// Waits for "MCM,3" over serial then reads minimum current and
// converts to "MCMR3,xx,yy" where xxyy = current in 0.1mA per bit
  else if (incomingString == "MCM,3") // await serial string "MCM,3"
  {
    char currentBuffer[7];
    String currentString = dtostrf(minCurr, 7, 2, currentBuffer);
    currentString.trim();
    int currentLength = currentString.length();
    if (currentLength < 5)
    {
      Serial.print("MCMR3,0");
    }
    else if (currentLength >= 5)
    {
      Serial.print("MVMR3,");
    }
    currentString.replace('.', ',');
    Serial.print(currentString);
    Serial.print('\n');
    incomingString = "";
    currentString = "";
  }


// CURRENT max
// Waits for "MCM,4" over serial then reads maximum current and
// converts to "MCMR4,xx,yy" where xxyy = current in 0.1mA per bit
  if (incomingString == "MCM,4") // await serial string "MCM,4"
  {
    char currentBuffer[7];
    String currentString = dtostrf(maxCurr, 7, 2, currentBuffer);
    currentString.trim();
    int currentLength = currentString.length();
    if (currentLength < 5)
    {
      Serial.print("MCMR4,0");
    }
    else if (currentLength >= 5)
    {
      Serial.print("MVMR4,");
    }
    currentString.replace('.', ',');
    Serial.print(currentString);
    Serial.print('\n');
    incomingString = "";
    currentString = "";
  }
}

//reset CURRENT stores
void resetCurrStores() {
  if (incomingString == "MCM,5") {
    memset(currreadings, 0, sizeof(currreadings));
    averageCurr = 0;
    minCurr = 1023;
    maxCurr = 0;
    totalCurr =  0;
    if (currreadings > 0 && averageCurr == 0 && minCurr == 1023 && maxCurr == 0 
    && totalCurr ==  0) {
      Serial.print("MCMR5,01");
      Serial.print('\n');
      incomingString = "";
    }
    else if (currreadings > 0) {
      Serial.print("MCMR5,00");
      Serial.print('\n');
      incomingString = "";
    }
  }
}

/**************************************************************************/
/*
    Light sensor read every 100ms
*/
/**************************************************************************/
void checkLightSensors() {
  //reads light sensor
  unsigned long currentMillisLight = millis();
  if (currentMillisLight - previousMillisLight > intervalLight) {
    previousMillisLight = currentMillisLight;
    // Read a value from the light sesnor
    sensors_event_t event;
    tsl.getEvent(&event);
    sensorValueLight = event.light;

    // subtract the last reading:
    totalLight = totalLight - lightreadings[readLightIndex];
    // read from the sensor:
    lightreadings[readLightIndex] = (sensorValueLight);
    // add the reading to the total:
    totalLight = totalLight + lightreadings[readLightIndex];

    // calc min/max LIGHT
    if (lightreadings[readLightIndex] > maxLight) maxLight = lightreadings[readLightIndex];
    if (lightreadings[readLightIndex] < minLight) minLight = lightreadings[readLightIndex];

    // advance to the next position in the array:
    readLightIndex = readLightIndex + 1;
    // if we're at the end of the array...
    if (readLightIndex >= lightReadings) {
      // ...wrap around to the beginning:
      readLightIndex = 0;
    }
    averageLight = totalLight / lightReadings;
    //Serial.print("MINLIGHT: "); Serial.print(minLight); Serial.print("  ||  ");
    //Serial.print("MAXLIGHT: ");Serial.print(maxLight); Serial.println("");
    //Serial.print("Light readings: "); Serial.print(lightReadings); Serial.print("  ||  ");
    //Serial.print("Total light: "); Serial.print(totalLight); Serial.print("  ||  ");
    //Serial.print("Average light: "); Serial.print(averageLight);Serial.println("");
  }
}

/**************************************************************************/
/*
    Light sensor serial read
*/
/**************************************************************************/
// Waits for "MLM,1" over serial then reads light sensor and converts to 
// "MVLMR1,xx,yy"
void checkLightSerial() {
  if (incomingString == "MLM,1") // await serial string == MLM,1
  {
    // Output value  to the serial monitor to 2 decimal
    // places and check length of string and formats
    char lightBuffer[7];
    String lightString = dtostrf(sensorValueLight, 7, 2, lightBuffer);
    lightString.trim();
    int lightLength = lightString.length();
    if (lightLength < 5)
    {
      Serial.print("MLMR1,0");
    }
    else if (lightLength >= 5)
    {
      Serial.print("MLMR1,");
    }
    lightString.replace('.', ',');
    Serial.print(lightString);
    Serial.print('\n');
    incomingString = "";
    lightString = "";
  }

//Light sensor average
// Waits for "MLM,2" over serial then reads light sensor and calculates average 
// then converts to "MLMR2,xx,yy"
  else if (incomingString == "MLM,2") // await serial string "MLM,2"
  {
    char lightBuffer[7];
    String lightString = dtostrf(averageLight, 7, 2, lightBuffer);
    lightString.trim();
    int lightLength = lightString.length();
    if (lightLength < 5)
    {
      Serial.print("MLMR2,0");
    }
    else if (lightLength >= 5)
    {
      Serial.print("MLMR2,");
    }
    lightString.replace('.', ',');
    Serial.print(lightString);
    Serial.print('\n');
    incomingString = "";
    lightString = "";
  }

//Light sensor min
// Waits for "MLM,3" over serial then reads minimum light sensor value and
// converts to "MLMR3,xx,yy"
  else if (incomingString == "MLM,3") // await serial string "MLM,3"
  {
    char lightBuffer[7];
    String lightString = dtostrf(minLight, 7, 2, lightBuffer);
    lightString.trim();
    int lightLength = lightString.length();
    if (lightLength < 5)
    {
      Serial.print("MLMR3,0");
    }
    else if (lightLength >= 5)
    {
      Serial.print("MLMR3,");
    }
    lightString.replace('.', ',');
    Serial.print(lightString);
    Serial.print('\n');
    incomingString = "";
    lightString = "";
  }

// Light sensor max
// Waits for "MLM,4" over serial then reads maximum light sensor value and
// converts to "MLMR4,xx,yy" 
  else if (incomingString == "MLM,4") // await serial string "MLM,4"
  {
    char lightBuffer[7];
    String lightString = dtostrf(maxLight, 7, 2, lightBuffer);
    lightString.trim();
    int lightLength = lightString.length();
    if (lightLength < 5)
    {
      Serial.print("MLMR4,0");
    }
    else if (lightLength >= 5)
    {
      Serial.print("MLMR4,");
    }
    lightString.replace('.', ',');
    Serial.print(lightString);
    Serial.print('\n');
    incomingString = "";
    lightString = "";
  }
}

/**************************************************************************/
/*
    Light sensor reset stores
*/
/**************************************************************************/
void resetLightStores() {
  if (incomingString == "MLM,5") {
    memset(lightreadings, 0, sizeof(lightreadings));
    averageLight = 0;
    minLight = 1023;
    maxLight = 0;
    totalLight =  0;

    if (lightreadings > 0 && averageLight == 0 && minLight == 1023 && maxLight == 0 
    && totalLight ==  0) {
      Serial.print("MLMR5,01");
      Serial.print('\n');
      incomingString = "";
    }
    else if (lightreadings > 0) {
      Serial.print("MLMR5,00");
      Serial.print('\n');
      incomingString = "";
    }
  }
}

/**************************************************************************/
/*
    FFT read serial
*/
/**************************************************************************/
void checkFFTSerial() {
   // while(1) { // reduces jitter

  if (incomingString == "FFT1") // await serial string == FTT1
  {
    cli();  // UDRE interrupt slows this way down on arduino1.0

    for (int i = 0 ; i < 512 ; i += 2)
    { // save 256 samples
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      //  k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i + 1] = 0; // set odd bins to 0
    }

    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft

    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      Result_1[i] = fft_log_out[i]; // Save the data in a temporary store
    }

    for (int i = 0 ; i < 512 ; i += 2)
    { // save 256 samples
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      //  k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i + 1] = 0; // set odd bins to 0
    }

    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft

    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      Result_2[i] = fft_log_out[i]; // Save the data in a temporary store
    }

    for (int i = 0 ; i < 512 ; i += 2)
    { // save 256 samples
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      //  k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i + 1] = 0; // set odd bins to 0
    }

    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft

    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      Result_3[i] = fft_log_out[i]; // Save the data in a temporary store
    }

    //Average the data
    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      Answer[i] = Result_1[i];
      if (Result_2[i] <  Answer[i])
      {
        Answer[i] = Result_2[i];
      }
      if (Result_3[i] <  Answer[i])
      {
        Answer[i] = Result_3[i];
      }
    }
    sei();
    Serial.println("[start]");
    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      // Serial.print(fft_log_out[i]); // send out the data
      Serial.print(Answer[i]); // send out the data
      Serial.println(",");
    }
    Serial.println("[end]");
    incomingString = "";
  }
  else if (incomingString == "FFT2") // await serial string == FTT2
  {
    while (temp_volume < 30)
    {
      temp_volume = 0;
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      k <<= 6; // form into a 16b signed int
      temp_volume += k;

      if (temp_volume < 12300)
      {
        temp_volume = 12300 - temp_volume;
      }
      else
      {
        temp_volume = temp_volume - 12300;
      }
      temp_volume = temp_volume  >> 6;
    }

    cli();  // UDRE interrupt slows this way down on arduino1.0

    for (int i = 0 ; i < 1000 ; i += 1)
    {
      temp_volume = 0;
      for (int t = 0; t < 36 ; t += 1)
      {
        //while (!(ADCSRA & 0x10)); // wait for adc to be ready
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        ADCSRA = 0xf5; // restart adc
        int k = (j << 8) | m; // form into an int
        k <<= 6; // form into a 16b signed int
        temp_volume += k;
      }
      temp_volume = temp_volume / 36;
      if (temp_volume < 12300)
      {
        temp_volume = 12300 - temp_volume;
      }
      else
      {
        temp_volume = temp_volume - 12300;
      }
      Volume[i] = temp_volume >> 6;
    }
    sei();

    Serial.println("[start]");
    for (int i = 0 ; i < 1000 ; i++)
    {
      Serial.print(i);
      Serial.print(",");
      Serial.print(Volume[i]); // send out the data
      Serial.println(",");
    }
    Serial.println("[end]");
    incomingString = "";
  }

  else  if (incomingString == "FFT3") // await serial string == FTT3
  {
    temp_volume = 0;
    while (temp_volume < 30)
    {
      temp_volume = 0;
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      k <<= 6; // form into a 16b signed int
      temp_volume += k;

      if (temp_volume < 12300)
      {
        temp_volume = 12300 - temp_volume;
      }
      else
      {
        temp_volume = temp_volume - 12300;
      }
      temp_volume = temp_volume  >> 6;
    }

    cli();  // UDRE interrupt slows this way down on arduino1.0

    for (int i = 0 ; i < 512 ; i += 2)
    { // save 256 samples
      //while (!(ADCSRA & 0x10)); // wait for adc to be ready
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      ADCSRA = 0xf5; // restart adc
      int k = (j << 8) | m; // form into an int
      //  k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i + 1] = 0; // set odd bins to 0
    }

    temp_volume = 0;
    for (int i = 0 ; i < 72 ; i += 2)
    {
      temp_volume += fft_input[i];
    }
    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[0] = temp_volume >> 6;


    temp_volume = 0;
    for (int i = 72 ; i < 144 ; i += 2)
    {
      temp_volume += fft_input[i];
    }
    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[1] = temp_volume >> 6;

    temp_volume = 0;
    for (int i = 144 ; i < 216 ; i += 2)
    {
      temp_volume += fft_input[i];
    }

    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[2] = temp_volume >> 6;


    temp_volume = 0;
    for (int i = 216 ; i < 288 ; i += 2)
    {
      temp_volume += fft_input[i];
    }

    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[3] = temp_volume >> 6;

    temp_volume = 0;
    for (int i = 288 ; i < 360 ; i += 2)
    {
      temp_volume += fft_input[i];
    }

    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[4] = temp_volume >> 6;

    temp_volume = 0;
    for (int i = 360 ; i < 432 ; i += 2)
    {
      temp_volume += fft_input[i];
    }

    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[5] = temp_volume >> 6;

    temp_volume = 0;
    for (int i = 432 ; i < 504 ; i += 2)
    {
      temp_volume += fft_input[i];
    }

    temp_volume = temp_volume / 36;
    if (temp_volume < 12300)
    {
      temp_volume = 12300 - temp_volume;
    }
    else
    {
      temp_volume = temp_volume - 12300;
    }
    Volume[6] = temp_volume >> 6;

    for (int i = 7 ; i < 1000 ; i += 1)
    {
      temp_volume = 0;
      for (int t = 0; t < 36 ; t += 1)
      {
        //while (!(ADCSRA & 0x10)); // wait for adc to be ready
        byte m = ADCL; // fetch adc data
        byte j = ADCH;
        ADCSRA = 0xf5; // restart adc
        int k = (j << 8) | m; // form into an int
        k <<= 6; // form into a 16b signed int
        temp_volume += k;
      }
      temp_volume = temp_volume / 36;
      if (temp_volume < 12300)
      {
        temp_volume = 12300 - temp_volume;
      }
      else
      {
        temp_volume = temp_volume - 12300;
      }
      Volume[i] = temp_volume >> 6;
    }

    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft

    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      Result_3[i] = fft_log_out[i]; // Save the data in a temporary store
    }

    sei();

    Serial.println("[start]");
    for (byte i = 0 ; i < FFT_N / 2 ; i++)
    {
      // Serial.print(fft_log_out[i]); // send out the data
      Serial.print(Result_3[i]); // send out the data
      Serial.println(",");
    }

    Serial.println("[mid]");
    for (int i = 0 ; i < 1000 ; i++)
    {
      Serial.print(i);
      Serial.print(",");
      Serial.print(Volume[i]); // send out the data
      Serial.println(",");
    }
    Serial.println("[end]");
    incomingString = "";
  }
}

/**************************************************************************/
/*
    main loop
*/
/**************************************************************************/
void loop()
{
  sleepCheck();           // checks and sets sleep mode
  resetArduino();         // checks if arduino needs resetting
  checkVoltageSensors();  // checks voltage sensors every 100ms
  checkCurrentSensors();  // checks current sensors every 100ms
  checkLightSensors();    // checks light sensors every 100ms
    if (Serial.available() > 0)     // checks serial has data
      {
      char incomingByte = (char)Serial.read();  //Sets incoming byte variable
      incomingString += incomingByte;           //Sets incoming string variable
      checkVoltageSerial();                     //checks for voltage commands over serial
      resetVoltStores();                        //checks if voltage stores need resetting
      checkCurrentSerial();                     //checks for current commands over serial
      resetCurrStores();                        //checks if current stores need resetting
      checkLightSerial();                       //checks for light commands over serial
      resetLightStores();                       //checks if light stores need resetting
      checkFFTSerial();                         //checks for FFT commands over serial
      }
}

