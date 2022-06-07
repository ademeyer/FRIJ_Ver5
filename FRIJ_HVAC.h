#pragma once

#define COMPRESSOR_ENABLE           PB9
#define SPEED_A                     PB12
#define SPEED_B                     PB13

s8                                  old_compr_mode                =         compr_mode;
const char*                         compressor_state[]            =         {"MAX", "ECO", "MID"};

void turnOn_off_Compressor(void)
{
  if (compr_mode > -1 && digitalRead(COMPRESSOR_ENABLE) == LOW)
    digitalWrite(COMPRESSOR_ENABLE, HIGH);
  else if (compr_mode <= -1)
  {
    digitalWrite(COMPRESSOR_ENABLE, LOW);
  }
}

void box_temperature_control(void)
{
  if (box_temp <= set_box_temp || !powered)
  {
    if (compr_mode != old_compr_mode && compr_mode > -1)
      old_compr_mode = compr_mode;
    compr_mode = -1;
  }
  else
  {
    if (compr_mode < 0)
      compr_mode = old_compr_mode;
    else if (compr_mode != old_compr_mode)
      old_compr_mode = compr_mode;
  }
}

void activateECO(void)
{
  digitalWrite(SPEED_A, LOW);
  digitalWrite(SPEED_B, LOW);
}

void activateMAX(void)
{
  digitalWrite(SPEED_A, HIGH);
  digitalWrite(SPEED_B, HIGH);
}

void activateMID(void)
{
  digitalWrite(SPEED_A, HIGH);
  digitalWrite(SPEED_B, LOW);
}

void FRIJ_HVAC_Setup(void)
{
  pinMode(SPEED_A, OUTPUT);
  pinMode(SPEED_B, OUTPUT);
  pinMode(COMPRESSOR_ENABLE, OUTPUT);
}

void FRIJ_HVAC_Loop(void)
{
  box_temperature_control();
  turnOn_off_Compressor();
  switch (compr_mode)
  {
    case 0:
      activateMAX();
      break;
    case 1:
      activateECO();
      break;
    case 2:
      activateMID();
      break;
  }
}
