/*************************************************************************
  Testing sketch for Freematics OBD-II I2C Adapter
  Reads and prints several OBD-II PIDs value and MEMS sensor data
  Distributed under GPL v2.0
  Visit http://freematics.com for more information
  Written by Stanley Huang <support@freematics.com.au>
*************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <OBD.h>

COBDI2C obd;
bool hasMEMS;

void testOut() {
  static const char cmds[][6] = {"ATZ\r", "ATH0\r", "ATRV\r", "0100\r", "010C\r", "0902\r"};
  char buf[128];

  for (byte i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
    const char *cmd = cmds[i];
    Serial.print("Sending ");
    Serial.println(cmd);
    if (obd.sendCommand(cmd, buf, sizeof(buf))) {
      char *p = strstr(buf, cmd);
      if (p)
        p += strlen(cmd);
      else
        p = buf;
      while (*p == '\r') p++;
      while (*p) {
        Serial.write(*p);
        if (*p == '\r' && *(p + 1) != '\r')
          Serial.write('\n');
        p++;
      }
    } else {
      Serial.println("Timeout");
    }
    delay(1000);
  }
  Serial.println();
}

void readMEMS() {
  int acc[3];
  int gyro[3];
  int temp;

  if (!obd.memsRead(acc, gyro, 0, &temp)) return;

  Serial.print('[');
  Serial.print(millis());
  Serial.print(']');

  Serial.print("ACC:");
  Serial.print(acc[0]);
  Serial.print('/');
  Serial.print(acc[1]);
  Serial.print('/');
  Serial.print(acc[2]);

  Serial.print(" GYRO:");
  Serial.print(gyro[0]);
  Serial.print('/');
  Serial.print(gyro[1]);
  Serial.print('/');
  Serial.print(gyro[2]);
  boolMotion(acc[2]);

  Serial.print(" TEMP:");
  Serial.print((float)temp / 10, 1);
  Serial.println("C");
}

void readPIDs() {
  static const byte pidlist[] = {PID_ENGINE_LOAD, PID_COOLANT_TEMP, PID_RPM, PID_SPEED, PID_TIMING_ADVANCE, PID_INTAKE_TEMP, PID_THROTTLE, PID_FUEL_LEVEL};
  Serial.print('[');
  Serial.print(millis());
  Serial.print(']');
  for (byte i = 0; i < sizeof(pidlist) / sizeof(pidlist[0]); i++) {
    byte pid = pidlist[i];
    bool valid = obd.isValidPID(pid);
    Serial.print((int)pid | 0x100, HEX);
    Serial.print('=');
    if (valid) {
      int value;
      if (obd.readPID(pid, value)) {
        Serial.print(value);
        boolRPM(pid, value);
      }
    }
    Serial.print(' ');
  }
  Serial.println();
}

void readBatteryVoltage() {
  Serial.print('[');
  Serial.print(millis());
  Serial.print(']');
  Serial.print("Battery:");
  Serial.print(obd.getVoltage(), 1);
  Serial.println('V');
}

bool boolMotion(int gyro) {
  if (gyro < 15000, 20000 < gyro) {
    digitalWrite(8, HIGH);
    return true;
  } else {
    digitalWrite(8, LOW);
    return false;
  }
}

bool boolRPM(byte pid, int rpm) {
  if(pid == PID_RPM)
  if (rpm > 2000) {
    digitalWrite(13, HIGH);
    return true;
  } else {
    digitalWrite(13, LOW);
    return false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);
  obd.begin();
  pinMode(13, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  hasMEMS = obd.memsInit();
  Serial.print("MEMS:");
  Serial.println(hasMEMS ? "Yes" : "No");

  // send some commands for testing and show response for debugging purpose
  testOut();

  // initialize OBD-II adapter
  do {
    digitalWrite(13, HIGH);
    Serial.println("Init...");
  } while (!obd.init(PROTO_ISO_9141_2));
  
  // send some commands for testing and show response for debugging purpose
  testOut();
  
  digitalWrite(13, LOW);
  char buf[64];
  if (obd.getVIN(buf, sizeof(buf))) {
    Serial.print("VIN:");
    Serial.println(buf);
  }

  unsigned int codes[6];
  byte dtcCount = obd.readDTC(codes, 6);
  if (dtcCount == 0) {
    Serial.println("No DTC");
  } else {
    Serial.print(dtcCount);
    Serial.print(" DTC:");
    for (byte n = 0; n < dtcCount; n++) {
      Serial.print(' ');
      Serial.print(codes[n], HEX);
    }
    Serial.println();
  }
}

void loop() {
  readPIDs();
  readBatteryVoltage();
  if (hasMEMS) {
    readMEMS();
  }
}

