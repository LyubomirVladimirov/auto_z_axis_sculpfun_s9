/*!
 * @File  DFRobot_IraserSensor.ino
 * @brief  In this example, the infrared laser ranging sensor is used to measure the distance, and the sensor data is processed to obtain the measured distance
 * @copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 * @licence  The MIT License (MIT)
 * @author  [liunian](nian.liu@dfrobot.com)
 * @version  V1.0
 * @date  2020-08-13
 * 
 * 
 * https://wiki.dfrobot.com/Infrared_Laser_Distance_Sensor_50m_80m_SKU_SEN0366
 * 
 *  3/10 is TX, 2/9 is RX  (on cnc shield 9 is X-endstop, 10 is Y-end stop)
 */
#include <SoftwareSerial.h>
#include <AccelStepper.h>

SoftwareSerial mySerial(9, 10);  // Define software serial, 3/10 is TX, 2/9 is RX  (on CNC shield 9 is X-endstop, 10 is Y-end stop)
AccelStepper stepper(1, 2, 5);  // (1: Driver mode, Step Pin, Direction Pin)

char cont_measure[4] = {0x80, 0x06, 0x03, 0x77};  // cont measure
char single_measure[4] = {0xFA, 0x06, 0x06, 0xFA};  // single measure
char read_cache[4] = {0x80, 0x06, 0x07, 0x73};  // read cache
//char laser_on[5] = {0x06, 0x05, 0x01, 0x  };  // turn on laser
char set_range_to_5m[5] = {0xFA, 0x04, 0x09, 0x05, 0xF4}; // Set measuring range: FA 04 09 05 F4 5m
unsigned char data[11] = {0};

float desiredDistance = -1.0;  // init distance in meters
const float tolerance = 0.001;  // Tolerance in meters
const int stepsPerRevolution = 10;  // Change this according to your stepper motor's specification

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  stepper.setMaxSpeed(2000);  // Set the maximum speed of the stepper motor (steps per second)
  stepper.setAcceleration(20000);  // Set the acceleration of the stepper motor (steps per second squared)

  // set laser sensor
  //mySerial.print(set_range_to_5m);
  Serial.print("exiting setup");
}

void loop() {

  mySerial.print(cont_measure);         /////  ###########$$$$$$  --TODO--  $$$$############## now that the stepper is blocking till pos is reached, poll the sensor only when ready to read
  while (1) {
    
    //mySerial.print(single_measure);
    //delay(20);   /////        ########### see min value that is ok

    //mySerial.print(read_cache);

    
    if (mySerial.available() > 0) {
      delay(5);
      for (int i = 0; i < 11; i++) {
        data[i] = mySerial.read();
      }
      unsigned char Check = 0;
      for (int i = 0; i < 10; i++) {
        Check = Check + data[i];
      }
      Check = ~Check + 1;
      if (data[10] == Check) {
        if (data[3] == 'E' && data[4] == 'R' && data[5] == 'R') {
          Serial.println("Out of range");
        } else {
          float distance = 0;
          distance = (data[3] - 0x30) * 100 + (data[4] - 0x30) * 10 + (data[5] - 0x30) * 1 + (data[7] - 0x30) * 0.1 + (data[8] - 0x30) * 0.01 + (data[9] - 0x30) * 0.001;
          Serial.print("Distance = ");
          Serial.print(distance, 3);
          Serial.println(" M");

       if (desiredDistance < 0) {
          desiredDistance = distance;  // Set initial distance as the target
          Serial.print("Target Distance Set: ");
          Serial.println(desiredDistance, 3);
        }
        
          // Check if the distance is within the tolerance range
          if (distance < desiredDistance - tolerance) {
            stepper.move(-stepsPerRevolution);  // Rotate the stepper motor to decrease the distance

            while(stepper.distanceToGo() != 0) {  
              stepper.runSpeedToPosition();
            }
    
          } else if (distance > desiredDistance + tolerance) {
            stepper.move(stepsPerRevolution);  // Rotate the stepper motor to increase the distance

            while(stepper.distanceToGo() != 0) {  
              stepper.runSpeedToPosition();
            }
            
          }
        }
      } else {
        Serial.println("Invalid Data!");
      }
    }
    delay(5);
  }
}
