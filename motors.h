#ifndef motors_h
#define motors_h

#include <AccelStepper.h>

#define maxmoveamount 100
#define azfullcirclesteps -30975
#define altfullcirclesteps 30970

#define maxspeed 500
#define acceleration 300

class stepperMotors 
{
  public:

  void init()
  {
    // position zero is alt = 90, az = 0 (so tube vertical, az base set to north
    // - this corresponds to storage position, so easier set up)
    // user starts the scope like this (using compasses, spirit levels etc), 
    // then does a goto to a known object. Once there, turn on calibrate mode,
    // manual move to center the object, and turn off calibrate mode (calibrate
    // mode simply doesn't update the stored current position during a manual move)

    // ESP-wroom-32 pin out - using 10 pins on LHS going from 3.3V
    // first four used for V & GND, then two sets of Enable, Step, Dir
    //  Az step = 25, dir = 27

  // 34, 35, 36, & 39 are all input only!!!

    // Alt step = 42, dir = 40 
    // enable pins wired to GND (enabled)
    AzStepper = new AccelStepper(AccelStepper::DRIVER, 27, 26 ); // step, dir 
    AzStepper->setMaxSpeed(maxspeed);
    AzStepper->setAcceleration(acceleration);
    AzStepper->setCurrentPosition(0);
    //AzStepper->setMinPulseWidth(5);

    AltStepper = new AccelStepper(AccelStepper::DRIVER, 12, 14);    // step, dir 
    AltStepper->setMaxSpeed(maxspeed);
    AltStepper->setAcceleration(acceleration);
    AltStepper->setCurrentPosition(getStepsFordegrees(true, 90));
    //AltStepper->setMinPulseWidth(5);

    moveamount = 10;
  }

  void update()
  {
    AzStepper->run();
    AltStepper->run();
  }

  int getStepsFordegrees(bool alt, double angle)
  {
    return (int)(angle * ((alt)? altfullcirclesteps : azfullcirclesteps) / 360.0 );
  }

  double getDegreesForSteps(bool alt, long steps)
  {
    return steps / (((alt)? altfullcirclesteps : azfullcirclesteps) / 360.0 );
  }

  void Move(int altsteps, int azsteps, bool continuouspress, bool calibrate)
  {
    moveamount = (continuouspress) ? moveamount * 2 : 1;
    if (moveamount > maxmoveamount) moveamount = maxmoveamount;
    if (calibrate)
    {
      // to move while leaving the currentposition unchanged (i.e. fine tune
      // what the scope thinks it's pointing at), we set the current position
      // backwards, and then let it move back to what it thinks was the original
      // viewpoint.
      AltStepper->setCurrentPosition(AltStepper->currentPosition()-(altsteps*moveamount));
      AzStepper->setCurrentPosition(AzStepper->currentPosition()-(azsteps*moveamount));
    }
    AltStepper->moveTo(AltStepper->currentPosition() + altsteps*moveamount);
    AzStepper->moveTo(AzStepper->currentPosition() + azsteps*moveamount);
    keepMovingUntilDone();
  }

  void setTarget(double alt, double az)
  {
    AltStepper->moveTo(getStepsFordegrees(true, alt));
    AzStepper->moveTo(getStepsFordegrees(false, az));
    keepMovingUntilDone();
  }

  void keepMovingUntilDone()
  {
    do {
        update();
    } while (!completedSlew());
  }

  bool completedSlew()
  {
    return (AltStepper->currentPosition() == AltStepper->targetPosition() &&
            AzStepper->currentPosition() == AzStepper->targetPosition());
  }

  void getCurrentPostion(double& alt, double& az)
  {
    alt = getDegreesForSteps(AltStepper->currentPosition(), true);
    az = getDegreesForSteps(AzStepper->currentPosition(), false);
  }

private:
  AccelStepper* AzStepper; 
  AccelStepper* AltStepper;
  int moveamount;
};

#endif // motors_h