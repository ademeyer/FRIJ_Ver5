#pragma once

#define MAX_APN_DEFINED                     4

const char Glo[]                            = "gloflat";
const char MTN[]                            = "web.gprs.mtnnigeria.net";
const char Airtel[]                         = "internet.ng.airtel.com";
const char NineMobile[]                     = "9mobile";
const char Flolive[]                        = "flolive.net";
const char JTIOT[]                          = "JTM2M";

const char * const APN[]                    =
{
  MTN,
  Glo,
  Airtel,
  NineMobile,
  Flolive,
  JTIOT,
};

const char  NETWORK_ID[5][2][8]             =
{
  { {"MTN"},      {"62130"} },
  { {"GLO"},      {"62150"} },
  { {"AIRTEL"},   {"Econet"} },
  { {"9Mobile"},  {"62160"} },
  { {"UNKOWN"},   {""} },
};
