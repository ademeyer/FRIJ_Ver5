#pragma once
#include "GSM_String.h"
#if NEW_FRIJ
#define GSM_PWR         PB8
#define GSM_DELAY       10000   //10s
#else
#define GSM_DELAY       5000
#endif

void send_GSM_str(const char *str, u16 str_len, u32 wait, bool endTrans = false);

void GSM_Response_Handler(void)
{
  bool resp = false;
  switch (gsm_stage)
  {
    case _DETECT_GSM_SIM:  //detect_gsm_sim
      if (resp = GSM_str_is_valid(detect_gsm[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif
        //process of grabbing useful data
        if (stage_step == 2) //grab IMEI
        {
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, "\n", '\r', IMEI, sizeof(IMEI));
#if LOGLEVEL
          FRIJ.printf(F("IMEI: %s\n"), IMEI);
#endif
        }
        else if (stage_step == 3) //grab ICCID
        {
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, "\n", '\r', ICCID, sizeof(ICCID));
#if LOGLEVEL
          FRIJ.printf(F("ICCID: %s\n"), ICCID);
#endif
        }
        else if (stage_step == 8) //check if SIM is roaming
        {
          if (strstr(gsm_Raw_Buffer, "+CGREG: 0,5") != NULL) Isroaming = true;
        }
        else if (stage_step == 9)    //grab net ID
        {
          //if(net = id_network_name(gsm_Raw_Buffer) == -1) net = 4;
          net = id_network_name(gsm_Raw_Buffer);
          if (strlen(custom_apn) <= 0 && net > -1 && net < 4) apn_net = net;

        }
        else if (stage_step == 10 && Isroaming) //differentiate between Flolive and JT
        {
          if (strstr(gsm_Raw_Buffer, "FloLive") != NULL) apn_net = 4; //AT+CSPN?
          else apn_net = 5;
        }
      }
      break;

    case _INIT_GPRS: //init gprs connectivity
      if (resp = GSM_str_is_valid(init_gprs[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif

        //process of grabbing useful data
        if (stage_step == 0)    //grab CSQ
        {
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, "+CSQ: ", ',', CSQ, sizeof(CSQ));
#if LOGLEVEL
          FRIJ.printf(F("CSQ: %s\n"), CSQ);
#endif
        }
        else if (stage_step == 2)    //process jump of APN is already set
        {
          if (strstr(gsm_Raw_Buffer, APN[net]) != NULL)
            stage_jump = 1;
        }
        else if (stage_step == 5) //process grab assigned IP
        {
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, "\n", '\n', IP, sizeof(IP));
          gprs_Is_available = true;
#if LOGLEVEL
          FRIJ.printf(F("IP: %s\n"), IP);
#endif
        }
      }
      break;
    case _NTP_SERVER: // module graps GPRS time
      if (resp = GSM_str_is_valid(ntp_server[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 0)
        {
          if (gprs_time_found)
            stage_jump = 3;
          else
            stage_jump =  2;
        }
        else if (stage_step == 3)
        {
          if (strstr((char*)gsm_Raw_Buffer, "04/01/01") != NULL)
          {
#ifdef LOGLEVEL
            FRIJ.printf("We are unable to find an accurate date&time data to update RTC\r\n");
#endif
          }
          else
          {
            memset(ttimeBuf, 0, sizeof(ttimeBuf));
            tLen = gsm_data_grabber(gsm_Raw_Buffer, grb_len, "+CCLK: \"", '+', ttimeBuf, sizeof(ttimeBuf));
            gprs_time_found = true;
          }
        }
      }
      else
      {
        if (stage_step == 0) resp = true;
        else if (stage_step == 3)
        {
#ifdef debug
          FRIJ.printf("continue even if network time is not available\r\n");
#endif
          resp = true;
        }
      }
      break;

    case _INIT_TCP: //init tcp communication
      if (resp = GSM_str_is_valid(init_tcp[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif
        //process of grabbing useful data
        if (stage_step == 1)    //set tcp_alive true
        {
          tcp_alive = true;
          tcp_alive_timer = millis() + MAX_ALIVE_TIME;
          useUSB = false;
#if LOGLEVEL
          FRIJ.printf(F("tcp is opened!\n"));
#endif
        }
      }
      else
      {
        if (stage_step == 0)    //set tcp_alive true
        {
          if (strstr(gsm_Raw_Buffer, "ALREADY CONNECT") != NULL)
          {
            tcp_alive = true;
            tcp_alive_timer = millis() + MAX_ALIVE_TIME;
            Res_Len = 0;
#if LOGLEVEL
            FRIJ.printf(F("OK, tcp connection exists\n"));
#endif
            stage_jump = 1;
          }
          else
          {
            tcp_alive = false;
          }
          resp = true;
        }
      }
      break;

    case _NET_STAT: //get network status info
      if (resp = GSM_str_is_valid(net_stat[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 0)
        {
          char tem_bb[32] = "";
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, ",\"", ',', MCC, sizeof(MCC));
          strcat(tem_bb, MCC);
          strcat(tem_bb, ",");
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, tem_bb, ',', MNC, sizeof(MNC));
          memset(tem_bb, 0, sizeof(tem_bb));
          strcat(tem_bb, MNC);
          strcat(tem_bb, ",");
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, tem_bb, ',', LAC, sizeof(LAC));
          memset(tem_bb, 0, sizeof(tem_bb));
          strcat(tem_bb, LAC);
          strcat(tem_bb, ",");
          gsm_data_grabber(gsm_Raw_Buffer, grb_len, tem_bb, ',', CID, sizeof(CID));

#if LOGLEVEL
          FRIJ.printf(F("MCC: %s\n"), MCC);
          FRIJ.printf(F("MNC: %s\n"), MNC);
          FRIJ.printf(F("LAC: %s\n"), LAC);
          FRIJ.printf(F("CID: %s\n"), CID);

#endif
        }
      }
      else
      {
        if (stage_step == 0)
        {
          resp = true;
        }
      }
      break;
    case _SEND_DATA:    //send
      if (resp = GSM_str_is_valid(send_data[stage_step].resp_str))
      {
#ifdef debug
        FRIJ.printf(F("GSM response is valid for stage_step: %d:%d\n"), gsm_stage, stage_step);
#endif
        if (stage_step == 2)
        {
          tcp_alive_timer = millis() + MAX_ALIVE_TIME;
          dataAck = true;
          Res_Len = 0;
        }
      }
      else
      {
        if (stage_step == 0)
        {
#ifdef debug
          FRIJ.printf(F("we're unsure why this happened, but we need to configure GSM modem allover again\n"));
#endif
          stage_step = 0;
          gsm_stage = 0;
          gprs_Is_available = false;
        }
        else if(stage_step == 2)
        {
          Res_Len = 0; 
        }
      }
      break;

    default:
#ifdef debug
      FRIJ.printf(F("gsm_stage: %d\n"), gsm_stage);
#endif
      break;
  }

  if (resp)
  {
    if (stage_step < (GSM_MAX_STAGE_STEP[gsm_stage] - 1))
    {
      stage_step = stage_step + stage_jump + 1;
      stage_jump = 0;
    }
    else if (stage_step >= (GSM_MAX_STAGE_STEP[gsm_stage] - 1))
    {
      stage_step = 0;
      if (gsm_stage < MAX_GSM_STAGE - 1) //prevent process from going to the last stage, which is 'close net'
        gsm_stage++;
    }
  }
}

void GSM_Send_Handler()
{
  if (tmr_not_expired(millis(), gsm_response_tmr) || gsm_stage >= (MAX_GSM_STAGE - 1) || !powered)return;

  switch (gsm_stage)
  {
    case _DETECT_GSM_SIM:
      if (!Isroaming && stage_step == 10)
      {
        gsm_stage++; // if time is already, move to the next gms stage
        stage_step = 0;
      }
      else
      {
#ifdef debug
        FRIJ.printf(F("query: %s, wait resp: %ld\n"), detect_gsm[stage_step].query_str, detect_gsm[stage_step].wait_tmr);
#endif
        send_GSM_str(detect_gsm[stage_step].query_str, strlen(detect_gsm[stage_step].query_str), detect_gsm[stage_step].wait_tmr);
      }
      break;

    case _INIT_GPRS:
      if (apn_net == -1) return;        //if sim apn is not detected, no reason to run this at all
#ifdef debug
      FRIJ.printf(F("query: %s, wait resp: %ld\n"), init_gprs[stage_step].query_str, init_gprs[stage_step].wait_tmr);
#endif
      if (stage_step == 3)
      {
        char temp_str[MAX_LEN] = "";
        if (strlen(custom_apn) <= 0)
          putStringInArray(init_gprs[stage_step].query_str, "{0}", APN[apn_net], temp_str, sizeof(temp_str));
        else
          putStringInArray(init_gprs[stage_step].query_str, "{0}", custom_apn, temp_str, sizeof(temp_str));
        send_GSM_str(temp_str, strlen(temp_str), init_gprs[stage_step].wait_tmr);
      }
      else
        send_GSM_str(init_gprs[stage_step].query_str, strlen(init_gprs[stage_step].query_str), init_gprs[stage_step].wait_tmr);
      break;

    case _NTP_SERVER:
#ifdef debug
      FRIJ.printf(F("query: %s, wait resp: %ld\n"), ntp_server[stage_step].query_str, ntp_server[stage_step].wait_tmr);
#endif
      if (gprs_time_found)
      {
        gsm_stage++; // if time is already, move to the next gms stage
        stage_step = 0;
      }
      else
        send_GSM_str(ntp_server[stage_step].query_str, strlen(ntp_server[stage_step].query_str), ntp_server[stage_step].wait_tmr);
      break;

    case _INIT_TCP:
      if (!gprs_Is_available) gsm_stage = gsm_stage - 2; //if internet is not active, no reason to run this at all, rather move a step backward and try init gprs
      else
      {
#ifdef debug
        FRIJ.printf(F("query: %s, wait resp: %ld\n"), init_tcp[stage_step].query_str, init_tcp[stage_step].wait_tmr);
#endif
        if (stage_step == 0) //insert detected APN
        {
          char temp_str1[MAX_LEN] = "", temp_str2[MAX_LEN] = "";
          putStringInArray(init_tcp[stage_step].query_str, "{0}", server, temp_str1, sizeof(temp_str1));
          putStringInArray(temp_str1, "{1}", host, temp_str2, sizeof(temp_str2));
          send_GSM_str(temp_str2, strlen(temp_str2), init_tcp[stage_step].wait_tmr);
        }
        else
          send_GSM_str(init_tcp[stage_step].query_str, strlen(init_tcp[stage_step].query_str), init_tcp[stage_step].wait_tmr);
      }
      break;

    case _NET_STAT:
#ifdef debug
      FRIJ.printf(F("query: %s, wait resp: %ld\n"), net_stat[stage_step].query_str, net_stat[stage_step].wait_tmr);
#endif
      send_GSM_str(net_stat[stage_step].query_str, strlen(net_stat[stage_step].query_str), net_stat[stage_step].wait_tmr);
      break;

    case _SEND_DATA:
#ifdef debug
      FRIJ.printf(F("query: %s, wait resp: %ld\n"), send_data[stage_step].query_str, send_data[stage_step].wait_tmr);
#endif
      if (Res_Len <= 0)
      {
#ifdef debug
        FRIJ.println(F("There is no data to send"));
#endif
        gsm_stage++;        //if there is nothing to send, just move on
        stage_step = 0;
      }
      else
      {
        if (stage_step == 1) //insert data to be sent
        {

          char temp_str[MAX_LEN] = "";
          putStringInArray(send_data[stage_step].query_str, "{0}", RES_BUFF, temp_str, sizeof(temp_str));
          send_GSM_str(temp_str, strlen(temp_str), send_data[stage_step].wait_tmr, true);
          stage_step++;
#ifdef debug
          FRIJ.printf(F("\r\ndatalen: %d\r\n"), Res_Len);
          FRIJ.write((u8*)temp_str, strlen(temp_str));
          FRIJ.println();
#endif
        }
        else
          send_GSM_str(send_data[stage_step].query_str, strlen(send_data[stage_step].query_str), send_data[stage_step].wait_tmr);
      }
      break;

    case _CLOSE_NET:
#ifdef debug
      FRIJ.printf(F("query: %s, wait resp: %ld\n"), close_net[stage_step].query_str, close_net[stage_step].wait_tmr);
#endif
      send_GSM_str(close_net[stage_step].query_str, strlen(close_net[stage_step].query_str), close_net[stage_step].wait_tmr);
      break;


    default:
#ifdef debug
      FRIJ.printf(F("unknown gsm_stage: %d\n"), gsm_stage);
#endif
      break;
  }
}


void GSM_Process(void)
{
  if (tmr_not_expired(millis(), gsm_operation_delay) || !powered)
  {
#ifdef debug
    FRIJ.printf(F("."));
#endif
    return;
  }

  GSM_Send_Handler();

  if ((grb_len = rec_GSM_str()) > 0)
  {
#ifdef debug
    FRIJ.printf(F("\ngsm len: %d\n"), grb_len);
    FRIJ.write((u8*)gsm_Raw_Buffer, grb_len);
    FRIJ.println();
#endif

    GSM_Response_Handler();
    if (inputLen == 0)
      inputLen = process_GSM_Frames(gsm_Raw_Buffer, grb_len, userInput, sizeof(userInput));
    memset(gsm_Raw_Buffer, 0, sizeof(gsm_Raw_Buffer));
    grb_len = 0;
  }
  connectivity_watchdog();
  //routine_id_network_name();
}

void setUp_GSM(void)
{
#if NEW_FRIJ
  pinMode(GSM_PWR, OUTPUT);
  digitalWrite(GSM_PWR, HIGH);
#endif
  GSM.begin(BAUD_RATE);
  GSM.setTimeout(50);
  init_GSM_str();
  gsm_operation_delay = millis() + GSM_DELAY;
  tcp_alive_timer = millis() + MAX_ALIVE_TIME + 120000;

}
