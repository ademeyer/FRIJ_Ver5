#pragma once

const u8                            eeprom_stat[]                 =         {1, 3, 4, 5, 6, 7};
const u8                            eeprom_stat_no                =         6;//strlen((char*)eeprom_stat);

void form_standard_packet()
{
  if (op > -1 && BUFF_Len > 0 && BUFF_Len < (MAX_LEN / 2) && Res_Len == 0)
  {
    //Res_Len = 0;
    u16 len = 0;
    memset(RES_BUFF, 0, sizeof(RES_BUFF));
    strcpy(RES_BUFF, start_str);
    strncat(RES_BUFF, IMEI, strlen(IMEI));
    strcat(RES_BUFF, ",");
    tLen = getTime_string(ttimeBuf);
    strncat(RES_BUFF, ttimeBuf, tLen);
    strcat(RES_BUFF, ",");
    strcat(RES_BUFF, op_command[op]);
    strcat(RES_BUFF, ",");
    strncat(RES_BUFF, BUFF, BUFF_Len);
    strcat(RES_BUFF, end_str);
    strcat(RES_BUFF, '\0'); //null terminator
    Res_Len = strlen(RES_BUFF);
#ifdef debug
    FRIJ.printf(F("Formed: %s:%d\n"), RES_BUFF, Res_Len);
#endif
    if (tcp_alive)
    {
      gsm_stage = _NET_STAT;
    }
    else if (op == PD_No)
    {
#ifdef debug
      FRIJ.printf("\r\nLogging offline data: %s:%d\r\n");
#endif
      if ((saveIndex + Res_Len) >= (EEPROMMAX - 1) || readIndex > saveIndex)
      {
        clearEventSector(true);
        //delay(1);
      }

      if (WriteMessage(saveIndex, (u8*)RES_BUFF, Res_Len) == Res_Len) //WriteMessage(u32 eeprom_addr, u8 *data, u16 len)
      {
        delay(1);
        if (saveEventIndexes(Res_Len)) //if(saveEventIndex((len+saveIndex), saveStart[0]))
        {
          delay(1);
          saveIndex = saveIndex + Res_Len;
#ifdef debug
          if (FRIJ)
          {
            FRIJ.print(F("saved, saveIndex: "));
            FRIJ.print(saveIndex);
            FRIJ.print(F(" readIndex: "));
            FRIJ.println(readIndex);
          }
#endif
          Res_Len = 0;
        }
      }
    }
  }

  BUFF_Len = 0;
  op = -1;
}

void sendEvents()
{
  //not ready to run condidition here
  if (!tcp_alive || Res_Len != 0 || BUFF_Len != 0) return;
  if (dataAck)
  {
    dataAck = false;
    //clear eventBuf
    memset(RES_BUFF, 0, sizeof(RES_BUFF));
    Res_Len = 0;

    if (readIndex >= saveIndex && saveIndex > Event[0])
    {
      clearEventSector(false);
    }
  }
  //sending out control
  if (readIndex < saveIndex && Res_Len == 0)
  {
    Res_Len =  ReadMessage(readIndex, (u8*)RES_BUFF, sizeof(RES_BUFF)); //ReadMessage(u32 eeprom_addr, u8* byteArr, u16 maxLen)
    readIndex = readIndex +  Res_Len;
    gsm_stage = _NET_STAT;
#ifdef debug
    FRIJ.print(F("Res_Len: "));
    FRIJ.print(Res_Len);
    FRIJ.print(F(" readIndex: "));
    FRIJ.print(readIndex);
    FRIJ.printf(" from eeprom->%s:%d\r\n", RES_BUFF, Res_Len);
#endif
  }
}

void periodic_dump_packet()
{
  if (BUFF_Len != 0 || op != -1) return;
  if (millis() >= (pdTimer + (pdTime * 60 * 1000)) && pdTime > 0)
  {
    u8 value[128] = {0};
    s16 len = 0;
    BUFF_Len = 0;

    if ((len = FrijBoxInformation(value, sizeof(value), 0)) > 0)
    {
#ifdef debug
      FRIJ.printf(F("PD1 ->%d\n"), len);
#endif
      for (int i = 0; i < len; i++)
        BUFF[BUFF_Len++] = (char)value[i];
    }
    else
    {
      BUFF_Len = 0;
      //return;
    }
    if ((len = NetworkInformation(value, sizeof(value), 0)) > 0)
    {
      BUFF[BUFF_Len++] = ',';
#ifdef debug
      FRIJ.printf(F("PD2 - >%d\n"), len);
#endif
      for (int i = 0; i < len; i++)
        BUFF[BUFF_Len++] = (char)value[i];
    }
    else
    {
      BUFF_Len = 0;
      //return;
    }

    if (BUFF_Len > 0)
    {
      op = PD_No;
      if (valid_GPS) op = op + 1;
    }
    pdTimer = millis();
    beatTimer = millis();
  }
}

void heart_beat_packet()
{
  if (BUFF_Len != 0 || op != -1)
  {
    beatTimer = millis();
    return;
  }
  if (tcp_alive && millis() >= (beatTimer + MAX_BEAT_TIME))
  {
    u8 value[128] = {0};
    u8 len = 0;
    if ((len = (u8)NetworkInformation(value, sizeof(value), 0)) > 0)
    {
      for (int i = 0; i < len; i++)
        BUFF[BUFF_Len++] = (char)value[i];
#ifdef debug
      FRIJ.printf(F("HB --> %s:%d:%d\n"), BUFF, len, BUFF_Len);
#endif
      op = HB_No;
    }
    beatTimer = millis();
  }
}

void operate_on_data()
{
  if (BUFF_Len != 0 || op != -1) return;
  if (operate)
  {
    operate = false;
    if (inputLen > 1)
    {
      char foo[MAX_LEN] = "", _next[MAX_LEN] = "";
      u8 len = 0;
      /* start_of_msg, imei, datamode, system_address, other data, end_of_msg*/
      //$$,862643039032861,0,002,1*
      //$$,862643039032861,1,001,1*
      //$$,862643039032861,1,007,Hello*
#ifdef debug
      FRIJ.printf(F("operate->%s: %d\n"), userInput, inputLen);
#endif
      //grab data mode
      strcat(_next, IMEI);
      strcat(_next, ",");
      len = gsm_data_grabber(userInput, inputLen, _next, ',', foo, sizeof(foo));

      s8 data_mode = atoi(foo);
      strcat(_next, foo);
      memset(foo, 0, sizeof(foo));

      //grab system para address
      strcat(_next, ",");
      len = gsm_data_grabber(userInput, inputLen, _next, ',', foo, sizeof(foo));

      if ((op = progNumber(foo)) < 0)
      {
#ifdef debug
        FRIJ.printf(F("invalid sys_addr: %s\n"), foo);
#endif
        /*
                if (strstr(userInput, "ACK") != NULL)
                {
          #ifdef debug
                  FRIJ.printf("server ack data!\r\n");
          #endif
                  dataAck = true;
                }
        */
        inputLen = 0;
        memset(userInput, 0, sizeof(userInput));
        return;
      }
      byte _value[32] = {0}, r = 0;
      beatTimer = millis();
      if (data_mode)
      {
        memset(_next, 0, sizeof(_next));
        strcat(_next, foo);
        strcat(_next, ",");
        len = gsm_data_grabber(userInput, inputLen, _next, '*', foo, sizeof(foo));
#ifdef debug
        FRIJ.printf(F("_next: %s - data: %s\n"), _next, foo);
#endif


        if (ArrayIsDigit((u8*) foo, len))
        {
          _value[r++] = atoi(foo) & 0xFF;
#ifdef debug
          FRIJ.printf(F("r is %d, Looks like we have digit %d\n"), r, _value[0]);
#endif
        }
        else if (ArrayIsChar((u8*) foo, len))
        {
#ifdef debug
          FRIJ.printf(F("len is %d, Looks like we have char %s\n"), len, foo);
#endif
          for (int p = 0; p < len; p++)
          {
            _value[r++] = foo[p];
          }
          _value[r++] = 0xFF; //terminating char
        }
        else
        {
#ifdef debug
          FRIJ.printf(F("unknown data!\n"));
#endif
        }

        if (r == ((u8)ParameterOperation(op, _value, r, 1)))
        {
          BUFF[BUFF_Len++] = 'A';
          BUFF[BUFF_Len++] = 'C';
          BUFF[BUFF_Len++] = 'K';
          EEPROM_Read = false;
        }
        else
        {
          BUFF[BUFF_Len++] = 'N';
          BUFF[BUFF_Len++] = 'A';
          BUFF[BUFF_Len++] = 'K';
        }
      }
      else
      {
        BUFF_Len = 0;
        memset(BUFF, 0, sizeof(BUFF));
        r = ParameterOperation(op, _value, sizeof(_value), 0);
        if (r == 1)
        {
#ifdef debug
          FRIJ.printf(F("r is %d, Looks like we have digit!\n"), r);
#endif
          for (int p = 0; p < r; p++)
          {
            memset(foo, 0, sizeof(foo));
            int pr = sprintf(foo, "%d", _value[p]);
            for (int t = 0; t < pr; t++)
              BUFF[BUFF_Len++] = foo[t];
          }
        }
        else if (ArrayIsChar(_value, r))
        {
#ifdef debug
          FRIJ.printf(F("r is %d, Looks like we have char: %s!\n"), r, _value);
#endif
          for (int t = 0; t < r; t++)
            BUFF[BUFF_Len++] = _value[t];
        }

        else
        {
#ifdef debug
          FRIJ.printf(F("we are unsure of what is saved %d!\n"), _value[0]);
#endif
          BUFF[BUFF_Len++] = 'N';
          BUFF[BUFF_Len++] = 'U';
          BUFF[BUFF_Len++] = 'L';
          BUFF[BUFF_Len++] = 'L';
        }
      }
      FRIJ.printf(F("BUFF: %s:%d\n"), BUFF, BUFF_Len);
    }
  }
  inputLen = 0;
}

void EEPROM_Read_Statics()
{
  if (!EEPROM_Read)//{1, 3, 4, 5, 6, 7};
  {
    u8 value[32];
    s16 len = 0;
    for (int i = 0; i < eeprom_stat_no; i++)
    {
      len = 0;
      if ((len = ParameterOperation((eeprom_stat[i] - 1), value, sizeof(value), 0)) > 0)
      {
#ifdef debug
        FRIJ.printf(F("no #%d--> paraID #%d:%d\n"), i, eeprom_stat[i], value[0]);
#endif
        if      (i == 0) mode = value[0];
        //else if (i == 1) pdTime = value[0];
        else if (i == 2) lock = value[0];
        else if (i == 3) set_box_temp = (s8)value[0];
        else if (i == 4)
        {
          if (ArrayIsChar(value, len))
          {
#ifdef debug
            FRIJ.printf(F("Server Info: %s, len: %d\n"), value, len);
#endif
          }
        }
        else if (i == 5)
        {
          if (ArrayIsChar(value, len))
          {
#ifdef debug
            FRIJ.printf(F("APN: %s, len: %d\n"), value, len);
#endif
          }
        }
      }
    }

    EEPROM_Read = true;
    clear_disp = true;
  }
}


void FRIJ_Operation_Loop()
{
  operate_on_data();
  EEPROM_Read_Statics();
  form_standard_packet();
  heart_beat_packet();
  periodic_dump_packet();
  sendEvents();
  
#ifdef debug
#if LOGLEVEL
  if (FRIJ.available() > 0 && inputLen == 0)
  {
    if ((inputLen = FRIJ.readBytesUntil('\r', userInput, sizeof(userInput))) > 0)
    {
      FRIJ.printf(F("debug input: "));
      FRIJ.write((u8*)userInput, inputLen);
      FRIJ.println();

      operate = true;
      useUSB = true;
    }
  }
#endif
#endif

}
