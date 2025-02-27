#include "AppConnection.h"

#include <ArduinoJson.h>


WiFiServer m_server(80);

bool AppConnection::init()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("homewifi", "pants-run-glad");

  const char* ntpServer = "pool.ntp.org";
  configTime(0, 0, ntpServer);

  m_server.begin();
  return true;
}

void AppConnection::getStatus()
{

}

bool AppConnection::getCommand(AppData &cmd)
{ 
  WiFiClient client = m_server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        if (c == '\n')
        {
          // EOF. Respond with current status
          client.println("all ok, probably more to add here...");
          break;
        }
        else {
          currentLine += c;
        }
      }
    }
    client.stop();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, currentLine);
    if (error) return false;

    cmd.cmd = doc["cmd"];
    cmd.dec = doc["dec"];
    cmd.RA = doc["RA"];


  }
  return true;
}

void AppConnection::reconnect()
{

}
