/*
 * MicroMark Mill Drill
 * 1 turn of the lead screw = 0.050" travel
 * 20 turns = 1" travel
 * 1 ipm travel speed = 20 RPM
 * 12 ipm = 1200 RPM
 */

#include "myStepper.h"
#include "debugprint.h"

// Pin Definitions
#define BUZZER_PIN 10
#define LED_PIN    13
#define SPEED_PIN  A9
#define FWD_PIN    A0
#define REV_PIN    A1
#define START_PIN  A2

// Constants
#define SLEEP_TIME     1000
#define SAMPLE_TIMER   500
#define DEBOUNCE_TIME  250
#define AVG_LIST_LEN   5
#define RPM_MIN        20
#define RPM_MAX        260

bool wasPressed         = false;
bool moveEnabled        = false;
int moveDir             = MC_DIR_FWD;
int speedKnob           = 0;
int oldSpeed            = 0;
uint32_t waitTime       = 0;
uint32_t debounceTimer  = 0;
uint32_t speedSampleTimer = 0;
int avgList[AVG_LIST_LEN];

void setup() {
  Serial.begin(115200);
  
  // Allow everybody to wake up...
  //delay(2000);
  while ( ! Serial )
    delay(10);

  debugprint(DEBUG_TRACE, "Mill Drive 0.1.0");

  // Set up pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(SPEED_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FWD_PIN, INPUT_PULLUP);
  pinMode(REV_PIN, INPUT_PULLUP);
  pinMode(START_PIN, INPUT_PULLUP);

  // I'm awake, I'm awake!!!
  beep(100);

  // Set up the motor...
  theMotor.begin();
  theMotor.setStepType(MC_STEP_FULL);

  // All done!
  beep(100);

  // Speed debugging cruft...
  //theMotor.moveNow();
}

void beep(int duration) {
  // Set up pins...
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  // Turn on LED and enable buzzer...
  digitalWrite(LED_PIN, HIGH);
  analogWrite(BUZZER_PIN, 127);

  // Wait for the specified time...
  delay(duration);

  // Turn off the LED and buzzer...
  digitalWrite(LED_PIN, LOW);
  analogWrite(BUZZER_PIN, 0);
}

void complain() {
  beep(50); delay(50); beep(50); delay(50); beep(50);
}

int speedAverage(int speedReading)
{
  return (speedReading / 100 * 100) + 1;
  
  // Rotate the list
  for ( int i = 0; i < AVG_LIST_LEN - 1; i++ ) {
    //debugprint(DEBUG_TRACE, "avgList[%d] = %d", i, avgList[i]);
    avgList[i + 1] = avgList[i];
  }

  // Save the new value
  avgList[0] = speedReading;
  
  // Add them up
  int total = 0;
  for ( int i = 0; i < AVG_LIST_LEN; i++ ) {
    total += avgList[i];
  }

  return total / AVG_LIST_LEN;
}

void handleControls()
{
  // Check the speed pot...
  speedKnob = speedAverage(analogRead(SPEED_PIN));

  // Check the direction switch...
  if ( ! digitalRead(FWD_PIN) ) {
    moveDir = MC_DIR_FWD;
  }
  else if ( ! digitalRead(REV_PIN) ) {
    moveDir = MC_DIR_REV;
  }
  
  // Check the start button...
  if ( digitalRead(START_PIN) == LOW ) {

    if ( ! wasPressed ) {
      beep(50);
  
      if ( moveEnabled ) {
        moveEnabled = false;
      }
      else {
        moveEnabled = true;
      }
  
      wasPressed = true;
      debounceTimer = millis() + DEBOUNCE_TIME;
    }
  }
  else {
    if ( millis() > debounceTimer ) {
      wasPressed = false;
    }
  }
}

void handleMotor ()
{
  if ( moveEnabled ) {

    // Set the motor speed
    if ( oldSpeed != speedKnob ) {
      theMotor.setSpeed(map(speedKnob, 1, 1023, RPM_MIN, RPM_MAX));
      oldSpeed = speedKnob;
    }
  
    if ( ! theMotor.getCanMove() ) {

      //theMotor.setSpeed(map(speedKnob, 1, 1023, RPM_MIN, RPM_MAX));

      // Set the motor direction
      switch ( moveDir ) {
        case MC_DIR_FWD:
          theMotor.setReverse(false);
          break;
        case MC_DIR_REV:
          theMotor.setReverse(true);
          break;
        default:
          debugprint(DEBUG_ERROR, "Invalid direction: %d", moveDir);
          break;
      }
  
      // Start the motor moving
      theMotor.moveNow();
    }
  }
  else {
    if ( theMotor.getIsMoving() )
      theMotor.stopNow();
  }
}

void debugOutput()
{
  if ( millis() > waitTime ) {

    debugprint(DEBUG_TRACE, "\n\n*** Current Status %ld ***", millis());
    debugprint(DEBUG_TRACE, "Speed = %d", speedKnob);
    debugprint(DEBUG_TRACE, "Move is %s", moveEnabled?"enbaled":"disabled");
  
    switch ( moveDir ) {
      case MC_DIR_FWD:
        debugprint(DEBUG_TRACE, "Forward");
        break;
      case MC_DIR_REV:
        debugprint(DEBUG_TRACE, "Reverse");
        break;
      default:
        debugprint(DEBUG_ERROR, "Invalid direction: %d", moveDir);
        break;
    }

    theMotor.dumpDebug();

    waitTime = millis() + SLEEP_TIME;
  }
}

uint32_t theSpeed = RPM_MIN;

void loop() {
/* Speed debugging cruft...
  theMotor.setSpeed(theSpeed++);
  if ( theSpeed > RPM_MAX ) {
    theSpeed = RPM_MIN;
  }

  delay(250);
*/  

  handleControls();

  debugOutput();

  handleMotor();

}

