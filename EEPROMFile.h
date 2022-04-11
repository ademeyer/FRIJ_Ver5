/*
  EEPROM Driver is only suitable for CAT24C256
*/
#pragma once

//Data Definition
#define EEPROMPAGE                  64
#define EEPROMMAX                   32768
#define ctrl_24fc1025               0x50

//Memory Allocation
const u32                           Parameter_Address[]               =         {0, 200};                                                         //200 bytes
const u32                           indexes_Address[]                 =         {(Parameter_Address[1] + 1), (Parameter_Address[1] + 8)};         //8 byte space
const u32                           Event[]                           =         {(indexes_Address[1] + 1), (EEPROMMAX - 1)};                      //Variable byte space


void getEventFreeMemoryIndex();
void clearEventSector(bool form);


void EEPROMSetup()
{
  Wire.begin();
  Wire.setClock(400000);
  delay(100);
  getEventFreeMemoryIndex();
#ifdef debug
  FRIJ.printf(F("Parameter_Address[0]: %ld, Parameter_Address[1]: %ld\n"), Parameter_Address[0], Parameter_Address[1]);
  FRIJ.printf(F("indexes_Address[0]: %ld, indexes_Address[1]: %ld\n"), indexes_Address[0], indexes_Address[1]);
  FRIJ.printf(F("Event[0]: %ld, Event[1]: %ld\n"), Event[0], Event[1]);
#endif
}


/*
   Same as EEPROM_Write_P (mem_handle_t *handle, u16 page, u16 offset, u8 *data, u16 size), convert memory address to page num
   @*handle is the struct properties of the EEPROM chip
   @eeprom_addr is desire address to start writing from
   @data is the user buffer where read data is saved
   @amt is the data length of data
*/
s16 write_eeprom(u32 eeprom_addr, u8 data)
{
  if (eeprom_addr >= EEPROMMAX)
    return -1;

  s16 i2c_addr = ctrl_24fc1025;

  if (eeprom_addr > 65535)
    i2c_addr = i2c_addr | B00000001;

  Wire.beginTransmission(i2c_addr);
  Wire.write((byte)(eeprom_addr >> 8));
  Wire.write((byte)(eeprom_addr & 0xFF));
  Wire.write(data);
  int err = Wire.endTransmission();
  delay(5);
  return err;
}

/*
   Same as EEPROM_Read_P (mem_handle_t *handle, u16 page, u16 offset, u8 *data, u16 size), convert memory address to page num
   @*handle is the struct properties of the EEPROM chip
   @eeprom_addr is desire address to start reading from
   @data is the user buffer where read data is saved
   @amt is the data length of data
*/
s16 read_eeprom(u32 eeprom_addr)
{
  s16 i2c_addr = ctrl_24fc1025;
  s16 ee_out = 0;

  if (eeprom_addr > 65535)
    i2c_addr = i2c_addr | B00000001;

  Wire.beginTransmission(i2c_addr);
  Wire.write((byte)(eeprom_addr >> 8));
  Wire.write((byte)(eeprom_addr & 0xFF));
  byte i2c_status = Wire.endTransmission();
  delay(1);

  if (i2c_status == 0)
  {
    Wire.requestFrom(i2c_addr, 1);
    while (Wire.available() > 0)
    {
      ee_out = Wire.read();
    }

    return ee_out;
  }
  else
    return -1;
}


s16 ReadMessage(u32 eeprom_addr, u8* byteArr, u16 maxLen)
{
  delay(10);
  u32 eLen = eeprom_addr;
  u32 nowLen = 0;
  u8 inMe = 0;
  char c = '\0';

  for (nowLen = 0; nowLen < maxLen; nowLen++)
  {
    inMe = read_eeprom(eLen++);
#ifdef debug
    if (FRIJ)
      FRIJ.printf(F("eLen: %d, inMe: %u\n"), eLen, inMe);
#endif
    if (inMe == 0xFF)
      break;
    else if (inMe > 0)
    {
      c = inMe;
      byteArr[nowLen] = c;
      if (c == endMsg && maxLen > 0) //if (c == endMsg && maxLen > 3)
      {
        nowLen++;
        break;
      }
    }
    else
    {
#ifdef debug
      if (FRIJ)
        FRIJ.printf(F("Empty byte: %d found!\n"), inMe);
#endif
      break;
    }
    delay(1);
  }
  return nowLen; //return number of byte read
}

s16 WriteMessage(u32 eeprom_addr, u8 *data, u16 len)
{
#ifdef debug
  if (FRIJ)
    FRIJ.printf(F("len: %d\n"), len);
#endif
  s16 stat = 0;
  u32 nowLen = eeprom_addr;
  for (int i = 0; i < len; i++)
  {
    if (stat = write_eeprom(nowLen, (byte)data[i]) != 0)
    {
#ifdef debug
      if (FRIJ)
        FRIJ.printf(F("ee write err: %d\n"), stat);
#endif
      //creatEvent(101, (s32)stat, (s32)nowLen, -1); // send critical memory failure event
      return stat;
    }
    nowLen++;
  }
  return (nowLen - eeprom_addr);
}

/*  read and write parameter with set location address
    parameter:    eeprom start address, eeprom stop address, data to write
    returns:      data read from set address location
*/
s16 parameterMemoryReadAndWrite(u32 startAddr, u32 stopAddr, u8* value, u16 len, s8 _mode)
{
  u32 val = 0;
  byte Byte[4] = {0};

  u8 noOfByte = stopAddr - startAddr;

  if (len > noOfByte) len = noOfByte;

  if (_mode == 1) //write
  {
    return WriteMessage(startAddr, value, len); //write_eeprom(u32 eeprom_addr, u8* data, u16 amt)
  }
  else          //read
  {
    return ReadMessage(startAddr, value, len); //read_eeprom(u32 eeprom_addr, u8* data, u16 amt)
  }

  return -1;
}


bool saveEventIndexes(u32 _l)
{
  byte ui[8] = {0};
  u8 j = 0;
  u32 _addr = indexes_Address[0];

  if ((saveIndex + _l) >= Event[1]) return false;
  //if ((saveIndex + _l) < Event[1])       //if saveIndex is within EEPROM event capacity
  //{
  byte pui[4] = {0};

#ifdef debug
  if (FRIJ)
  {
    FRIJ.printf(F("Memory space available: %ld\n"), (Event[1] - saveIndex));
  }
#endif

  make8((saveIndex + _l), 4, pui);

  for (int p = 0; p < 4; p++)
    ui[j++] = pui[p];

  make8(readIndex, 4, pui);

  for (int p = 0; p < 4; p++)
    ui[j++] = pui[p];
  /*}

    else                      //memory has overflown, clear saveIndex
    {
    #ifdef debug
    if (FRIJ)
    {
      FRIJ.printf(F("Not enough space available: %ld\n"), (Event[1] - saveIndex));
    }
    #endif

    clearEventSector(true);
    }
  */

#ifdef debug
  if (FRIJ)
  {
    FRIJ.print(F("Indexes bytes: "));

    for (int k = 0; k < 8; k++)
    {
      FRIJ.printf(F("%d "), ui[k]);
    }

    FRIJ.println();
  }
#endif

  if (WriteMessage(_addr, ui, 8) == 8)
  {
    return true;
  }

#ifdef debug
  if (FRIJ)
  {
    FRIJ.println(F("saveEventIndexes error!"));
  }
#endif

  return false;
}

/*  get number of data available in the eeprom
    parameter:    null
    returns:      null
*/

void getEventFreeMemoryIndex()
{
#ifdef debug
  if (FRIJ)
    FRIJ.println(F("getEventFreeMemoryIndex"));
#endif
  saveIndex = Event[0];
  readIndex = Event[0];

  byte ui[8];
  u32 _addr = indexes_Address[0], i = 0;

#ifdef debug
  if (FRIJ)
  {
    FRIJ.printf(F("_addr: %d\n"), _addr);
  }
#endif

  s16 IsRead = 0;
  IsRead = ReadMessage(_addr, ui, 8); //read_eeprom(u32 eeprom_addr, u8* data, u16 amt)
  delay(5);

#ifdef debug
  if (FRIJ)
  {
    for (int k = 0; k < 8; k++)
      FRIJ.printf(F("%d\n"), ui[k]);
  }
#endif

  if (IsRead > 0)
  {
    byte pui[4] = {0};
    u8 j = 0;

    for (int k = 0; k < 2; k++) //it's going to handle saveIndex and readIndex
    {
      for (int p = 0; p < 4; p++)
        pui[p] = ui[j++];

      i = merge(pui, 4);

      if (i < Event[0] || i >= Event[1])
        i = Event[0];

      if (k == 0)
        saveIndex = i;
      else if (k == 1)
        readIndex = i;
    }
  }

#ifdef debug
  if (FRIJ)
  {
    FRIJ.printf(F("i: %d, saveIndex: %d, readIndex: %d\n"), i, saveIndex, readIndex);
  }
#endif
}


void clearEventSector(bool form)
{
#ifdef debug
  if (FRIJ)
  {
    FRIJ.println(F("clearEventSector"));
  }
#endif
  if (form)
    saveIndex = Event[0];

  readIndex = 0;
  readIndex = saveIndex;

  if (form)
    saveEventIndexes(0);

}
