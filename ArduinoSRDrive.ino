/*
  Arduino Switched Reluctance Motor Control for Arduino Leonardo
  v1.0 built on 13.03.2025

  Parts of this work are leaned on the work of Mladen Mijatov:
  https://github.com/MeanEYE/Arduino-Switched-Reluctance-Motor-Control

  This code is supposed to run on an Arduino Leonardo to drive a Vorwerk Thermomix TM31 Motor
  This is a 4-phase 8/6-pole Switched Reluctance Motor with a DC-link-voltage of 326V
*/

#include "ArduinoSRDrive.h"

void timerSecondFcn() {
  // this function is called every second

    Serial.print("RPM      = ");
    if (periode == 0) {
      rotorSpeedRpm = 0;
    }else{
      rotorSpeedRpm = (60/(periode/1000000.0f)) / polePairs;
    }
    Serial.print(rotorSpeedRpm);
    Serial.println(" U/min");
    Serial.print("Position = ");
    Serial.println(rotorState);
    Serial.print("Reverse  = ");
    Serial.println(rotorInReverse);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Switched-Reluctance-Motor Control v1.0");

  // configure motor-hall-sensor-inputs and connect interrupts for both edges
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), isr_pin2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(3), isr_pin3, CHANGE);

  // initialize timer
  Timer1.initialize(1000); // 1ms
  Timer1.attachInterrupt(isr_timer1); // isr_timer1 to run every ms

  // intialize motor-controller
  sequence = motorInit(8, 9, 10, 11, 12, 13, SEQUENCE_SIMPLE);
  //sequence = motorInit(8, 9, 10, 11, 12, 13, SEQUENCE_OVERLAP);
  //motorSetSpeedControl(sequence, true);
  //motorSetSpeed(sequence, 84); // 84 rpm
  motorRelease(sequence); // stop motor
}

void loop() {
  if (timerSerial >= 1000) {
    timerSerial = 0;
    timerSecondFcn();
  }

  handleUartCommunication();

  motorStep(sequence, timerStartup>0);
}


void isr_timer1() {
  timerSerial+=1;
  if (timerStartup>0) {
    timerStartup-=1;
  }
}

void isr_pin2() {
  // Calculate position
  if (digitalRead(2)) {
    // rising edge pin 2
    rotorState = 4;
    rotorInReverse = !digitalRead(3);
  }else{
    // falling edge pin 2
    rotorState = 1;
    rotorInReverse = digitalRead(3);
  }

  // calculate speed
  periode = (micros() - timeBetweenISR) * 2;
  timeBetweenISR = micros();
}

void isr_pin3() {
  // Calculate Position
  if (digitalRead(3)) {
    // rising edge pin 3
    rotorState = 2;
    rotorInReverse = digitalRead(2);
  }else{
    // falling edge pin 3
    rotorState = 8;
    rotorInReverse = !digitalRead(2);
  }
}