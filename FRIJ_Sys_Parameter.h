#pragma once

s16 DateAndTimeSetting(u8* value, u16 len, s8 _mode)
{
  //$$,860665050927986,1,000,22,6,4,8,20,0*
  //value: char
#ifdef debug
  FRIJ.printf("mode: %d, Time str: %s, len: %d\r\n", _mode, value, len);
#endif
  if(_mode == 1)
  {
    strncpy(ttimeBuf, (char*)value, len);
    gprs_time_found = true;
    frij_time_synchronized = false;
    tLen = len;
    return 0;
  }
  else
  {
    return getTime_string((char*)value);
  }
  
}
s16 ModeSetting(u8* value, u16 len, s8 _mode)
{
  //eeprom_addr = 0 - 1 | value: integer
  return parameterMemoryReadAndWrite((0 + Parameter_Address[0]), (1 + Parameter_Address[0]), value, len, _mode);
}

s16 ConnectionTypeSetting(u8* value, u16 len, s8 _mode)
{
  /*
     0 = TCP
     1 = HTTP
     2 = Websocket
  */
  //eeprom_addr = 1 - 2 | value: integer
  return parameterMemoryReadAndWrite((1 + Parameter_Address[0]), (2 + Parameter_Address[0]), value, len, _mode);
}

s16 FrijBoxInformation(u8* value, u16 len, s8 _mode)
{
  if (_mode == 1) return 0;

  char foo[64] = "";
  s16 p = 0, vlen = 0;
  p = snprintf(foo, sizeof(foo), "%ld", round((millis() - system_time) / 1000.0));    //sys running time
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", batLevel);                                     //sys battery level
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ':';

  p = snprintf(foo, sizeof(foo), "%d", adc_buffer[0]);                                //sys battery adc
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", box_temp);                                     //sys temperature
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", charg_stat);                                   //sys charging status
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", set_box_temp);                                 //set temperature
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", compr_mode);                                 //compressor mode
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  value[vlen++] = ',';

  p = snprintf(foo, sizeof(foo), "%d", lock);                                       //Frij mode
  for (int k = 0; k < p; k++)
    value[vlen++] = (u8)foo[k];

  if (vlen < len) return vlen;
  else return 0;
}

s16 NetworkInformation(u8* value, u16 len, s8 _mode)
{
  /*MCC, MNC, LAC, CID, CSQ*/
  if (_mode == 1) return 0;
  char foo[64] = "";
  s16 p = 0, vlen = 0;

  for (int k = 0; k < strlen(MCC); k++)
    value[vlen++] = (u8)MCC[k];

  value[vlen++] = ',';

  for (int k = 0; k < strlen(MNC); k++)
    value[vlen++] = (u8)MNC[k];

  value[vlen++] = ',';


  if (valid_GPS)
  {
    for (int k = 0; k < strlen(G_Lat); k++)
      value[vlen++] = (u8)G_Lat[k];

    value[vlen++] = ',';

    for (int k = 0; k < strlen(G_Long); k++)
      value[vlen++] = (u8)G_Long[k];

  }
  else
  {
    for (int k = 0; k < strlen(LAC); k++)
      value[vlen++] = (u8)LAC[k];

    value[vlen++] = ',';

    for (int k = 0; k < strlen(CID); k++)
      value[vlen++] = (u8)CID[k];
  }


  value[vlen++] = ',';

  for (int k = 0; k < strlen(CSQ); k++)
    value[vlen++] = (u8)CSQ[k];


  if (vlen < len) return vlen;
  else return 0;
}

s16 GSMInformation(u8* value, u16 len, s8 _mode)
{
  /*IMEI, Phone Number*/
  if (_mode == 1) return 0;
  char foo[64] = "";
  s16 p = 0, vlen = 0;

  for (int k = 0; k < strlen(IMEI); k++)
    value[vlen++] = (u8)IMEI[k];

  value[vlen++] = ',';

  for (int k = 0; k < strlen(ICCID); k++)
    value[vlen++] = (u8)ICCID[k];

  if (vlen < len) return vlen;
  else return 0;
}

s16 PeriodicDump(u8* value, u16 len, s8 _mode)
{
  //eeprom_addr = 2 - 3 | value: integer
  return parameterMemoryReadAndWrite((2 + Parameter_Address[0]), (3 + Parameter_Address[0]), value, len, _mode);
}


s16 ButtonFlagSetting(u8* value, u16 len, s8 _mode)
{
  //eeprom_addr = 3 - 4 | value: integer
  return parameterMemoryReadAndWrite((3 + Parameter_Address[0]), (4 + Parameter_Address[0]), value, len, _mode);
}


s16 BoxTemperatureSetting(u8* value, u16 len, s8 _mode)
{
  /*
     0 - run operation
     1 - suspend operation
  */
  //eeprom_addr = 4 - 5 | value: integer
  return parameterMemoryReadAndWrite((4 + Parameter_Address[0]), (5 + Parameter_Address[0]), value, len, _mode);
}

s16 ServerInformation(u8* value, u16 len, s8 _mode)
{
  /*
     Server IP/Domain, Port
  */

  //eeprom_addr = 5 - 37 | value: char
  return parameterMemoryReadAndWrite((5 + Parameter_Address[0]), (37 + Parameter_Address[0]), value, len, _mode);
}

s16 CustomAPNSetting(u8* value, u16 len, s8 _mode)
{
  //eeprom_addr = 37 - 69 | value: char
  return parameterMemoryReadAndWrite((37 + Parameter_Address[0]), (69 + Parameter_Address[0]), value, len, _mode);
}

s16 GRDSetting(u8* value, u16 len, s8 _mode)
{
  //eeprom_addr = 69 - 101 | value: char
  return parameterMemoryReadAndWrite((69 + Parameter_Address[0]), (101 + Parameter_Address[0]), value, len, _mode);
}



s16 ParameterOperation(u8 No, u8* value, u16 len, s8 data_mode)
{
  if (data_mode < 0 || data_mode > 1 || No < 0 || No >= MAX_Para)
  {
#ifdef debug
    FRIJ.printf(F("Invalid ParameterOperation, cause by data_mode: %d or op: %d\n"), data_mode, op);
#endif
    return -1;
  }

#ifdef debug
  FRIJ.printf(F("op: %d\n"), No);
#endif

  typedef s16 (*fn)(u8*, u16, s8);
  static fn funcs[] =
  {
    //Parameter                                                             //code             //memory addresses
    DateAndTimeSetting,                                                     //0
    ModeSetting,                                                            //1                 0 - 1
    ConnectionTypeSetting,                                                  //2                 1 - 2
    PeriodicDump,                                                           //3                 2 - 3
    ButtonFlagSetting,                                                      //4                 3 - 4
    BoxTemperatureSetting,                                                  //5                 4 - 5
    ServerInformation,                                                      //6                 5 - 37  - 32bytes
    CustomAPNSetting,                                                       //7                 37 - 69 - 32bytes
    GRDSetting,                                                             //8                 69 - 101 - 32bytes
    /***** Parameter not saved in EEPROM ******/
    FrijBoxInformation,/*runtime, battery, temp, charging status*/          //9
    NetworkInformation, /*MCC, MNC, LAC, CID, CSQ*/                         //10
    GSMInformation, /*IMEI, Phone Number*/                                  //11
  };
  return funcs[No] (value, len, data_mode);
}
