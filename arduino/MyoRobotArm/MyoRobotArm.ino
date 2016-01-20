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
#define CMD_NEW_GESTURE 0x01

#define MOTOR_CLOCKWISE 0x01
#define MOTOR_COUNTERCLOCK 0x02

void setup()
{
  //ble_set_name("MyoRobotArm");

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

void loop()
{
  static boolean transmit_light_sensor = false;

  // If data is ready
  while(ble_available())
  {
    // read out command and data
    byte data0 = ble_read();
    byte data1 = ble_read();
    byte data2 = ble_read();

        digitalWrite(MOTOR_A_1, LOW);
        digitalWrite(MOTOR_A_2, LOW);
    if (data0 == CMD_NEW_GESTURE)
    {
      Serial.println("Motor control OP code");
      controlMotor(data1);
    }
    else if (data0 == CMD_SET_LIGHT_TRANSMIT)
    {
      Serial.println("Light Transmit OP code");
      transmit_light_sensor = (data1 == 0x01);
    }
    else if (data0 == 0x04)
    {
      transmit_light_sensor = false;
      digitalWrite(MOTOR_A_1, LOW);
      digitalWrite(MOTOR_A_2, LOW);
    }
  }

  if (transmit_light_sensor)  // if analog reading enabled
  {
    // Read and send out
    uint16_t value = analogRead(ANALOG_IN_PIN);
    ble_write(0x0B);
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



