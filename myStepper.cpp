#include "myStepper.h"
#include "debugprint.h"

int stepTypes[NUM_STEPTYPES] = {
  MC_STEP_FULL,
  MC_STEP_HALF,
  MC_STEP_QUARTER,
  MC_STEP_EIGHTH,
  MC_STEP_SIXTEENTH,
};

const char * stepTypeNames[NUM_STEPTYPES] = {
  "FULL",
  "HALF",
  "QUARTER",
  "EIGHTH",
  "SIXTEENTH",
};

int stepTypeNickNames[NUM_STEPTYPES] = { 1, 2, 4, 8, 16 };

const char * dirNames[NUM_DIRS] = {
  "MC_DIR_REV",
  "MC_DIR_FWD",
};

myStepper theMotor;

// Set up the motor driver
void myStepper::begin() {

  // Set the pin modes, and set them all to default values...
  pinMode(MC_PIN_DIR, OUTPUT);    digitalWrite(MC_PIN_DIR, MC_DIR_FWD);
  pinMode(MC_PIN_STEP, OUTPUT);   digitalWrite(MC_PIN_STEP, LOW);
  pinMode(MC_PIN_MS1, OUTPUT);    digitalWrite(MC_PIN_MS1, LOW);
  pinMode(MC_PIN_MS2, OUTPUT);    digitalWrite(MC_PIN_MS2, LOW);
  pinMode(MC_PIN_MS3, OUTPUT);    digitalWrite(MC_PIN_MS3, LOW);
  pinMode(MC_PIN_SLEEP, OUTPUT);  digitalWrite(MC_PIN_SLEEP, HIGH);
  pinMode(MC_PIN_RESET, OUTPUT);  digitalWrite(MC_PIN_RESET, HIGH);
  pinMode(MC_PIN_ENABLE, OUTPUT); digitalWrite(MC_PIN_ENABLE, HIGH);
  
  // Set up the interrupt...
  myTimer.begin(doInterrupt, m_stepInterval);
}

void myStepper::dumpDebug() {
  debugprint(DEBUG_TRACE, "\nmyStepper status:");
  debugprint(DEBUG_TRACE, "stepType = %s", stepTypeNames[m_stepType]);
  debugprint(DEBUG_TRACE, "stepInterval = %d", m_stepInterval);
  debugprint(DEBUG_TRACE, "Speed: %d RPM", 60000000 / m_revSteps / m_stepInterval );
  debugprint(DEBUG_TRACE, "canMove = %s", m_canMove?"TRUE":"FALSE");
  debugprint(DEBUG_TRACE, "isMoving = %s", m_isMoving?"TRUE":"FALSE");
  debugprint(DEBUG_TRACE, "coilPwr = %s", m_coilHold?"TRUE":"FALSE");
  debugprint(DEBUG_TRACE, "reverse = %s", m_reverse?"TRUE":"FALSE");
}

// Accessors

int myStepper::getStepType() {
  return m_stepType;
}

void myStepper::setStepType(int newType) {

  debugprint(DEBUG_TRACE, "Setting stepType to %s", stepTypeNames[newType]);

  m_stepType = newType >= 0 && newType < NUM_STEPTYPES ? newType : m_stepType;
  
  switch ( newType ) {
    case MC_STEP_FULL:
      digitalWrite(MC_PIN_MS1, LOW);
      digitalWrite(MC_PIN_MS2, LOW);
      digitalWrite(MC_PIN_MS3, LOW);
      break;
    case MC_STEP_HALF:
      digitalWrite(MC_PIN_MS1, HIGH);
      digitalWrite(MC_PIN_MS2, LOW);
      digitalWrite(MC_PIN_MS3, LOW);
      break;
    case MC_STEP_QUARTER:
      digitalWrite(MC_PIN_MS1, LOW);
      digitalWrite(MC_PIN_MS2, HIGH);
      digitalWrite(MC_PIN_MS3, LOW);
      break;
    case MC_STEP_EIGHTH:
      digitalWrite(MC_PIN_MS1, HIGH);
      digitalWrite(MC_PIN_MS2, HIGH);
      digitalWrite(MC_PIN_MS3, LOW);
      break;
    case MC_STEP_SIXTEENTH:
      digitalWrite(MC_PIN_MS1, HIGH);
      digitalWrite(MC_PIN_MS2, HIGH);
      digitalWrite(MC_PIN_MS3, HIGH);
      break;
    default:
      debugprint(DEBUG_ERROR, "*** EROR *** Invalid stepType requested: %d", newType);
      break;
  }
}

int myStepper::getStepInterval() {
  return m_stepInterval;
}

void myStepper::setStepInterval(int newInterval) {

  debugprint(DEBUG_TRACE, "Setting stepInterval to %d", newInterval);

  if ( newInterval >= MC_MIN_STEP_INTERVAL && newInterval <= MC_MAX_STEP_INTERVAL ) {
    m_stepInterval = newInterval;
  }
  else {
    debugprint(DEBUG_ERROR, "Invalid stepInterval: %d", newInterval);
  }
}

int myStepper::getRevSteps() {
  return m_revSteps;
}

void myStepper::setRevSteps(int newSteps) {

  debugprint(DEBUG_TRACE, "Setting revSteps to %d", newSteps);
  
  m_revSteps = newSteps >= MC_MIN_STEP_INTERVAL && newSteps <= MC_MAX_STEP_INTERVAL ? newSteps : m_revSteps;
}

boolean myStepper::getIsMoving() {
  return m_isMoving;
}

void myStepper::setIsMoving(boolean flag) {
  m_isMoving = flag;
}

boolean myStepper::getCanMove() {
  return m_canMove;
}

void myStepper::setCanMove(boolean flag) {
  m_canMove = flag;
}

boolean myStepper::getCoilPwr() {
  return m_coilHold;
}

void myStepper::setCoilPwr(boolean flag) {
  m_coilHold = flag;
}

boolean myStepper::getReverse() {
  return m_reverse;
}

void myStepper::setReverse(boolean flag ) {
  m_reverse = flag;
}

//
// myStepper Implementation
//

// Turn on the coil power...
void myStepper::coilsOn() {
  digitalWrite(MC_PIN_ENABLE, LOW);
  debugprint(DEBUG_TRACE, "coilsOn()");
}

// Turn off the coil power...
void myStepper::coilsOff() {
  digitalWrite(MC_PIN_ENABLE, HIGH);
  debugprint(DEBUG_TRACE, "coilsOff()");
}
/*
void myStepper::runUntil(bool * flag) {
  m_isMoving = true;
  
  while ( *flag ) {
    digitalWrite(MC_PIN_DIR, m_reverse);

    // Step the motor...
    digitalWrite(MC_PIN_STEP, HIGH); // assert STEP
    delayMicroseconds(2);            // wait two microseconds
    digitalWrite(MC_PIN_STEP, LOW);  // clear STEP
  }
}
*/
void myStepper::stepOnce() {
  //Serial.print(".");

  // Are we allowed to move?
  if ( ! m_canMove )
    return;

  m_isMoving = true;

  // Set the direction...
  digitalWrite(MC_PIN_DIR, m_reverse);

  // Step the motor...
  digitalWrite(MC_PIN_STEP, HIGH); // assert STEP
  delayMicroseconds(4);            // wait two microseconds
  digitalWrite(MC_PIN_STEP, LOW);  // clear STEP
}

// Handle the timer interrupt...
void myStepper::doInterrupt() {
  // Hold that thought...
  noInterrupts();

  // Step the motor...
  theMotor.stepOnce();

  // You were saying...?
  interrupts();
}

void myStepper::moveNow() {
  
  debugprint(DEBUG_TRACE, "Moving...");
  
  m_canMove = true;
  coilsOn();
}

// Stop the motor immediately...
void myStepper::stopNow() {
  
  debugprint(DEBUG_TRACE, "Stopping...");

  m_isMoving = false;
  m_canMove = false;
  if ( ! m_coilHold )
    coilsOff();
}

// Set the motor speed...
void myStepper::setSpeed( int newRPM ) {
  // TODO: based on stepType.

  debugprint(DEBUG_TRACE, "Setting speed to %d", newRPM);

  // Calculate the new step interval...
  setStepInterval(60000000 / (newRPM * m_revSteps));
  
  // Start another with the new stepInterval...
  myTimer.begin(doInterrupt, m_stepInterval);  
}

