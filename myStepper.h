
#ifndef _myStepper_h_
#define _myStepper_h_

#include <Arduino.h>

#define STEP_PULSE(steps, microsteps, rpm) (60*1000000L/steps/microsteps/rpm)

#define DEBUG_COILS 0x0010

// Motor Controller Pins
#define MC_PIN_DIR               2
#define MC_PIN_STEP              3
#define MC_PIN_SLEEP             4
#define MC_PIN_RESET             5
#define MC_PIN_MS3               6
#define MC_PIN_MS2               7
#define MC_PIN_MS1               8
#define MC_PIN_ENABLE            9

// Motor Step Types
#define NUM_STEPTYPES            5 // min step interval
#define MC_STEP_FULL             0 // 1600
#define MC_STEP_HALF             1 // 800
#define MC_STEP_QUARTER          2 // 400
#define MC_STEP_EIGHTH           3 // 200
#define MC_STEP_SIXTEENTH        4 // 100

// Step Interval constraints
#define MC_MIN_STEP_INTERVAL        2 // 2 uS ( 1200 rpm )
#define MC_MAX_STEP_INTERVAL   300000 // 300 mS ( 1 rpm )

// Motor Directions
#define NUM_DIRS                 2
#define MC_DIR_REV               0
#define MC_DIR_FWD               1

// Text debugging aids
extern int stepTypes[NUM_STEPTYPES];
extern const char * stepTypeNames[NUM_STEPTYPES];
extern int stepTypeNickNames[NUM_STEPTYPES];
extern const char * dirNames[NUM_DIRS];

//
// myStepper Class
//

class myStepper
{
  public:
  
    void begin();
    void setSpeed(int);
    void moveNow();
    void stopNow();
    void dumpDebug();
    
    // Accessors
    int getStepType();      void setStepType(int);
    int getStepInterval();  void setStepInterval(int);
    int getRevSteps();      void setRevSteps(int);
    boolean getIsMoving();  void setIsMoving(boolean);
    boolean getCanMove();   void setCanMove(boolean);
    boolean getCoilPwr();   void setCoilPwr(boolean);
    boolean getReverse();   void setReverse(boolean);

  private:
  
    int m_stepType = MC_STEP_FULL;
    int m_stepInterval = MC_MAX_STEP_INTERVAL;
    int m_revSteps = 200;
    boolean m_isMoving = false;
    boolean m_canMove = false;
    boolean m_coilHold = false;
    boolean m_reverse = false;
    IntervalTimer myTimer;
    
    void coilsOn();
    void coilsOff();
    void stepOnce();
    static void doInterrupt();
};

// Global motor object
extern myStepper theMotor;

#endif

