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
    AzStepper = new AccelStepper(AccelStepper::DRIVER, 14,15); // step A0, dir A1 
    AzStepper->setMaxSpeed(maxspeed);
    AzStepper->setAcceleration(acceleration);
    AzStepper->setCurrentPosition(0);

    AltStepper = new AccelStepper(AccelStepper::DRIVER, 16,17);    // step A2, dir A3
    AltStepper->setMaxSpeed(maxspeed);
    AltStepper->setAcceleration(acceleration);
    AltStepper->setCurrentPosition(getStepsFordegrees(true, 90));

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
  }

  void setTarget(double alt, double az)
  {
    AltStepper->moveTo(getStepsFordegrees(true, alt));
    AzStepper->moveTo(getStepsFordegrees(false, az));
  }

  bool completedSlew()
  {
    return (AltStepper->currentPosition() == AltStepper->targetPosition() &&
            AzStepper->currentPosition() == AzStepper->targetPosition());
  }

private:
  AccelStepper* AzStepper; 
  AccelStepper* AltStepper;
  int moveamount;
};

#endif // motors_h