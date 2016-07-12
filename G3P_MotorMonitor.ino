#include "G3P_MotorMonitor.h"

const int HLFB_THRESH = 3; //the level at which motor is considered errored (to 'debounce'), by side-effect controls the number of 'change' updates occuring. 

//HLFB and EN pins, in order [X, XP, Y, Z, A, S] which is the order chilipeppr expects it.
//NOTICE: X and XP share an EN pin, but not a HLFB pin
const int HLFB_PINS[] = {2,3,4,5,6,7};
const int EN_PINS[] = {8,8,9,10,11,12};
const int MOTOR_COUNT = sizeof(HLFB_PINS)/sizeof(int);

//'enum' for motor status
const int MOT_ENABLED = 1;
const int MOT_DISABLED = 0;
const int MOT_ERROR = -1;

const int MIN_UPDATE_INTERVAL = 2500; //if this time (milliseconds) or more passed since the last update, send another
const int SAMPLING_INTERVAL = 100; //sample the motor status every N milliseconds. (recommended 50-200)

unsigned long lastUpdateTime;

MotorStatus motors[MOTOR_COUNT];

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i<MOTOR_COUNT; i++) {
    pinMode(HLFB_PINS[i], INPUT_PULLUP);
    pinMode(EN_PINS[i],INPUT);
  }

  initializeMotorsStruct();

  lastUpdateTime = millis();
  Serial.begin(115200);
  Serial.println("MotorMon Start");
  Serial.println("[------]");

}

void initializeMotorsStruct() {
  for (int i = 0; i<MOTOR_COUNT; i++) {
    motors[i].en = true;
    motors[i].hlfb = 0;
  }
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

bool updateMotorStatus(MotorStatus* ms, bool enPinState, bool hlfbPinState) {
  bool changed = _updateEnStatus(ms, enPinState);
  changed |= _updateHLFB(ms, hlfbPinState);

  return changed;
}

/**
 * Update motor struct. Return true iff something changed.
 */
bool updateAllMotorsStatus() {
  bool changeOccured = false;

  for (int i = 0; i<MOTOR_COUNT; i++) {
    changeOccured |= updateMotorStatus(&motors[i], digitalRead(EN_PINS[i]), digitalRead(HLFB_PINS[i]));
  }

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

//for debugging
String motorsStatusStr() {
  return "X: "+motorStatusStr(motors[0])
  + " XP: "+motorStatusStr(motors[1])
  + " Y: "+motorStatusStr(motors[2])
  + " Z: "+motorStatusStr(motors[3])
  + " A: "+motorStatusStr(motors[4])
  + " S: "+motorStatusStr(motors[5])+"\n";
}

//for 'production' to communicate w/ Chilipeppr
String motorStatusShortStr() {
  String result = "[";
  for (int i=0; i<MOTOR_COUNT; i++) {
    result+=motorStatusShortStr(motors[i]);
  }
  return result+"]\n";  
}

void sendUpdate() {
  lastUpdateTime = millis();

 // String str = motorsStatusStr(); //debug
  String str = motorStatusShortStr(); //production
  
  Serial.print(str);
//  Serial.println(millis()); //for debug
}

void loop() {
  if (updateAllMotorsStatus() || ((millis()-lastUpdateTime) >= MIN_UPDATE_INTERVAL)) {
    sendUpdate();
  }
  
  delay(SAMPLING_INTERVAL); 
}

