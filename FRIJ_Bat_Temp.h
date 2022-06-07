#pragma once

#define USEADCINTERRUPT             1
#if USEADCINTERRUPT
#include <STM32ADC.h>
#endif
#define MAX_ADC                     2
#define BATT_LEVEL_PIN              PB1
#define TEMP_ADC_PIN                PB0
#define adc_sampling_rate           20000
#define BAT_MAX                     12.6
#define BAT_MIN                     9.5
#define VRWT                        4000


u32                                 sampleTimer                   =         0;
u32                                 batTimer                      =         0;
u8                                  adc_pin[]                     =         {BATT_LEVEL_PIN, TEMP_ADC_PIN};
u16                                 adc_buffer[MAX_ADC]           =         {0};
s8                                  old_compr                     =         compr_mode;
#if USEADCINTERRUPT
STM32ADC myADC(ADC1);
#define MAXSAMPLE                   10000
u16                                 dataPoints[MAX_ADC]           =         {0};
u32                                 old_adcBuffer[MAX_ADC]        =         {0};
u32                                 cnt                           =         0;

void ADC_counted(void)
{
  //FRIJ.println("done");
  if (cnt < MAXSAMPLE)
  {
    for (int i = 0; i < MAX_ADC; i++)
    {
      old_adcBuffer[i] += dataPoints[i];
    }
    cnt++;
  }
}
#else
u16 sampleADC(const u8 pin, u8 samples)
{
  u32 val = 0;
  for (int j = 0; j < samples; j++)
  {
    val += analogRead(pin);
    delayMicroseconds(10);
  }
  return u16(val / samples);
}
#endif

void FRIJ_Bat_Temp_Setup(void)
{

  for (int i = 0; i < MAX_ADC; i++)
    pinMode(adc_pin[i], INPUT_ANALOG);
#if USEADCINTERRUPT
  myADC.calibrate();
  myADC.setSampleRate(ADC_SMPR_55_5);//set the Sample Rate
  myADC.setScanMode();              //set the ADC in Scan mode.
  myADC.setPins(adc_pin, MAX_ADC);           //set how many and which pins to convert.
  myADC.setContinuous();            //set the ADC in continuous mode.
  myADC.setDMA(dataPoints, MAX_ADC, (DMA_MINC_MODE | DMA_CIRC_MODE | DMA_TRNS_CMPLT), ADC_counted);
  myADC.startConversion();
#endif
}

void FRIJ_Bat_Temp_Loop(void)
{
#if !USEADCINTERRUPT
  for (int i = 0; i < MAX_ADC; i++)
  {
    adc_buffer[i] = sampleADC(adc_pin[i], 250);
    /*#ifdef debug
          FRIJ.printf(F("%d: adc_buffer[i]\n"), i, adc_buffer[i]);
      #endif*/
  }
#else
  if (cnt >= MAXSAMPLE)
  {
    for (int i = 0; i < MAX_ADC; i++)
    {
      old_adcBuffer[i] = old_adcBuffer[i] / cnt;
      adc_buffer[i] = old_adcBuffer[i];
      old_adcBuffer[i] = 0;
      /*#ifdef debug
              FRIJ.printf(F("%d: %d\n"), i, adc_buffer[i]);
  #endif*/
      }
      cnt = 0;
      }
#endif
  s16 tp = 0;
  float v = 12.33 * adc_buffer[0] / 2786.0;
  tp = mapfloat(v, BAT_MIN, BAT_MAX, 0.0, 100.0);

  if (tp > batLevel && batLevel != 0) //changes capturing
  {
    if (batTimer == 0)
    {
#ifdef debug
      FRIJ.printf(F("starting bat level rise rate capture\n"));
#endif
      batTimer = millis() + VRWT;
    }
  }
  //check if it is charging or not
  if ((millis() >= batTimer && batTimer != 0 && tp > batLevel) || (old_compr != compr_mode && compr_mode != -1))
  {
    charg_stat = 1;
#ifdef debug
    FRIJ.printf(F("seems it's charging\n"));
#endif
    batTimer = millis() + VRWT;
    if (old_compr != compr_mode)
      old_compr = compr_mode;
  }
  else if (millis() >= batTimer && batTimer != 0 && tp <= batLevel)
  {
    charg_stat = 0;
    batTimer = 0;
#ifdef debug
    FRIJ.printf(F("seems charging stopped\n"));
#endif
    clear_disp = true;
  }

  //update battery level
  if (tp != batLevel)
  {
    if (tp < 0) batLevel = 0;
    else if (tp > 100) batLevel = 100;
    else batLevel = tp;
    if (powered)
      clear_disp = true;
  }

  /*
    if (millis() > batTimer)
    {
    float v = 12.33 * adc_buffer[0] / 2786.0;
    s16 tp = mapfloat(v, BAT_MIN, BAT_MAX, 0.0, 100.0);// map(adc_buffer[0], 2250, 2650, 10, 100);
    if (tp != batLevel)
    {
      charg_stat = 0;
      if (tp > 100)
      {
        charg_stat = 1;
        batLevel = 100;
      }
      else if (tp < 10) batLevel = 0;
      else
      {
        batLevel = tp;
      }
      clear_disp = true;
    }

    batTimer = millis() + 1000;
    }
  */

  if (millis() >= sampleTimer)
  {
    tp = convertTemp(adc_buffer[1]);
    if (tp != box_temp)
    {
      box_temp = tp;
      clear_disp = true;
    }
    sampleTimer = millis() + adc_sampling_rate;
  }
}
