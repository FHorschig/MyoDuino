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

void setup()
{
  ble_set_name("MyoRobotArm");

  // Init. and start BLE library.
  ble_begin();

  // Enable serial debug
  Serial.begin(9600);

  pinMode(MOTOR_A_1, OUTPUT);
  pinMode(MOTOR_A_2, OUTPUT);

  digitalWrite(MOTOR_A_1, LOW);
  digitalWrite(MOTOR_A_2, LOW);
}

void controlMotor(byte data) {
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
    break;
    case GESTURE_FIST:
      Serial.println("Gesture: Fist");
    break;
    case GESTURE_SPREAD:
      Serial.println("Gesture: Spread Fingers");
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

void loop()
{
  static boolean transmit_light_sensor = false;

  // If data is ready
  while(ble_available())
  {
    // read out command and data
    byte op_code = ble_read();
    byte data1 = ble_read();
    byte data2 = ble_read();

        digitalWrite(MOTOR_A_1, LOW);
        digitalWrite(MOTOR_A_2, LOW);
    if (op_code == CMD_MOTOR_CONTROL)
    {
      Serial.println("Motor control OP code");
      controlMotor(data1);
    }
    else if (op_code == CMD_SET_LIGHT_TRANSMIT)
    {
      Serial.println("Light Transmit OP code");
      transmit_light_sensor = (data1 == TRUE);
    }
    else if (op_code == CMD_NEW_GESTURE)
    {
      Serial.println("New Gesture OP code");
      // TODO(fhorschig): Handle data1 and data2 accordingly.
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
    uint16_t value = analogRead(ANALOG_IN_PIN);
    ble_write(OUT_LIGHT_TRANSMIT);
    ble_write(value >> 8);
    ble_write(value);
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



