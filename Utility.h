#pragma once
#include <RTClock.h>

RTClock                             rtclock (RTCSEL_HSE);

//Data Definition
bool                                BitRead(u32 val, u8 pos);
time_t                              tt                            =         0;

void SecondCount (void)
{
  tt++;
}

u16 gsm_data_grabber(const char *rawstr, u16 len, const char *start_id, char end_id, char *grabs, u16 max_grab)
{
  u16 p = 0, j = 0, u = 0;
  char s_id[64] = "";
  u8 grab = 0;
  for (j; j < len; j++)
  {
    if ((strcmp(s_id, start_id) == 0) && !grab)
      grab = 1;

    char c = rawstr[j];
    if (grab == 0)
    {
      if (c == start_id[u])
        s_id[u++] = c;

      else
        u = 0;
    }

    if (c == end_id && end_id != '\0' && grab == 1)
      break;

    if (grab == 1 && p < max_grab)
    {
      grabs[p++] = c;
    }
  }

  grabs[p] = '\0';

  return p;
}

void putStringInArray(const char* s, const char* oldW, const char* newW, char* result, int rLen)
{
  //char* result;
  int i, cnt = 0;
  int newWlen = strlen(newW);
  int oldWlen = strlen(oldW);

  // Counting the number of times old word
  // occur in the string
  for (i = 0; s[i] != '\0'; i++)
  {
    if (strstr(&s[i], oldW) == &s[i])
    {
      cnt++;
      // Jumping to index after the old word.
      i += oldWlen - 1;
    }
  }

  i = 0;
  while (*s)
  {
    // compare the substring with the result
    if (strstr(s, oldW) == s)
    {
      strcpy(&result[i], newW);
      i += newWlen;
      s += oldWlen;
    }
    else
    {
      if (i < rLen)
        result[i++] = *s++;
    }
  }

  result[i] = '\0';
}

s8 progNumber(char *buf)
{
  s8 f = -1;
  for (s8 i = 0; i < MAX_Para; i++)
  {
    if (strstr(buf, op_command[i]) != NULL)
    {
      f = i;
#ifdef debug
      FRIJ.printf(F("found match %s:%d\n"), op_command[i], f);
#endif
      break;
    }
  }

  return f;
}

int makeIntArr(char *buf, int len, s16* arrInt)
{
  char _temp[10];
  int j = 0, Len = 0;

  for (int i = 0; i <= len; i++)
  {
    char c = buf[i];
    if ((c == ',') || (c == ' ') || (c == '.') || (c == '/') || (c == ':') || i == len)
    {
      if (j > 0 && strlen(_temp) > 0)
      {
        arrInt[Len++] = (s16)atoi(_temp);
        memset(_temp, 0, sizeof(_temp));
        j = 0;
      }
    }
    else if (c == '-')
    {
      arrInt[Len++] = -1;
      memset(_temp, 0, sizeof(_temp));
      i++; //move i one step
    }
    else if (c >= 48 && c <= 57)
    {
      _temp[j++] = c;
    }

  }
  return Len;
}

u32 merge(u8 *_data, u8 _len)
{
  u32 bulk = 0;
  u8 sh = 0;

  if (_len == 1) return (u32)_data[0];
  else if (_len == 2) sh = 8;
  else if (_len == 3) sh = 16;
  else if (_len <= 4) sh = 24;
  else return 0;

  for (int i = _len - 1; i > -1; i--)
  {
    bulk = bulk | (_data[i] << sh);
    sh = sh - 8;
  }

  return bulk;
}


bool make8(u32 _data, u8 byte_no, u8* _sm)
{
  u8 m[] = {0, 8, 16, 24};

  if (byte_no > 4)return false;

  for (int i = 0; i < byte_no; i++)
  {
    *_sm++ = (_data >> m[i]) & 0xFF;
  }
  return true;
}

void RTC_Setup(void)
{
  rtclock.attachSecondsInterrupt(SecondCount);
}

void setDateTime(char *buf, int len)
{
  //yr, mn, dy, hr, mm,ss;
  s16 timeTemp[6] = {0};
  s16 P = makeIntArr(buf, len, timeTemp);
  if (P < 6 ) return;
  tm_t tm;
  if (timeTemp[0] >= 2000) timeTemp[0] = timeTemp[0] - 2000;
  tm.year    =   (u8)timeTemp[0];
  tm.month   =   (u8)timeTemp[1];
  tm.day     =   (u8)timeTemp[2];
  tm.hour    =   (u8)timeTemp[3];
  tm.minute  =   (u8)timeTemp[4];
  tm.second  =   (u8)timeTemp[5];

  rtclock.setTime(tm);
}

s16 getTime_string(char* timeStr)
{
  tm_t mtt;
  rtclock.breakTime(rtclock.now(), mtt);
  delay(1);

  s16 len = 0;
  len = sprintf(timeStr, "%u/%u/%u %02u:%02u:%02u", mtt.year/* + 2000*/, mtt.month, mtt.day, mtt.hour, mtt.minute, mtt.second);
  return len;
}


void creatEvent(u8 code, s32 val_1, s32 val_2, s32 val_3)
{
  s16 num = 0;
  BUFF_Len = 0;
  char buffer[MAX_LEN] = "";
  num = snprintf((char*)buffer, sizeof(buffer), "%d", 0);

  for (int i = 0; i < num; i++)   //add data mode
    BUFF[BUFF_Len++] = buffer[i];

  BUFF[BUFF_Len++] = ',';

  num = snprintf((char*)buffer, sizeof(buffer), "%d", code);

  for (int i = 0; i < num; i++)   //add event code ID
    BUFF[BUFF_Len++] = buffer[i];

  if (val_1 > -1)
  {
    BUFF[BUFF_Len++] = ',';
    num = snprintf((char*)buffer, sizeof(buffer), "%ld", val_1);

    for (int i = 0; i < num; i++)   //add value
      BUFF[BUFF_Len++] = buffer[i];
  }

  if (val_2 > -1)
  {
    BUFF[BUFF_Len++] = ',';
    num = snprintf((char*)buffer, sizeof(buffer), "%ld", val_2);

    for (int i = 0; i < num; i++)   //add value
      BUFF[BUFF_Len++] = buffer[i];
  }

  if (val_3 > -1)
  {
    BUFF[BUFF_Len++] = ',';
    num = snprintf((char*)buffer, sizeof(buffer), "%ld", val_3);

    for (int i = 0; i < num; i++)   //add value
      BUFF[BUFF_Len++] = buffer[i];
  }

  num = getTime_string(ttimeBuf);
  /*
    BUFF[BUFF_Len++] =  ',';

    for (int i = 0; i < num; i++)   //datetime str
      BUFF[BUFF_Len++] = ttimeBuf[i];
  */

  op = RE_No;

  //if (code != 25 && code != 101 && code != 102)
  //saveInfo = true;
}


u16 mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return round((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}//isPrintable

bool ArrayIsChar(u8* arr, u16 len)
{
  bool is_Char = 1;
  for (int t = 0; t < len; t++)
  {
    is_Char &= (isAscii(arr[t]) && isPrintable(arr[t])) ? 1 : 0;
  }
  return is_Char;
}

bool ArrayIsDigit(u8* arr, u16 len)
{
  bool is_digit = 1;
  for (int t = 0; t < len; t++)
  {
    is_digit &= isDigit((int)arr[t]) ? 1 : 0;
  }
  return is_digit;
}

int convertTemp(u16 adcAverage)
{
  // variables that live in this function
  float rThermistor = 0;            // Holds thermistor resistance value
  float tKelvin     = 0;            // Holds calculated temperature
  float tCelsius    = 0;            // Hold temperature in celsius

  /* Here we calculate the thermistor’s resistance using the equation
     discussed in the article. */
  rThermistor = 9710.0 * ( (4095.0 / adcAverage) - 1);

  /* Here is where the Beta equation is used, but it is different
     from what the article describes. Don't worry! It has been rearranged
     algebraically to give a "better" looking formula. I encourage you
     to try to manipulate the equation from the article yourself to get
     better at algebra. And if not, just use what is shown here and take it
     for granted or input the formula directly from the article, exactly
     as it is shown. Either way will work! */
  tKelvin = (3974.0 * 298.15) /
            (3974.0 + (298.15 * log(rThermistor / 10000.0)));

  /* I will use the units of Celsius to indicate temperature. I did this
     just so I can see the typical room temperature, which is 25 degrees
     Celsius, when I first try the program out. I prefer Fahrenheit, but
     I leave it up to you to either change this function, or create
     another function which converts between the two units. */
  tCelsius = tKelvin - 273.15;  // convert kelvin to celsius

  return (int)tCelsius;    // Return the temperature in Celsius
}

void synchronize_time(void)
{
  //format: yr/mn/dd, hr:mm:ss+tz
  if (gprs_time_found && !frij_time_synchronized)
  {
    setDateTime(ttimeBuf, tLen);
#ifdef debug
    FRIJ.println(F("Time Synchronized!"));
#endif
    memset(ttimeBuf, 0, sizeof(ttimeBuf));
    tLen = 0;
    frij_time_synchronized = true;
  }
}
