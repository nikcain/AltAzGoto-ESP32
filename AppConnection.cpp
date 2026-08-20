#include "AppConnection.h"

#define LED 2
const char *ssid = "Astro";
const char *password = "pants-run-glad";

bool connectedtoservermsg = true;

WiFiServer m_server(80);

bool AppConnection::init()
{
  Serial.println("app init");
  m_gotConnection = false;
  
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  m_server.begin();
  return true;
}

String AppConnection::checkForCommand( bool Tracking, bool Calibrating, bool DateTimeSet, double targetRA, double targetDEC, double currentAlt, double currentAz)
{
 //Serial.println("getCommand");
 String ret = "None";
  WiFiClient client = m_server.available();
  if (client) { 
    Serial.println("client connected");
    String currentLine = "";
    bool currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        m_mobileIP = client.remoteIP();
        if (connectedtoservermsg) {
          log("Connected to server");
          connectedtoservermsg = false;
        }
        if (!m_gotConnection) { Serial.println("got cnx from " + m_mobileIP.toString()); }
        m_gotConnection = true;
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
      else
      {
        connectedtoservermsg = false;
        digitalWrite(LED, LOW);
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
    DeserializationError error = deserializeJson(m_latestCmd, content);
    if (error) {
      Serial.println("bad json: " + content);
      client.println("HTTP/1.1 400 Bad Request");
      log("bad json");
      //delay(100);
      return "None";
    }
    //Serial.println("good json: " + content);
    //log(content);
    if (m_latestCmd["year"]) //content != "{}")
    {
      //Serial.println("inbound: " + content);
      client.println("HTTP/1.1 200 OK");  
      digitalWrite(LED, HIGH);
      ret ="Date";
    }
    else
    {
      String currentStatus = "{\"Tracking\":" +Tracking+ ",\"Calibrating\":" +Calibrating+ ",\"DateTimeSet\":" +DateTimeSet+ ", \"targetRA\":" +targetRA+ ",\"targetDEC\":" +targetDEC+ ",\"currentAlt\":" +currentAlt+ ",\"currentAz\":" +currentAz+ "}";
    
      //Serial.println("return http 200 status plus device status: " + currentStatus);
      int msglen = currentStatus.length();
      client.println("HTTP/1.1 200 OK\nContent-Length: "+String(msglen)+"\n\n" + currentStatus);
      String c = m_latestCmd["messageType"];
      if (c == "Move") c += " " + String(m_latestCmd["message"]["Move"]);
      log(c);  
      digitalWrite(LED, HIGH);
      ret = c;
    }
    client.stop();
  }
  return ret;
}

void AppConnection::getCommand(JsonDocument &cmd, String currentStatus)
{         
  cmd = m_latestCmd;
  m_latestStatus = currentStatus;
}


void AppConnection::reconnect()
{

}

void AppConnection::log(String txt)
{
  if (!m_gotConnection) return;
  Serial.println("logging to " + m_mobileIP.toString() + " " + txt);
  NetworkClient client;

  if (client.connect(m_mobileIP, 5432)) {
    client.print(txt);
  }
  client.stop();
}