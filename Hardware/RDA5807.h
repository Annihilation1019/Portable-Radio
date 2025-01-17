#ifndef __RDA5807_H__
#define __RDA5807_H__
#include "main.h"

uint8_t RDA5807_Reset();
uint8_t RDA5807_PowerOn();
uint8_t RDA5807_SetChannel(float channelDesired);
uint8_t RDA5807_isStation();
float RDA5807_ReadChannelVal();
uint8_t RDA5807_AutoSearch();

# endif