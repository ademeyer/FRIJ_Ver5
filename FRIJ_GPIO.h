#pragma once

#define USEINTERRUPT                1
#define SECONDFUNCTIONTIME          1200
#define FIRSTFUNCTIONTIME           70

#define MAX_GPIO                    4
#define PWR_PIN                     PB2
#define SET_PIN                     PC13
#define INC_PIN                     PB4
#define DEC_PIN                     PB5

const u8                            GPIO[MAX_GPIO]                     =          {PWR_PIN, SET_PIN, INC_PIN, DEC_PIN};
volatile u8                         GPIO_state[MAX_GPIO]               =          {0};
volatile u32                        GPIO_timer[MAX_GPIO]               =          {0};

#if USEINTERRUPT
void InterruptFunctionCall(void)
{
  for (int i = 0; i < MAX_GPIO; i++)
  {
    GPIO_state[i] = digitalRead(GPIO[i]);
    if (GPIO_timer[i] == 0 && GPIO_state[i])
      GPIO_timer[i] = millis();
  }
}
#endif


void FRIJ_GPIO_setup()
{
  for (int i = 0; i < MAX_GPIO; i++)
  {
    pinMode(GPIO[i], INPUT);
#if USEINTERRUPT
    attachInterrupt(digitalPinToInterrupt(GPIO[i]), InterruptFunctionCall, CHANGE);
#endif
  }
}

void FRIJ_GPIO_Loop()
{
  for (int i = 0; i < MAX_GPIO; i++)
  {
#if !USEINTERRUPT
    GPIO_state[i] = digitalRead(GPIO[i]);
#endif

    if (!GPIO_state[i] && GPIO_timer[i] != 0) // get button pressed and how long.
    {
#ifdef debug
      FRIJ.printf(F("pin %d: %ld\n"), i, (millis() - GPIO_timer[i]));
#endif
      if (i == 0 || !lock)
      {
        if ((millis() - GPIO_timer[i]) >= SECONDFUNCTIONTIME)
        {
          if(i == 1)//loop through compressor mode
          {
            if(compr_mode < 2) compr_mode++;
            else compr_mode = 0;
            clear_disp = true;
          }
          else if (i == 2) // show IMEI
          {
            memset(quick_msg, 0, sizeof(quick_msg));
            strcat(quick_msg, "IMEI:\n");
            strncat(quick_msg, IMEI, strlen(IMEI));
            strcat(quick_msg, "\n");
            display_stage = 1;
            clear_disp = true;
          }
          else if (i == 3)
          {
            memset(quick_msg, 0, sizeof(quick_msg));
            strcat(quick_msg, "ICCID:");
            strncat(quick_msg, ICCID, strlen(ICCID));
            strcat(quick_msg, "\n");            
            strncat(quick_msg, MCC, strlen(MCC));
            strncat(quick_msg, MNC, strlen(MNC));
            strcat(quick_msg, "\n");
            display_stage = 1;
            clear_disp = true;
          }
        }
        else if((millis() - GPIO_timer[i]) >= FIRSTFUNCTIONTIME) //ensure button is properly pressed
        {
          if (i == 0) //power button
          {
            if (powered && !force_shut_down) // turn off
            {
              powered = false;
              force_shut_down = true;
              EEPROM_Read = false;
            }
            else if (!powered && force_shut_down) // turn ON
            {
              powered = true;
              force_shut_down = false;
              display_stage = 0;
            }
          }
          else if (i == 1) //save set temp
          {
            u8 value[1];
            value[0] = (u8)set_box_temp;
            BoxTemperatureSetting( value, 1, 1);
          }
          else if (i == 2) //increment
          {
            if (set_box_temp < 126)
              set_box_temp ++;
            clear_disp = true;
          }
          else if (i == 3) //decrement
          {
            if (set_box_temp > -126)
              set_box_temp --;

            clear_disp = true;
          }
        }
      }
      GPIO_timer[i] = 0;
    }
  }
}
