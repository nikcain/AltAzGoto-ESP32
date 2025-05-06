#include "AppConnection.h"



WiFiServer m_server(80);

bool AppConnection::init()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin("homewifi", "pants-run-glad");

  const char* ntpServer = "pool.ntp.org";
  configTime(0, 0, ntpServer);
wl_status_t st;
do {
  st = WiFi.status();
  Serial.println(st);
  delay(200);
} while (st != 3);
  
  m_server.begin();
  return true;
}

void AppConnection::getStatus()
{

}

bool AppConnection::getCommand(JsonDocument &cmd)
{ 
  
  //Serial.println("getCommand");
  WiFiClient client = m_server.available();
  if (client) {
    
  Serial.println("client connected");
    String currentLine = "";
    bool currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        if (c == '\n' && currentLineIsBlank)
        {
          // EOF. Respond with current status
          //client.println("HTTP/1.1 200 OK\nall ok, probably more to add here...");
          
          //delay(100);
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
    JsonDocument doc;
    DeserializationError error = deserializeJson(cmd, content);
    if (error) {
      Serial.println("bad json: " + content);
      client.println("HTTP/1.1 400 Bad Request");
      delay(100);
      return false;
    }
    Serial.println("good json: " + content);

    client.println("HTTP/1.1 200 OK\nall ok, probably more to add here...");
    delay(100);
    client.stop();
  }
  return true;
}

void AppConnection::reconnect()
{

}
