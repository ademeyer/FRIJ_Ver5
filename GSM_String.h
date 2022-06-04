#pragma once
#include "Flash_Data.h"

#define GSM                         Serial3
#define BAUD_RATE                   115200
#define USE_GLOBAL_SIM              0
#define MAX_GSM_STAGE               7
#define MAX_ALIVE_TIME              (3*MAX_BEAT_TIME)
//Action maximum process number
#define DETECT_GSM_NO               11
#define INIT_GPRS_NO                6
#define NTP_SERVER_NO               4
#define INIT_TCP_NO                 2
#define NET_STAT_NO                 1
#define SEND_DATA_NO                3
#define CLOSE_NET_NO                1

enum                                _GSM_PROCESS                            {_DETECT_GSM_SIM, _INIT_GPRS, _NTP_SERVER, _INIT_TCP, _NET_STAT, _SEND_DATA, _CLOSE_NET};
const u16                           GSM_MAX_STAGE_STEP[]          =         {DETECT_GSM_NO, INIT_GPRS_NO, NTP_SERVER_NO, INIT_TCP_NO, NET_STAT_NO, SEND_DATA_NO, SEND_DATA_NO, CLOSE_NET_NO};
char                                gsm_Raw_Buffer[MAX_LEN]       =         "";
s8                                  net                           =         -1;
s8                                  apn_net                       =         -1;
u8                                  stage_step                    =         0;
u8                                  stage_jump                    =         0;
u16                                 grb_len                       =         0;
u32                                 gsm_response_tmr              =         0;
u32                                 gsm_operation_delay           =         0;
u32                                 tcp_alive_timer               =         0;
u32                                 routine_id                    =         60000;


typedef struct
{
  const char *query_str;
  const char *resp_str;
  u32 wait_tmr;
} gsm_parameter;


gsm_parameter detect_gsm    [DETECT_GSM_NO];
gsm_parameter init_gprs     [INIT_GPRS_NO];
gsm_parameter ntp_server    [NTP_SERVER_NO];
gsm_parameter init_tcp      [INIT_TCP_NO];
gsm_parameter net_stat      [NET_STAT_NO];
gsm_parameter send_data     [SEND_DATA_NO];
gsm_parameter close_net     [CLOSE_NET_NO];



//mcc, mnc, lac and cellid
void init_GSM_str()
{
  //0 = creating the detect gsm string
  detect_gsm[0] = {"AT", "OK", 200};                                    //0
  detect_gsm[1] = {"AT+CFUN= 1,1", "OK", 25000};                        //1
  detect_gsm[2] = {"AT+GSN", "OK", 500};                                //2 = grab IMEI
  detect_gsm[3] = {"AT+CCID", "OK", 2000};                              //3 = grab ICCID
  detect_gsm[4] = {"AT+CENG=3", "OK", 500};                             //4
  detect_gsm[5] = {"ATE0", "OK", 500};                                  //5
  detect_gsm[6] = {"AT+CNMI=2,2,0,0,0", "OK", 500};                     //6
  detect_gsm[7] = {"AT+CMGF=1", "OK", 500};                             //7
  detect_gsm[8] = {"AT+CGREG?", "OK", 1000};                            //8 = process roaming network affirmation
  detect_gsm[9] = {"AT+COPS?", "+COPS: 0,0,", 1000};                    //9 = process APN ID
  detect_gsm[10] = {"AT+CSPN?", "OK", 1000};                            //10 = reaffirm a roaming network, re-process a apn

  //1 = creating the init gprs string
  init_gprs[0] = {"AT+CSQ", "OK", 200};                                 //0 = grab network signal strength
  init_gprs[1] = {"AT+CGATT=1", "OK", 500};                             //1
  init_gprs[2] = {"AT+CSTT?", "OK", 500};                               //2 = process response for possible jump of APN settings
  init_gprs[3] = {"AT+CSTT = \"{0}\", \"\", \"\"", "OK", 500};          //3 = process sending for inserting APN settings
  init_gprs[4] = {"AT+CIICR", "OK", 3000};                              //4 = bring up gprs
  init_gprs[5] = {"AT+CIFSR", ".", 1500};                               //5 = grab assigned network IP

  //2 = creating the ntp server string
  ntp_server[0] = {"AT+CLTS?", "+CLTS: 1", 800};                        //0 = check if get network time is set, and if yes, process two stage jump
  ntp_server[1] = {"AT+CLTS=1", "OK", 500};                             //1
  ntp_server[2] = {"AT&W", "OK", 1000};                                 //2 = process for gsm failed restart
  ntp_server[3] = {"AT+CCLK?", "+CCLK: ", 1000};                        //3 = process time stamp

  //3 = creating the init tcp string
  init_tcp[0] = {"AT+CIPSTART=\"TCP\",\"{0}\",\"{1}\"", "OK", 10000};   //0 = process sending for inserting server details
  init_tcp[1] = {"AT+CIPSTATUS", "CONNECT OK", 500};                    //1 = process connected status

  //4 = creating the net stat string
  net_stat[0] = {"AT+CENG?", "+CENG:", 1000};                           //0 = process response to grab LAC, CELLID, MCC and MNC

  //5 = creating the send data string
  send_data[0] = {"AT+CIPSEND", ">", 1000};                             //0
  send_data[1] = {"{0}", "", 500};                                      //1 = process sending stage to insert message format
  send_data[2] = {"", "SEND OK", 2000};                                 //2 = process response for successful sent data

  //6 = creating the close net string
  close_net[0] = {"AT+CIPSHUT", "OK", 500};                                //0
  //close_net[1] = {"AT+CGATT=0", "OK", 100};                              //1

}

s8 id_network_name(const char* mccmnc)
{
  s8 net_id = -1;
  for (int i = 0; i < MAX_APN_DEFINED; i++)
  {
    if (strstr(mccmnc, NETWORK_ID[i][0]) != NULL || strstr(mccmnc, NETWORK_ID[i][1]) != NULL)
    {
      net_id = i;
      break;
    }
    else
      net_id = -1;
  }

  return net_id;
}

void routine_id_network_name()
{
  if (strlen(MNC) <= 0 || strlen(MCC) <= 0 || millis() <= routine_id) return;

  char mmcmcc[20] = "";
  strcat(mmcmcc, MCC);
  strcat(mmcmcc, MNC);
  net = id_network_name(mmcmcc);
  routine_id = millis() + 25000;
}

void send_GSM_str(const char *str, u16 str_len, u32 wait, bool endTrans)
{
#ifdef debug
  FRIJ.printf(("%d:%d->len %d and response within %d secs\r\n"), gsm_stage, stage_step, str_len, wait);
#endif
  if (str_len > 0)        //this is needed to maintain compatibility with when controller is only expecting server/computer
  {
    GSM.write(str, str_len);
    GSM.write("\r\n", 2);
    if (endTrans)
      GSM.write(0x1A);
    GSM.flush();
  }
  gsm_response_tmr = millis() + wait;
}

u16 rec_GSM_str()
{
  /*
    u16 p = 0;
    if (GSM.available() > 0)
    {
    while (GSM.available() > 0)
    {
      char c = GSM.read();
      if (p < sizeof(gsm_Raw_Buffer))
        gsm_Raw_Buffer[p++] = c;
      delayMicroseconds(100);
    }
    return p;
    }
  */

  if (GSM.available() > 0)
  {
    return (GSM.readBytes(gsm_Raw_Buffer, sizeof(gsm_Raw_Buffer)));
  }
  return 0;
}

bool GSM_str_is_valid(const char *comp_str)
{
  if (strstr(gsm_Raw_Buffer, comp_str) != NULL)
    return true;

#ifdef debug
  FRIJ.printf(F("\nGSM str byte %d is not valid\n"), grb_len);
#endif
  return false;
}

void network_failed_restart()
{
  gsm_stage = 0;
  stage_step = 0;
  tcp_alive = false;
  gprs_Is_available = false;
}

u16 process_GSM_Frames(const char *str, u16 str_len, char* resp, u16 respLen)
{
  if (str_len <= 0) return 0;
  u16 rLen = 0;
  if ((strstr(str, "+PDP DEACT") != NULL) || (strstr(str, "CLOSED") != NULL))
  {
#ifdef debug
    FRIJ.println(F("TCP closed"));
#endif
    network_failed_restart();
  }
 /* else if (strstr(str, "*PSUTTZ") != NULL)
  {
    memset(ttimeBuf, 0, sizeof(ttimeBuf));
    tLen = gsm_data_grabber(gsm_Raw_Buffer, grb_len, "*PSUTTZ: ", '+', ttimeBuf, sizeof(ttimeBuf));
    gprs_time_found = true;
  }*/
  else if ((strstr(str, server_start_str) != NULL) && (strstr(str, end_str) != NULL) && (strstr(str, IMEI) != NULL))
  {
    beatTimer = millis();
#ifdef debug
    FRIJ.println(F("Valid server data!"));
#endif
    rLen = gsm_data_grabber(str, str_len, server_start_str, '*', resp, respLen);
    operate = true;
  }

  return rLen;
}

void connectivity_watchdog()
{
  if (millis() > tcp_alive_timer)
  {
#ifdef debug
    FRIJ.printf(F("well, its over %dmin and no successful heart beat\n"), MAX_ALIVE_TIME / 60000);
#endif
    if (tcp_alive) tcp_alive = false;
    if (gsm_stage >= _INIT_TCP)
    {
      network_failed_restart();
    }

    tcp_alive_timer = millis() + MAX_ALIVE_TIME;
  }
}
