#pragma once
#include "bitmap.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/Org_01.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define MAX_DISP_NO   3
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

s16                                 disp_timer[]                  =         {5000, 40, -1};
u32                                 disp_switch_tmr               =         0;
bool                                not_drawn                     =         true;

void logo_display()
{
  //display.clearDisplay();
  display.drawBitmap(0, 0, gricdlogo, 128, 64, 1);
  display.display();

}

void charging_display()
{
  if (charg_stat == 1)
  {
    display.drawBitmap(0, 0, charging_image, 128, 64, 1);
    display.display();
  }
}

void home_display()
{

  s16 disp_offset = 0, shift = 0;
  if (box_temp >= 0)
    disp_offset = 13;
  else if (box_temp < 0 && box_temp > -10)
    disp_offset = 23;
    
  char bble[8] = "    ";
  if (charg_stat != 1)
    sprintf(bble, "%u%s", batLevel, "%");

  display.drawBitmap(105, 14, battery_empty, 15, 32, 1);
  if (lock)
  {
    shift = 13;
    display.drawBitmap(22, 0, Lock, 16, 16, 1);
  }
  display.drawBitmap(0, 0, network, 16, 16, 1);

  if (charg_stat == 1)
  {
    display.drawBitmap(107, 17, charge, 12, 26, 1);
  }
  else
  {
    display.fillRect(107, map(batLevel, 10, 100, 44, 18), 11, map(batLevel, 10, 100, 0, 26), SSD1306_WHITE);
  }

  display.setTextSize(1);                     // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(102, 1);                   // Start at top-left corner
  display.println(bble);
  display.setCursor((32 + shift), 0);
  if (net != -1)
    display.println(NETWORK_ID[net][0]);
  else
    display.println(F("       "));

  display.setCursor((84 - disp_offset), 18);
  display.println("C");
  display.setCursor(103, 57);
  if (compr_mode < 0)
    display.println(F("   "));
  else
    display.println(compressor_state[compr_mode]);
  display.setCursor((73 - disp_offset), 55);
  display.println(set_box_temp);

  sprintf(bble, "%02d", box_temp);
  display.setTextSize(1);                       // Normal 1:1 pixel scale
  display.setCursor(0, 55);
  display.setFont(&FreeSansBold24pt7b);
  display.println(bble);

  //display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setTextSize(1);                       // Normal 1:1 pixel scale
  display.setCursor(16, 5);
  display.setFont(&Org_01);
  if (gprs_Is_available)
    display.println('G');
  else
    display.println(' ');

  display.setFont(NULL);

  display.setTextSize(2);
  display.setCursor((72 - disp_offset), 18);
  display.cp437(true);
  display.write(248);

  display.display();
}

void message_display()
{
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(quick_msg);
  display.display();

}

void init_display()
{
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
#ifdef debug
    FRIJ.printf(F("SSD1306 allocation failed\n"));
#endif
    delay(1000);
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display_stage = 0;
}

void disp_timer_watch()
{
  if (millis() > disp_switch_tmr && disp_switch_tmr > 0 && !force_shut_down)
    display_stage++;
}

void control_display_flow()
{
  if (force_shut_down)
  {
    if (display_stage > 0)
    {
      display_stage = 0;
      disp_switch_tmr = millis() + disp_timer[display_stage];
      clear_disp = true;
      disp_timer[1] = 0;
    }
    else if (display_stage == 0 && millis() > disp_switch_tmr)
    {
      display_stage = -1;
      clear_disp = true;
    }
  }
  else
  {
    not_drawn = true;
    if (old_display_stage != display_stage)
    {
      if (disp_timer[display_stage] != -1)
        disp_switch_tmr = millis() + disp_timer[display_stage];
      else
      {
        disp_switch_tmr = 0;
        disp_timer[1] = 5000;
      }
      clear_disp = true;

      old_display_stage = display_stage;
    }
  }
}

void clearOLED()
{
  if (clear_disp)
  {
    display.clearDisplay();
    if (display_stage == -1)
      display.display();

    clear_disp = false;
  }
}

void display_process()
{
  clearOLED();
  /*
    if (display_stage == -1 )
    {

        #ifdef debug
        FRIJ.printf(F("sys not pwr: %d\n"), display_stage);
        #endif

      return;
    }
  */
  control_display_flow();

  switch (display_stage)
  {
    case -1:
      charging_display();
      break;
    case 0:
      logo_display();
      break;
    case 1:
      message_display();
      break;
    case 2:
      home_display();
      break;
  }

  disp_timer_watch();
}
