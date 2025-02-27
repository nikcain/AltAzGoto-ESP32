#ifndef devicetime_h
#define devicetime_h

tm getTime() {
  time_t now;
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  return timeinfo;
}

int getYear() { 
  tm timeinfo = getTime();
  return timeinfo.tm_year;
}

int getMonth() { 
  tm timeinfo = getTime();
  return timeinfo.tm_mon;
}

int getDay() { 
  tm timeinfo = getTime();
  return timeinfo.tm_mday;
}

int getHour() 
{
  tm timeinfo = getTime();
  return timeinfo.tm_hour;
}

int getMinute()
{
  tm timeinfo = getTime();
  return timeinfo.tm_min;
}

int getSeconds()
{
  tm timeinfo = getTime();
  return timeinfo.tm_sec;
}

String getTimeString()
{
  char buf1[10];
  sprintf(buf1, "%02d:%02d:%02d",  getHour(), getMinute(), getSeconds()); 

  return String(buf1);
}

String getDateString()
{
  char buf1[10];
  sprintf(buf1, "%02d/%02d/%04d",  getDay(), getMonth(), getYear());
  return String(buf1);
}


#endif //devicetime_h