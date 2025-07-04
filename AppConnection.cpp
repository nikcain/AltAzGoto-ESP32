#include "AppConnection.h"

#define LED 2

WiFiServer m_server(80);

bool AppConnection::init()
{
  Serial.println("app init");
  WiFi.mode(WIFI_STA);
  Serial.println("pre bgin");
  wl_status_t r = WiFi.begin("homewifi", "pants-run-glad");
  Serial.println("post begin "+ String(r));

  //const char* ntpServer = "pool.ntp.org";
  //Serial.println("pre config");
  //configTime(0, 0, ntpServer);
  //Serial.println("post config");
wl_status_t st;
do {
  st = WiFi.status();
  Serial.println(st);
  delay(200);
} while (st != 3);
  
  m_server.begin();
  Serial.println(WiFi.localIP());
  return true;
}

bool AppConnection::getCommand(JsonDocument &cmd, String currentStatus)
{ 
  
  //Serial.println("getCommand");
  WiFiClient client = m_server.available();
  if (client) {
    
  //Serial.println("client connected");
    String currentLine = "";
    bool currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank)
        {
          break;
        }
        //Serial.print(c);
        currentLine += c;
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }

    //Serial.println("header is " + currentLine);
    int p = currentLine.indexOf("Content-Length:");
    String contentline = currentLine.substring(p);
    p = contentline.indexOf("\n");
    String contentlength = contentline.substring(16, p);
    //Serial.println("content length is");
    //Serial.println(contentlength);
    String content;
    for (int i = 0; i<contentlength.toInt(); i++) {
      content += (char)client.read();
    }
    DeserializationError error = deserializeJson(cmd, content);
    if (error) {
      Serial.println("bad json: " + content);
      client.println("HTTP/1.1 400 Bad Request");
      //delay(100);
      return false;
    }
    //Serial.println("good json: " + content);
    //String currentStatus = "{\"Tracking\":false,\"Calibrating\":false,\"DateTimeSet\":true, \"targetRA\":1.234,\"targetDEC\":5.6789,\"currentRA\":0.1111,\"currentDEC\":0.2222}";
    if (cmd["year"]) //content != "{}")
    {
      //Serial.println("inbound: " + content);
        client.println("HTTP/1.1 200 OK");
    }
    else
    {
      //Serial.println("return http 200 status plus device status: " + currentStatus);
      int msglen = currentStatus.length();
      client.println("HTTP/1.1 200 OK\nContent-Length: "+String(msglen)+"\n\n" + currentStatus);
    }
    //delay(500);
    client.stop();
    
    return true;
  }
  return false;
}

void AppConnection::reconnect()
{

}
