/* Copyright (C) 2006 Hans-Christoph Steiner 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 *
 * -----------------------------
 * Firmata, the Arduino firmware
 * -----------------------------
 * 
 * Firmata turns the Arduino into a Plug-n-Play sensorbox, servo
 * controller, and/or PWM motor/lamp controller.
 *
 * It was originally designed to work with the Pd object [arduino]
 * which is included in Pd-extended.  This firmware is intended to
 * work with any host computer software package.  It can easily be
 * used with other programs like Max/MSP, Processing, or whatever can
 * do serial communications.
 *
 * @authors: Hans-Christoph Steiner <hans@at.or.at>
 *   help with protocol redesign: Jamie Allen <jamie@heavyside.net>
 *   key bugfixes: Georg Holzmann <grh@mur.at>
 *                 Gerda Strobl <gerda.strobl@student.tugraz.at>
 * @date: 2006-05-19
 * @locations: STEIM, Amsterdam, Netherlands
 *             IDMI/Polytechnic University, Brookyn, NY, USA
 *             Electrolobby Ars Electronica, Linz, Austria
 */

/* 
 * TODO: add pulseOut functionality
 * TODO: add software PWM for servos, etc (servo.h or pulse.h)
 * TODO: redesign protocol to accomodate boards with more I/Os
 * TODO: 
 * TODO: add "pinMode all 0/1" command
 * TODO: add cycle markers to mark start of analog, digital, pulseIn, and PWM
 */

/* firmware version numbers.  The protocol is still changing, so these version
 * numbers are important */
#define MAJOR_VERSION 0
#define MINOR_VERSION 1

/* firmata protocol
 * =============== 
 * data: 0-127   
 * control: 128-255
 */
  
/* computer<->Arduino commands
 * -------------------- */
/* 128-129 // UNUSED */
#define SET_PIN_ZERO_TO_IN      130 // set digital pin 0 to INPUT
#define SET_PIN_ONE_TO_IN       131 // set digital pin 1 to INPUT
#define SET_PIN_TWO_TO_IN       132 // set digital pin 2 to INPUT
#define SET_PIN_THREE_TO_IN     133 // set digital pin 3 to INPUT
#define SET_PIN_FOUR_TO_IN      134 // set digital pin 4 to INPUT
#define SET_PIN_FIVE_TO_IN      135 // set digital pin 5 to INPUT
#define SET_PIN_SIX_TO_IN       136 // set digital pin 6 to INPUT
#define SET_PIN_SEVEN_TO_IN     137 // set digital pin 7 to INPUT
#define SET_PIN_EIGHT_TO_IN     138 // set digital pin 8 to INPUT
#define SET_PIN_NINE_TO_IN      139 // set digital pin 9 to INPUT
#define SET_PIN_TEN_TO_IN       140 // set digital pin 10 to INPUT
#define SET_PIN_ELEVEN_TO_IN    141 // set digital pin 11 to INPUT
#define SET_PIN_TWELVE_TO_IN    142 // set digital pin 12 to INPUT
#define SET_PIN_THIRTEEN_TO_IN  143 // set digital pin 13 to INPUT
/* 144-149 // UNUSED */
#define DISABLE_DIGITAL_INPUTS  150 // disable reporting of digital inputs
#define ENABLE_DIGITAL_INPUTS   151 // enable reporting of digital inputs
/* 152-159 // UNUSED */
#define ZERO_ANALOG_INS         160 // disable reporting on all analog ins
#define ONE_ANALOG_IN           161 // enable reporting for 1 analog in (0)
#define TWO_ANALOG_INS          162 // enable reporting for 2 analog ins (0,1)
#define THREE_ANALOG_INS        163 // enable reporting for 3 analog ins (0-2)
#define FOUR_ANALOG_INS         164 // enable reporting for 4 analog ins (0-3)
#define FIVE_ANALOG_INS         165 // enable reporting for 5 analog ins (0-4)
#define SIX_ANALOG_INS          166 // enable reporting for 6 analog ins (0-5)
#define SEVEN_ANALOG_INS        167 // enable reporting for 6 analog ins (0-6)
#define EIGHT_ANALOG_INS        168 // enable reporting for 6 analog ins (0-7)
#define NINE_ANALOG_INS         169 // enable reporting for 6 analog ins (0-8)
/* 167-199 // UNUSED */
#define SET_PIN_ZERO_TO_OUT     200 // set digital pin 0 to OUTPUT
#define SET_PIN_ONE_TO_OUT      201 // set digital pin 1 to OUTPUT
#define SET_PIN_TWO_TO_OUT      202 // set digital pin 2 to OUTPUT
#define SET_PIN_THREE_TO_OUT    203 // set digital pin 3 to OUTPUT
#define SET_PIN_FOUR_TO_OUT     204 // set digital pin 4 to OUTPUT
#define SET_PIN_FIVE_TO_OUT     205 // set digital pin 5 to OUTPUT
#define SET_PIN_SIX_TO_OUT      206 // set digital pin 6 to OUTPUT
#define SET_PIN_SEVEN_TO_OUT    207 // set digital pin 7 to OUTPUT
#define SET_PIN_EIGHT_TO_OUT    208 // set digital pin 8 to OUTPUT
#define SET_PIN_NINE_TO_OUT     209 // set digital pin 9 to OUTPUT
#define SET_PIN_TEN_TO_OUT      210 // set digital pin 10 to OUTPUT
#define SET_PIN_ELEVEN_TO_OUT   211 // set digital pin 11 to OUTPUT
#define SET_PIN_TWELVE_TO_OUT   212 // set digital pin 12 to OUTPUT
#define SET_PIN_THIRTEEN_TO_OUT 213 // set digital pin 13 to OUTPUT
/* 214-228 // UNUSED */
#define OUTPUT_TO_DIGITAL_PINS  229 // next two bytes set digital output data 
/* 230-239 // UNUSED */
#define REPORT_VERSION          240 // return the firmware version
/* 240-249 // UNUSED */
#define DISABLE_PWM             250 // next byte sets pin # to disable
#define ENABLE_PWM              251 // next two bytes set pin # and duty cycle
#define DISABLE_SOFTWARE_PWM    252 // next byte sets pin # to disable
#define ENABLE_SOFTWARE_PWM     253 // next two bytes set pin # and duty cycle
#define SET_SOFTWARE_PWM_FREQ   254 // set master frequency for software PWMs
/* 255 // UNUSED */

 
/* two byte digital output data format
 * ----------------------
 * 0  get ready for digital input bytes (229)
 * 1  digitalOut 7-13 bitmask 
 * 2  digitalOut 0-6 bitmask
 */

/* two byte PWM data format
 * ----------------------
 * 0  get ready for digital input bytes (ENABLE_SOFTWARE_PWM/ENABLE_PWM)
 * 1  pin #
 * 2  duty cycle expressed as 1 byte (255 = 100%)
 */

/* digital input message format
 * ----------------------
 * 0   digital input marker (255/11111111)
 * 1   digital read from Arduino // 7-13 bitmask
 * 2   digital read from Arduino // 0-6 bitmask
 */

/* analog input message format
 * ----------------------
 * 0   analog input marker (160 + number of pins to report)
 * 1   high byte from analog input pin 0 
 * 2   low byte from analog input pin 0
 * 3   high byte from analog input pin 1 
 * 4   low byte from analog input pin 1
 * 5   high byte from analog input pin 2 
 * 6   low byte from analog input pin 2
 * 7   high byte from analog input pin 3 
 * 8   low byte from analog input pin 3
 * 9   high byte from analog input pin 4 
 * 10  low byte from analog input pin 4
 * 11  high byte from analog input pin 5 
 * 12  low byte from analog input pin 5
 */

#define TOTAL_DIGITAL_PINS 14

// for comparing along with INPUT and OUTPUT
#define PWM 2

// maximum number of post-command data bytes
#define MAX_DATA_BYTES 2
// this flag says the next serial input will be data
byte waitForData = 0;
byte executeMultiByteCommand = 0; // command to execute after getting multi-byte data
byte storedInputData[MAX_DATA_BYTES] = {0,0}; // multi-byte data

// this flag says the first data byte for the digital outs is next
boolean firstInputByte = false;

/* this int serves as a bit-wise array to store pin status
 * 0 = INPUT, 1 = OUTPUT
 */
int digitalPinStatus;

/* this byte stores the status off whether PWM is on or not
 * bit 9 = PWM0, bit 10 = PWM1, bit 11 = PWM2
 * the rest of the bits are unused and should remain 0
 */
int pwmStatus;

/* this byte stores the status of whether software PWM is on or not */
/* 00000010 00000000 means bit 10 is softWarePWM enabled */
int softPwmStatus;

boolean digitalInputsEnabled = true;
byte analogInputsEnabled = 6;

byte analogPin;
int analogData;

// -------------------------------------------------------------------------
void transmitDigitalInput(byte startPin) {
  byte i;
  byte digitalPin;
//  byte digitalPinBit;
  byte transmitByte = 0;
  byte digitalData;

  for(i=0;i<7;++i) {
    digitalPin = i+startPin;
/*    digitalPinBit = OUTPUT << digitalPin;
// only read the pin if its set to input
if(digitalPinStatus & digitalPinBit) {
digitalData = 0; // pin set to OUTPUT, don't read
}
else if( (digitalPin >= 9) && (pwmStatus & (1 << digitalPin)) ) {
digitalData = 0; // pin set to PWM, don't read
}*/
    if( !(digitalPinStatus & (1 << digitalPin)) ) {
      digitalData = (byte) digitalRead(digitalPin);
      transmitByte = transmitByte + ((1 << i) * digitalData);
    }
  }
  printByte(transmitByte);
}



// -------------------------------------------------------------------------
/* this function sets the pin mode to the correct state and sets the relevant
 * bits in the two bit-arrays that track Digital I/O and PWM status
 */
void setPinMode(int pin, int mode) {
  if(mode == INPUT) {
    digitalPinStatus = digitalPinStatus &~ (1 << pin);
    pwmStatus = pwmStatus &~ (1 << pin);
    pinMode(pin,INPUT);
  }
  else if(mode == OUTPUT) {
    digitalPinStatus = digitalPinStatus | (1 << pin);
    pwmStatus = pwmStatus &~ (1 << pin);
    pinMode(pin,OUTPUT);
  }
  else if( (mode == PWM) && (pin >= 9) && (pin <= 11) ) {
    digitalPinStatus = digitalPinStatus | (1 << pin);
    pwmStatus = pwmStatus | (1 << pin);
    pinMode(pin,OUTPUT);
  }
}

void setSoftPwm (int pin, byte pulsePeriod) {
  byte i;
  /*    for(i=0; i<7; ++i) {
      mask = 1 << i;
      if(digitalPinStatus & mask) {
      digitalWrite(i, inputData & mask);
      } 
      }
  */    
  //read timer type thing

  //loop through each pin, turn them on if selected
  //softwarePWMStatus
  //check timer type thing against pulsePeriods for each pin 
  //throw pin low if expired
}

void setSoftPwmFreq(byte freq) {
}


void disSoftPwm(int pin) {
  //throw pin low
    
}


/* -------------------------------------------------------------------------
 * this function checks to see if there is data waiting on the serial port 
 * then processes all of the stored data
 */
void checkForInput() {
  if(Serial.available()) {  
    while(Serial.available()) {
      processInput( (byte)Serial.read() );
    }
  }
}

/* -------------------------------------------------------------------------
 * processInput() is called whenever a byte is available on the
 * Arduino's serial port.  This is where the commands are handled.
 */
void processInput(byte inputData) {
  int i;
  int mask;
  
  // a few commands have byte(s) of data following the command
  if( waitForData > 0) {  
    waitForData--;
    storedInputData[waitForData] = inputData;

    if(executeMultiByteCommand && (waitForData==0)) {
      //we got everything
      switch(executeMultiByteCommand) {
      case ENABLE_PWM:
        setPinMode(storedInputData[1],PWM);
        analogWrite(storedInputData[1], storedInputData[0]);
        break;
      case DISABLE_PWM:
        setPinMode(storedInputData[0],INPUT);
        break;
      case ENABLE_SOFTWARE_PWM:
        setPinMode(storedInputData[1],PWM);
        setSoftPwm(storedInputData[1], storedInputData[0]);     
        break; 
      case DISABLE_SOFTWARE_PWM:
        disSoftPwm(storedInputData[0]);
        break;
      case SET_SOFTWARE_PWM_FREQ:
        setSoftPwmFreq(storedInputData[0]);
        break;
      }
      executeMultiByteCommand = 0;
    }
  }
  else if(inputData < 128) {
    if(firstInputByte) {
      // output data for pins 7-13
      for(i=7; i<TOTAL_DIGITAL_PINS; ++i) {
        mask = 1 << i;
        if( (digitalPinStatus & mask) && !(pwmStatus & mask) ) {
          // inputData is a byte and mask is an int, so align the high part of mask
          digitalWrite(i, inputData & (mask >> 7));
        }        
      }
      firstInputByte = false;
    }
    else { //
      for(i=0; i<7; ++i) {
        mask = 1 << i;
        if( (digitalPinStatus & mask) && !(pwmStatus & mask) ) {
          digitalWrite(i, inputData & mask);
        } 
      }
    }
  }
  else {
    switch (inputData) {
    case SET_PIN_ZERO_TO_IN:       // set digital pins to INPUT
    case SET_PIN_ONE_TO_IN:
    case SET_PIN_TWO_TO_IN:
    case SET_PIN_THREE_TO_IN:
    case SET_PIN_FOUR_TO_IN:
    case SET_PIN_FIVE_TO_IN:
    case SET_PIN_SIX_TO_IN:
    case SET_PIN_SEVEN_TO_IN:
    case SET_PIN_EIGHT_TO_IN:
    case SET_PIN_NINE_TO_IN:
    case SET_PIN_TEN_TO_IN:
    case SET_PIN_ELEVEN_TO_IN:
    case SET_PIN_TWELVE_TO_IN:
    case SET_PIN_THIRTEEN_TO_IN:
      setPinMode(inputData - SET_PIN_ZERO_TO_IN, INPUT);
      break;
    case SET_PIN_ZERO_TO_OUT:      // set digital pins to OUTPUT
    case SET_PIN_ONE_TO_OUT:
    case SET_PIN_TWO_TO_OUT:
    case SET_PIN_THREE_TO_OUT:
    case SET_PIN_FOUR_TO_OUT:
    case SET_PIN_FIVE_TO_OUT:
    case SET_PIN_SIX_TO_OUT:
    case SET_PIN_SEVEN_TO_OUT:
    case SET_PIN_EIGHT_TO_OUT:
    case SET_PIN_NINE_TO_OUT:
    case SET_PIN_TEN_TO_OUT:
    case SET_PIN_ELEVEN_TO_OUT:
    case SET_PIN_TWELVE_TO_OUT:
    case SET_PIN_THIRTEEN_TO_OUT:
      setPinMode(inputData - SET_PIN_ZERO_TO_OUT, OUTPUT);
      break;
    case DISABLE_DIGITAL_INPUTS:   // all digital inputs off
      digitalInputsEnabled = false;
      break;
    case ENABLE_DIGITAL_INPUTS:    // all digital inputs on
      digitalInputsEnabled = true;
      break;
    case ZERO_ANALOG_INS:   // analog input off
    case ONE_ANALOG_IN:     // analog 0 on  
    case TWO_ANALOG_INS:    // analog 0,1 on  
    case THREE_ANALOG_INS:  // analog 0-2 on  
    case FOUR_ANALOG_INS:   // analog 0-3 on  
    case FIVE_ANALOG_INS:   // analog 0-4 on  
    case SIX_ANALOG_INS:    // analog 0-5 on  
    case SEVEN_ANALOG_INS:  // analog 0-6 on  
    case EIGHT_ANALOG_INS:  // analog 0-7 on  
    case NINE_ANALOG_INS:   // analog 0-8 on  
	  analogInputsEnabled = inputData - ZERO_ANALOG_INS;
	  break;
    case ENABLE_PWM:
      waitForData = 2;  // 2 bytes needed (pin#, dutyCycle) 
      executeMultiByteCommand = inputData;
      break;
    case DISABLE_PWM:
      waitForData = 1;  // 1 byte needed (pin#)
      executeMultiByteCommand = inputData;
      break;      
    case SET_SOFTWARE_PWM_FREQ:
      waitForData = 1;  // 1 byte needed (pin#)
      executeMultiByteCommand = inputData;
      break;
    case ENABLE_SOFTWARE_PWM:
      waitForData = 2;  // 2 bytes needed (pin#, dutyCycle) 
      executeMultiByteCommand = inputData;
      break;
    case DISABLE_SOFTWARE_PWM:
      waitForData = 1;  // 1 byte needed (pin#)
      executeMultiByteCommand = inputData;
      break;
    case OUTPUT_TO_DIGITAL_PINS:   // bytes to send to digital outputs
      firstInputByte = true;
      break;
    case REPORT_VERSION:
      printByte(REPORT_VERSION);
      printByte(MAJOR_VERSION);
      printByte(MINOR_VERSION);
      break;
    }
  }
}


// =========================================================================

// -------------------------------------------------------------------------
void setup() {
  byte i;

  Serial.begin(19200);
  for(i=0; i<TOTAL_DIGITAL_PINS; ++i) {
    setPinMode(i,INPUT);
  }
}

// -------------------------------------------------------------------------
void loop() {
  checkForInput();  
  
  // read all digital pins, in enabled
  if(digitalInputsEnabled) {
    printByte(ENABLE_DIGITAL_INPUTS);
    transmitDigitalInput(7);
    checkForInput();
    transmitDigitalInput(0);
    checkForInput();
  }

  /* get analog in, for the number enabled */
  for(analogPin=0; analogPin<analogInputsEnabled; ++analogPin) {
    analogData = analogRead(analogPin);
    // these two bytes get converted back into the whole number in Pd
    // the higher bits should be zeroed so that the 8th bit doesn't get set
    printByte(ONE_ANALOG_IN + analogPin); 
    printByte(analogData >> 7);  // bitshift the big stuff into the output byte
    printByte(analogData % 128); // mod by 32 for the small byte
    checkForInput();
  }
}
