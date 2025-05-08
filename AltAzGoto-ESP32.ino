// modules:
// stepper moves
// input commands
// look up coordinates

#define MYDEBUG 1

#include "time.h"
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
#define MOVING 24
#define PACKAWAY 26

int updown = 0;
int leftright = 0;
int datetimeitembeingedited = 0;

SKYMAP_skymap_t skymap;
double currentRA;
double currentDEC;

double mylatitude = 52.6027972;
double mylongitude = -3.0954659;

void setup() {  

//#ifdef MYDEBUG
  Serial.begin(9600);
  Serial.println("lets go!");
//#endif

  app.init();
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 0;
  const int   daylightOffset_sec = 3600;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(100);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

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
  JsonDocument data;
  while (app.getCommand(data)) {
      JsonDocument msg = data["message"];
      if (data["messageType"] == "SetTarget") {
        if (currentAction == LOOKUP)
        {
          currentRA = msg["RA"];
          currentDEC = msg["message"]["DEC"];
          double alt, az;
          RADEC_to_ALTAZ(currentRA, currentDEC, alt, az);
          motors.setTarget(alt, az);
          currentAction = SLEWING;
        }
        else {
          currentAction = (currentAction == TRACKING) ? INACTIVE : TRACKING;
        }
      }
      if (data["messageType"] == "Move") {
        updown = (msg["Move"] == "up") ? 1 : (msg["Move"] == "down") ? -1 : 0;
        leftright = (msg["Move"] == "left") ? -1 : (msg["Move"] == "right") ? 1 : 0;
        motors.Move(updown,leftright,0,calibrating); // alt az cal
        currentAction = INACTIVE;
      }
      if (data["messageType"] == "SetCalibration") {
        calibrating = msg["Calibration"];
      }
      if (data["messageType"] == "SetTracking") {
        calibrating = msg["Tracking"];
      }
      if (data["messageType"] == "Reset") {
        // sets position to vertical up, az pointing north
        currentAction = PACKAWAY;
      }
      if (data["messageType"] == "Stop") {
        // stop all motors
        currentAction = INACTIVE;
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

