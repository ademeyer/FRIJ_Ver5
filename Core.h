#pragma once

#define  NEW_FRIJ                   0
//#define debug

#if NEW_FRIJ
#define FRIJ                        Serial
#else
#define FRIJ                        Serial2
#endif

#define LOGLEVEL                    0


#define tmr_not_expired(x, y)       x < y ? 1 : 0
#define MAX_Para                    11
#define MAX_LEN                     256
#define endMsg                      '\r'
#define progVersion                 "FRIJ_Ver 3.1"
#define MAX_BEAT_TIME               60000 // 1min

//datatypes
typedef uint8_t                     u8;
typedef uint16_t                    u16;
typedef uint32_t                    u32;
typedef int8_t                      s8;
typedef int16_t                     s16;
typedef int32_t                     s32;

//FRIJ Parameter Definition
const char                          *op_command[]                 =         {"001", "002", "003", "004", "005", "006", "007", "008", "009", "010", "011", "081", "082", "083", "144"};

const u8                            RE_No                         =         MAX_Para;
const u8                            PD_No                         =         MAX_Para + 1;
const u8                            HB_No                         =         MAX_Para + 2;

//message headers
const char*                         server_start_str              =         "$$,";
const char*                         start_str                     =         "##,";
const char*                         end_str                       =         "*\r";
u32                                 saveIndex                     =         0;
u32                                 readIndex                     =         0;
s32                                 system_time                   =         0;
u32                                 beatTimer                     =         0;
u32                                 pdTimer                       =         0;
u16                                 inputLen                      =         0;
u8                                  batLevel                      =         0;
u16                                 BUFF_Len                      =         0;
u16                                 Res_Len                       =         0;
u8                                  gsm_stage                     =         0;
u8                                  charg_stat                    =         0;
s8                                  box_temp                      =         0;
s8                                  set_box_temp                  =         0;
s8                                  op                            =         -1;
u8                                  tLen                          =         0;
u8                                  pdTime                        =         2;
bool                                mode                          =         0;


char                                userInput[MAX_LEN]            =         "";
char                                BUFF[MAX_LEN]                 =         "";
char                                RES_BUFF[MAX_LEN]             =         "";
char                                G_Lat[16]                     =         "";
char                                G_Long[16]                    =         "";
char                                MCC[8]                        =         "";
char                                MNC[8]                        =         "";
char                                LAC[8]                        =         "";
char                                CID[8]                        =         "";
char                                IMEI[32]                      =         "";
char                                ICCID[32]                     =         "";
char                                CSQ[8]                        =         "";
char                                IP[32]                        =         "";
char                                GRD[32]                       =         "GRD000000";
char                                server[]                      =         "143.110.236.10"; //"104.131.53.102"
char                                host[]                        =         "8001"; //"2000"
char                                custom_apn[32]                =         "";
char                                ttimeBuf[32]                  =         "";

bool                                valid_GPS                     =         false;
bool                                useUSB                        =         true;
bool                                operate                       =         false;
bool                                dataAck                       =         false;
bool                                force_shut_down               =         true;
bool                                gprs_Is_available             =         false;
bool                                powered                       =         false;
bool                                gprs_time_found               =         false;
bool                                frij_time_synchronized        =         false;
bool                                time_set                      =         false;
bool                                lock                          =         false;
bool                                tcp_alive                     =         false;
bool                                EEPROM_Read                   =         false;
bool                                Isroaming                     =         false;


//HVAC globals
s8                                  compr_mode                    =         1;
const char                          *_Compressor_mode[]           =         {"OFF", "MID", "ECO", "MAX"};


//Display globals
char                                quick_msg[72]                 =         "";
s8                                  display_stage                 =         -1;
s8                                  old_display_stage             =         -1;
bool                                clear_disp                    =         false;
