#ifndef appconnection_h
#define appconnection_h

#include <Arduino.h>
#include <WiFi.h>

#define HOST "192.168.125.122"
#define PORT 3333

#define key_goto 1
#define key_left 2
#define key_right 3
#define key_up 4
#define key_down 5
#define key_calibrate 6
#define key_home 7

struct AppData
{
  int cmd;
  double RA;
  double dec;
};

class AppConnection 
{
  public:

  bool init();
  void getStatus();
  bool getCommand(AppData &cmd);

  private:
    void reconnect();
  void sendRequest();

};

#endif // appconnection_h