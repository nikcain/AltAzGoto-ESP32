// modules:
// stepper moves
// input commands
// look up coordinates

#define MYDEBUG

#include "devicetime.h"
#include "SkyMap.h"
#include "AppConnection.h"
#include "motors.h"

AppConnection app;
int currentAction;
bool calibrating = false;
stepperMotors motors;

// status codes
#define INACTIVE -1
#define TRACKING 16
//celestialObjectID complete, ready to look up
#define LOOKUP 20 
//object found, waiting confirmation to slew
#define READYTOGO 21
#define SLEWING 22
#define SETTIME 23
#define MOVING 24
#define PACKAWAY 26

int updown = 0;
int leftright = 0;
int datetimeitembeingedited = 0;

SKYMAP_skymap_t skymap;
double currentRA;
double currentDEC;

#define mylatitude 52.6027972
#define mylongitude -3.0954659

void setup() {  
  
#ifdef MYDEBUG
  Serial.begin(9600);
  Serial.println("lets go!");
#endif
  app.init();
  calibrating = false;
  motors.init();
  currentAction = INACTIVE;

  SKYMAP_observer_position_t observation_location; 
  observation_location.latitude = mylatitude;
  observation_location.longitude = mylongitude;
  skymap.observer_position = observation_location;
}

void RADEC_to_ALTAZ(double ra, double dec, double& alt, double& az)
{
    SKYMAP_date_time_values_t dt;
    dt.year = getYear();
    dt.month = getMonth();
    dt.day = getDay();
    dt.hour = getHour();

    SKYMAP_star_t target;
    target.right_ascension = ra;
    target.declination = dec;

    skymap.date_time = dt;
    skymap.object_to_search = target;

    SKYMAP_search_result_t search_result = SKYMAP_observe_object(&skymap);
    alt = search_result.altitude;
    az = search_result.azimuth;
}

void loop() {
  AppData data;
  while (app.getCommand(data)) {
    switch (data.cmd) {
      case key_goto:
        if (currentAction == LOOKUP)
        {
          currentRA = data.RA;
          currentDEC = data.dec;
          double alt, az;
          RADEC_to_ALTAZ(currentRA, currentDEC, alt, az);
          motors.setTarget(alt, az);
          currentAction = SLEWING;
        }
        else {
          currentAction = (currentAction == TRACKING) ? INACTIVE : TRACKING;
        }
        break;
      case key_up:
      case key_down:
      case key_left:
      case key_right:
        updown = (data.cmd == key_up) ? 1 : (data.cmd == key_down) ? -1 : 0;
        leftright = (data.cmd == key_left) ? -1 : (data.cmd == key_right) ? 1 : 0;
        if (currentAction != SETTIME) {
          motors.Move(updown,leftright,0,calibrating); // alt az cal
          currentAction = INACTIVE;
        }
        break;
      case key_calibrate:
        if (calibrating) {
          calibrating = false;
        }
        else {
          calibrating = true;
        }
        break;
      case key_home:
        // sets position to vertical up, az pointing north
        currentAction = PACKAWAY;
        break;
      default:
        break;
    }
    delay(200);
  }

  // state machine actions
  switch (currentAction) {
    case SLEWING:
      // slewing to object
      if (motors.completedSlew()) currentAction = TRACKING;
      break;
    // manual move. Values for direction, not steps (motors will handle amounts)
    case TRACKING:
      // tracking
      double alt, az;
      RADEC_to_ALTAZ(currentRA, currentDEC, alt, az);
      motors.setTarget(alt, az);
      break;
    case PACKAWAY:
        motors.setTarget(90,0);
    break;
    default:
      break;
  }
  motors.update();
}

