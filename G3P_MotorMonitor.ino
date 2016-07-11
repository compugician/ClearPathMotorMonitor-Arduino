#include "G3P_MotorMonitor.h"

const int HLFB_THRESH = 3; //the level at which motor is considered errored (to 'debounce'), by side-effect controls the number of 'change' updates occuring. 

const int PIN_HLFB_X = 2;
const int PIN_HLFB_XP = 3;
const int PIN_HLFB_Y = 4;
const int PIN_HLFB_Z = 5;
const int PIN_HLFB_A = 6;

const int PIN_EN_X = 8;
const int PIN_EN_XP = 8;
const int PIN_EN_Y = 9;
const int PIN_EN_Z = 10;
const int PIN_EN_A = 11;

const int MOT_ENABLED = 1;
const int MOT_DISABLED = 0;
const int MOT_ERROR = -1;

const int MIN_UPDATE_INTERVAL = 2500; //if this time (milliseconds) or more passed since the last update, send another
const int SAMPLE_INTERVAL = 100; //sample the motor status every N milliseconds. (recommended 50-200)

unsigned long lastUpdateTime;

Motors motors;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_HLFB_X, INPUT_PULLUP);
  pinMode(PIN_HLFB_XP, INPUT_PULLUP);
  pinMode(PIN_HLFB_Y, INPUT_PULLUP);
  pinMode(PIN_HLFB_Z, INPUT_PULLUP);
  pinMode(PIN_HLFB_A, INPUT_PULLUP);
  pinMode(PIN_EN_X, INPUT);
  pinMode(PIN_EN_XP, INPUT);
  pinMode(PIN_EN_Y, INPUT);
  pinMode(PIN_EN_Z, INPUT);
  pinMode(PIN_EN_A, INPUT);

  initializeMotorsStruct();

  lastUpdateTime = millis();
  Serial.begin(115200);
  Serial.println("MotorMon Start");
  Serial.println("[-----]");

}

void initializeMotorsStruct() {
  motors.x.en = true;
  motors.xp.en = true;
  motors.y.en = true;
  motors.z.en = true;
  motors.a.en = true;

  motors.x.hlfb = 0;
  motors.xp.hlfb = 0;
  motors.y.hlfb = 0;
  motors.z.hlfb = 0;
  motors.a.hlfb = 0;
}

/**
 * take care of HLFB value, return true iff value changed;
 */
bool _updateHLFB(MotorStatus* ms, bool pinState) {
  
  //if HLFB is HIGH (meaning pull-up is able to pull, meaning motor 'broke' HLFB path)
  //then increment HLFB upto HLFB_MAX; otherwise make sure HLFB is at 0.  
  if (HIGH==pinState) {
    if (ms->hlfb < HLFB_THRESH) {
      ms->hlfb++;
      return true;
    } else {
      return false;
    }
  } else {
    if (0==ms->hlfb) {
      return false;
    } else {
      ms->hlfb = 0;
      return true;
    }
  }
  
}

/**
 * take care of EN value, return true iff value changed;
 */
bool _updateEnStatus(MotorStatus* ms, bool pinState) {
  if (ms->en==pinState) {
    return false;
  } else {
    ms->en=pinState;
    return true;
  }
}

/**
 * Update motor struct. Return true iff something changed.
 */
bool updateMotorStatus() {
  bool changeOccured = false;

  changeOccured |= _updateEnStatus(&motors.x, digitalRead(PIN_EN_X));
  changeOccured |= _updateEnStatus(&motors.xp, digitalRead(PIN_EN_XP));
  changeOccured |= _updateEnStatus(&motors.y, digitalRead(PIN_EN_Y));
  changeOccured |= _updateEnStatus(&motors.z, digitalRead(PIN_EN_Z));
  changeOccured |= _updateEnStatus(&motors.a, digitalRead(PIN_EN_A));

  changeOccured |= _updateHLFB(&motors.x, digitalRead(PIN_HLFB_X));
  changeOccured |= _updateHLFB(&motors.xp, digitalRead(PIN_HLFB_XP));
  changeOccured |= _updateHLFB(&motors.y, digitalRead(PIN_HLFB_Y));
  changeOccured |= _updateHLFB(&motors.z, digitalRead(PIN_HLFB_Z));
  changeOccured |= _updateHLFB(&motors.a, digitalRead(PIN_HLFB_A));

  return changeOccured;
}

int getMotorState(MotorStatus ms) {
  return (LOW==ms.en) ? MOT_DISABLED : ( (HLFB_THRESH <= ms.hlfb) ? MOT_ERROR : MOT_ENABLED );
}


String motorStatusStr(MotorStatus ms) {
 String enStr = (1==ms.en) ? "E":"D";
 return "[ "+enStr+"|"+ms.hlfb+"|"+motorStatusShortStr(ms)+"]";

}

String motorStatusShortStr(MotorStatus ms) {
  switch (getMotorState(ms)) {
    case MOT_ENABLED:
        return "1";
      break; 
    case MOT_DISABLED:
        return "0";
      break; 
    case MOT_ERROR:
        return "*";
      break; 
  }
}

String motorsStatusStr() {
  return "X: "+motorStatusStr(motors.x)
  + " XP: "+motorStatusStr(motors.xp)
  + " Y: "+motorStatusStr(motors.y)
  + " Z: "+motorStatusStr(motors.z)
  + " A: "+motorStatusStr(motors.a)+"\n";
}

String motorStatusShortStr() {
  return "["+motorStatusShortStr(motors.x)
    + motorStatusShortStr(motors.xp)  
    + motorStatusShortStr(motors.y)  
    + motorStatusShortStr(motors.z)  
    + motorStatusShortStr(motors.a)+"]\n";  
}

void sendUpdate() {
  lastUpdateTime = millis();

 // String str = motorsStatusStr();
  String str = motorStatusShortStr();
  
  Serial.print(str);
//  Serial.println(millis());
}

void loop() {
  if (updateMotorStatus() || ((millis()-lastUpdateTime) >= MIN_UPDATE_INTERVAL)) {
    sendUpdate();
  }
  delay(SAMPLE_INTERVAL); 
}
