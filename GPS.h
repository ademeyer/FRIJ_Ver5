#pragma once

#ifndef debug
#define GPS Serial2
u32 getCord_timer = millis();
#endif

int ExtractData(char *val, char *GPS_data, int maxsize)
{
  int Len = 0;
#ifndef debug
  int len = strlen(val);
  uint32_t noDataTimer = 0;
  bool match = false;
  noDataTimer = millis();

  while (GPS.available() == 0)
  {
    if (millis() >= (noDataTimer + 750)) break;
  }

  while (GPS.available() > 0)
  {
    char c = GPS.read();
    if (c != val[Len] && Len < len)
    {
      Len = 0;
    }
    else
    {
      if (Len < maxsize)
        GPS_data[Len++] = c;
    }
    if (c == '*')
    {
      GPS_data[Len] = '\0';
      break;
    }
    delayMicroseconds(500);
  }
#endif
  return Len;
}

void setup_GPS()
{
#ifndef debug
  GPS.begin(9600);
#endif
}

void handleGetCoordinate()
{
#ifndef debug
  if (millis() >= (getCord_timer + 5000) && !valid_GPS)
  {
    getCord_timer = millis();

    char GPSBuffer[256] = "";

    int amt = 0;
    if ((amt = ExtractData("$GPRMC", GPSBuffer, sizeof(GPSBuffer))) > 0)
    {
      char *dest = strtok(GPSBuffer, ",");
      int pos = 0;
      while (dest != NULL)
      {
        dest = strtok(NULL, ",");

        if (pos == 1)
        {
          if (dest[0] == 'A') valid_GPS = true;
        }
        if (valid_GPS && pos == 2)
        {
          memset(G_Lat, 0, sizeof(G_Lat));
          strcpy(G_Lat, dest);
        }
        else if (valid_GPS && pos == 3)
        {
          strcat(G_Lat, ",");
          strcat(G_Lat, (char*)dest[0]);
        }
        if (valid_GPS && pos == 4)
        {
          memset(G_Long, 0, sizeof(G_Long));
          strcpy(G_Long, dest);
        }
        else if (valid_GPS && pos == 5)
        {
          strcat(G_Long, ",");
          strcat(G_Long, (char*)dest[0]);
        }
        pos++;
      }
    }
    memset(GPSBuffer, 0, sizeof(GPSBuffer));
  }
#endif
}
