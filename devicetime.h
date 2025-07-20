#ifndef devicetime_h
#define devicetime_h

unsigned long timeoffset = 0;
unsigned int year=2024;
unsigned int month = 11;
unsigned int day = 29;
static bool isTimeSet = false;

bool deviceTimeSet()
{
  return isTimeSet;
}

void setDeviceTime( unsigned int _year, unsigned int _month, unsigned int _day,
                    unsigned long _h, unsigned long _m, unsigned long _s)
{
  year = _year;
  month = _month;
  day = _day;
  _h+=24;
  _m+=60;
  _s+=60;
  timeoffset =  ((_h % 24 * 60 * 60 * 1000) +
                (_m % 60 * 60 * 1000) +
                (_s % 60 * 1000)) - millis();
  Serial.println("offset = " + String(timeoffset));
  isTimeSet = true;
}

int getYear() { return year; }
int getMonth() { return month; }
int getDay() { return day; }
int getHour() 
{
  return ((((millis() + timeoffset) / 1000) / 60) / 60) % 24;
}

int getMinute()
{
  return (((millis() + timeoffset) / 1000) / 60) % 60;
}

int getSeconds()
{
  return ((millis() + timeoffset) / 1000) % 60;
}

String getTimeString()
{
  char buf1[10];
  sprintf(buf1, "%02d:%02d:%02d",  getHour(), getMinute(), getSeconds()); 

  return String(buf1);
}

String getDateString()
{
  char buf1[12];
  sprintf(buf1, "%02d/%02d/%04d",  day, month, year);
  return String(buf1);
}


#endif //devicetime_h