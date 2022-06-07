#pragma once

#define GPS Serial2
u32 getCord_timer = millis() + 10000; //with 10s initial delay

u16 splitDelIntoArr(char *raw, u16 rawLen, char arr[][32], u16 sizeT)
{
  u16 list = 0, i = 0;
  for (int j = 0; j < rawLen; j++)
  {
    if (raw[j] == ',' || raw[j] == '\0' )
    {
      if (list < sizeT - 1)
        list++;
      i = 0;
    }
    else if (raw[j] == '*') break;
    else
    {
      //*arr[list] = raw[j];
      if (i < 31)
        arr[list][i++] = raw[j];
    }
  }
  return list;
}

int ExtractData(char *val, char *GPS_data, int maxsize)
{
  int Len = 0;
  int len = strlen(val);
  u32 noDataTimer = millis();
  bool match = false;

  while (GPS.available() == 0 && millis() < (noDataTimer + 250));
  char *bigchar;
  bigchar = (char*)malloc(2048);
  if (!bigchar)
  {
#ifdef debug
    FRIJ.printf("malloc failed\r\n");
#endif

    return 0;
  }

  int redl = GPS.readBytes(bigchar, 2048);
#ifdef debug
  FRIJ.write((u8*)bigchar, redl);
  FRIJ.println();
#endif
  Len = gsm_data_grabber(bigchar, redl, val, '*', GPS_data, maxsize);

  free(bigchar);
  return Len;
}

void setup_GPS(void)
{
#ifdef debug
#if !NEW_FRIJ
  return;
#endif
#endif
  GPS.begin(9600);
  GPS.setTimeout(150);

}

void handleGetCoordinate(void)
{
#ifdef debug
#if !NEW_FRIJ
  return;
#endif
#endif
  if (millis() >= (getCord_timer + 5000))
  {
    getCord_timer = millis();

    char GPSBuffer[256] = "";
#ifdef debug
    FRIJ.printf("Extracting GPS data...");
#endif
    int amt = 0;
    if ((amt = ExtractData("$GPGLL,", GPSBuffer, sizeof(GPSBuffer))) > 0)
    {
      //$GPGLL,0632.64291,N,00318.12342,E,095015.00,A,A*62
      //$GPGLL,,,,,094659.00,V,N*4D

#ifdef debug
      FRIJ.printf("Done\r\n");
      //FRIJ.printf("Done: %s:%d\r\n", GPSBuffer, amt);
#endif
      char splitted[8][32] = {""};

      u16 pos = splitDelIntoArr(GPSBuffer, amt, splitted, 8);
#ifdef debug
      for (int p = 0; p < pos; p++)
        FRIJ.printf("%d:%s\r\n", p, splitted[p]);
#endif
      if (splitted[5][0] == 'A')
      {
        valid_GPS = true;
      }
      else
        valid_GPS = false;

      if (valid_GPS)
      {
#ifdef debug
        FRIJ.println("valid GPS data!");
#endif
        memset(G_Long, 0, sizeof(G_Long));
        memset(G_Lat, 0, sizeof(G_Lat));
        strncat(G_Lat, splitted[2], sizeof(G_Lat));
        strncat(G_Long, splitted[0], sizeof(G_Long));
      }
    }
#ifdef debug
    else
    {
      FRIJ.printf("Failed: %s\r\n", GPSBuffer);
    }

    FRIJ.printf("G_Lat: %s, G_Long: %s\r\n", G_Long, G_Lat);
#endif
    memset(GPSBuffer, 0, sizeof(GPSBuffer));
  }
}
