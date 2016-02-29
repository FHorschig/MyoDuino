/*

Copyright (c) 2012, 2013 RedBearLab

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

//"RBL_nRF8001.h/spi.h/boards.h" is needed in every new project
#include <SPI.h>
#include <EEPROM.h>
#include <boards.h>
#include <RBL_nRF8001.h>

#define MOTOR_A_1    6
#define MOTOR_A_2    7
#define ANALOG_IN_PIN A5

#define CMD_SET_LIGHT_TRANSMIT 0xA0
#define CMD_MOTOR_CONTROL 0x01
#define CMD_NEW_GESTURE 0x02
#define CMD_RESET 0x04

#define OUT_LIGHT_TRANSMIT 0x0B

#define TRUE 0x01
#define FALSE 0x00

#define GESTURE_NONE 0x00
#define GESTURE_FIST 0x01
#define GESTURE_SPREAD 0x02
#define GESTURE_WAVE_IN 0x03
#define GESTURE_WAVE_OUT 0x04

#define MOTOR_CLOCKWISE 0x01
#define MOTOR_COUNTERCLOCK 0x02


#define CLOSING_THRESHOLD 50

uint16_t light_sensor;
boolean can_close = false;

#include <NilTimer1.h>
#include <NilSerial.h>
#define Serial NilSerial // Redefine Serial as NilSerial to save RAM.

// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waBleReceivingThread, 64);

// Declare thread function for thread 1.
NIL_THREAD(BleReceivingThread, arg) {
  // Init. and start BLE library.
  ble_begin();

  while (TRUE) {
    static boolean transmit_light_sensor = false;

    // If data is ready
    while (ble_available())
    {
      // read out command and data
      byte op_code = ble_read();
      byte data1 = ble_read();
      byte data2 = ble_read();

      if (op_code == CMD_MOTOR_CONTROL)
      {
        Serial.println("Motor control OP code");
        manualMotorControl(data1);
      }
      else if (op_code == CMD_SET_LIGHT_TRANSMIT)
      {
        Serial.println("Light Transmit OP code");
        transmit_light_sensor = (data1 == TRUE);
      }
      else if (op_code == CMD_NEW_GESTURE)
      {
        Serial.println("New Gesture OP code");
        handleGesture(data1);
      }
      else if (op_code == CMD_RESET)
      {
        transmit_light_sensor = false;
        digitalWrite(MOTOR_A_1, LOW);
        digitalWrite(MOTOR_A_2, LOW);
      }
    }

    if (transmit_light_sensor)  // if analog reading enabled
    {
      ble_write(OUT_LIGHT_TRANSMIT);
      ble_write(light_sensor >> 8); // Even if values change: this transmission
      ble_write(light_sensor);      // is not important enough to get a mutex.
    }

    if (!ble_connected())
    {
      transmit_light_sensor = false;
      digitalWrite(MOTOR_A_1, LOW);
      digitalWrite(MOTOR_A_2, LOW);
    }

    // Allow BLE Shield to send/receive data
    ble_do_events();
  }
}
// Declare a stack with 16 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waSensorReadingThread, 16);

// Declare thread function for thread 2.
NIL_THREAD(SensorReadingThread, arg) {
  nilTimer1Start(400000);  // Execute while loop every 0.4 seconds.

  while (TRUE) {
    light_sensor = analogRead(ANALOG_IN_PIN);
    can_close = light_sensor > CLOSING_THRESHOLD;
    if (!can_close)
      digitalWrite(MOTOR_A_2, LOW); // A_2 is only high if closing right now.
    nilTimer1Wait();
  }
}
//------------------------------------------------------------------------------
/*
 * Threads static table, one entry per thread.  A thread's priority is
 * determined by its position in the table with highest priority first.
 *
 * These threads start with a null argument.  A thread's name is also
 * null to save RAM since the name is currently not used.
 */
NIL_THREADS_TABLE_BEGIN()
NIL_THREADS_TABLE_ENTRY(NULL, SensorReadingThread,
                        NULL, waSensorReadingThread,
                        sizeof(waSensorReadingThread))
NIL_THREADS_TABLE_ENTRY(NULL, BleReceivingThread,
                        NULL, waBleReceivingThread,
                        sizeof(waBleReceivingThread))
NIL_THREADS_TABLE_END()
//------------------------------------------------------------------------------

void setup()
{
  Serial.begin(9600);

  pinMode(MOTOR_A_1, OUTPUT);
  pinMode(MOTOR_A_2, OUTPUT);

  digitalWrite(MOTOR_A_1, LOW);
  digitalWrite(MOTOR_A_2, LOW);

  // start kernel
  nilSysBegin();
}

void manualMotorControl(byte data) {
  if (data == MOTOR_CLOCKWISE) {
    digitalWrite(MOTOR_A_1, HIGH);
    digitalWrite(MOTOR_A_2, LOW);
  } else if (data == MOTOR_COUNTERCLOCK) {
    digitalWrite(MOTOR_A_1, LOW);
    digitalWrite(MOTOR_A_2, HIGH);
  } else {
    digitalWrite(MOTOR_A_1, LOW);
    digitalWrite(MOTOR_A_2, LOW);
  }
}

void handleGesture(byte gesture) {
  switch (gesture) {
    case GESTURE_NONE:
      Serial.println("No Gesture");
      digitalWrite(MOTOR_A_1, LOW);
      digitalWrite(MOTOR_A_2, LOW);
    break;
    case GESTURE_FIST:
      Serial.println("Gesture: Fist");
      digitalWrite(MOTOR_A_1, LOW);
      if (can_close)
        digitalWrite(MOTOR_A_2, HIGH); // A_2 is only high if closing right now.
    break;
    case GESTURE_SPREAD:
      Serial.println("Gesture: Spread Fingers");
      digitalWrite(MOTOR_A_2, LOW);
      digitalWrite(MOTOR_A_1, HIGH);
    break;
    case GESTURE_WAVE_IN:
      Serial.println("Gesture: Wave In");
    break;
    case GESTURE_WAVE_OUT:
      Serial.println("Gesture: Wave Out");
    break;
    default:
      Serial.println("Unknown Gesture");
  }
}

//------------------------------------------------------------------------------
// Loop is the idle thread.  The idle thread must not invoke any
// kernel primitive able to change its state to not runnable.
void loop() {
  // Not used but must exist in order to compile.
}



