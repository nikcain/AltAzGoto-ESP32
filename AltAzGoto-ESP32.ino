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

#define LED 2

// status codes
#define INACTIVE -1
#define TRACKING 16
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

  pinMode(LED, OUTPUT);

//#ifdef MYDEBUG
  Serial.begin(9600);
  Serial.println("lets go!");
//#endif

  app.init();
  
  digitalWrite(LED,HIGH);
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

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
  //Serial.println("RADEC_to_ALTAZ " + String(ra) + " " + String(dec));
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
    
  //Serial.println("RADEC_to_ALTAZ result" + String(alt) + " " + String(az));
}

void loop() {
  JsonDocument data;
  double a1,a2;
  motors.getCurrentPostion(a1,a2);
  //Serial.println("current pos " + String(a1) + " " + String(a2));
  String currentStatus = "{\"Tracking\":" + String((currentAction==TRACKING)?"true":"false") + 
                          ",\"Calibrating\":" + String((calibrating)?"true":"false") + 
                          ",\"DateTimeSet\":true, \"targetRA\":1.234,\"targetDEC\":5.6789,\"currentRA\":" + String(a1) + 
                          ",\"currentDEC\":" + String(a2) + "}";
  //Serial.println(currentStatus);
  while (app.getCommand(data, currentStatus)) {
    if (data["year"])
    {
      if (!deviceTimeSet())
      {
        Serial.println("setting time");
        setDeviceTime(data["year"], data["month"], data["day"], data["hour"], data["minutes"], data["seconds"]);
        Serial.println(getTimeString());
      }
    }
    if (data["message"])
    {
      JsonDocument msg = data["message"];
      Serial.println("messagetype " + String(data["messageType"]));
      if (data["messageType"] == "SetTarget") {
          currentRA = msg["RA"];
          currentDEC = msg["DEC"];
          double alt, az;
          RADEC_to_ALTAZ(currentRA, currentDEC, alt, az);
          Serial.println("setting target alt: " + String(alt) + " az: " + String(az));
          motors.setTarget(alt, az);
          currentAction = SLEWING;
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
        Serial.println("have tracking msg");
        if (msg["Tracking"]) 
        {
          Serial.println("tracking on");
          currentAction = TRACKING;
        }
        else {
          Serial.println("tracking off");
          currentAction = INACTIVE;
        }
      }
      if (data["messageType"] == "Reset") {
        // sets position to vertical up, az pointing north
        currentAction = PACKAWAY;
      }
      if (data["messageType"] == "Stop") {
        // stop all motors
        currentAction = INACTIVE;
      }
    //delay(200);
    currentStatus = "{\"Tracking\":" + String((currentAction==TRACKING)?"true":"false") + 
                          ",\"Calibrating\":" + String((calibrating)?"true":"false") + 
                          ",\"DateTimeSet\":true, \"targetRA\":1.234,\"targetDEC\":5.6789,\"currentRA\":" + String(a1) + 
                          ",\"currentDEC\":" + String(a2) + "}";
    }
  }

  // state machine actions
  switch (currentAction) {
    case SLEWING:
      // slewing to object
      Serial.println("Completed Slew, now tracking");
      currentAction = TRACKING;        
      break;
    case TRACKING:
      // tracking - recalculate alt/az for current time, and
      // move there
      Serial.println("Tracking");
      double alt, az;
      RADEC_to_ALTAZ(currentRA, currentDEC, alt, az);
      motors.setTarget(alt, az);
      break;
    case PACKAWAY:
      motors.setTarget(90,0);
    break;
    case INACTIVE:
      //Serial.print("-");
      break;
    default:
      break;
  }
  motors.update();
}

