
void handleUartCommunication() {
  if (Serial.available() > 0) {
    String NewCommand = Serial.readStringUntil('\n');

    if (NewCommand == "?") {
      Serial.print("Current Periode = ");
      Serial.print(rotorSpeedRpm);
      Serial.println("s");
    }else{
      // NewCommand contains no command but new speed-values
      rotorSpeedRpmDesired = NewCommand.toInt();

      if (rotorSpeedRpmDesired != 0) {
        if (!sequence->running){
          timerStartup = 200;
          motorResume(sequence);
          // Startup-Speed
          motorSetSpeed(sequence, 42);
        }else{
          // desired speed
          motorSetSpeed(sequence, rotorSpeedRpmDesired);
        }
        Serial.print("Set new speed to ");
        Serial.print(rotorSpeedRpmDesired);
        Serial.println(" U/min");
      }else{
        motorRelease(sequence);
        Serial.println("Stopped motor.");
      }
    }
    // flush the buffer
    for (int i=0; i<Serial.available(); i++) {
      Serial.read();
    }
  }
}