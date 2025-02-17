#include "RDA5807.h"
#include "i2c.h"
#include <math.h>

#define ERR 1
#define OK 0

/* 变量定义 */
uint16_t readAddress = 0x23;  // 从机读地址
uint16_t writeAddress = 0x22; // 从机写地址

/* 状态位 */
uint8_t status;

/* 频道缓存和总计数 */
float channelBuf[25];
uint8_t channelTotal = 0;

/**
 * @brief  读取 RDA5807 寄存器
 * @param  reg: 寄存器地址
 * @param  data: 读取数据存放地址
 * @param  size: 数据长度
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_ReadRegister(uint8_t reg, uint8_t *data, uint8_t size)
{
    status = HAL_I2C_Mem_Read(&hi2c1, readAddress, reg, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        return ERR;
    }
    return OK;
}

/**
 * @brief  写入 RDA5807 寄存器
 * @param  reg: 寄存器地址
 * @param  data: 写入数据存放地址
 * @param  size: 数据长度
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_WriteRegister(uint8_t reg, uint8_t *data, uint8_t size)
{
    status = HAL_I2C_Mem_Write(&hi2c1, writeAddress, reg, I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
    if (status != HAL_OK)
    {
        return ERR;
    }
    return OK;
}

/**
 * @brief  RDA5807 复位
 * @note   向寄存器地址 0x02 写入 0x0002
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_Reset()
{
    uint8_t resetData[2] = {0x00, 0x02}; // 16 位数据，分为两个字节
    status = RDA5807_WriteRegister(0x02, resetData, 2);
    if (status != OK)
    {
        return status;
    }
    HAL_Delay(50);
    return status;
}

/**
 * @brief  RDA5807 上电
 * @note   向寄存器地址 0x02 写入 0xC001
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_PowerOn()
{
    uint8_t powerOnData[2] = {0xC0, 0x01}; // 16 位数据，分为两个字节
    status = RDA5807_WriteRegister(0x02, powerOnData, 2);
    if (status != OK)
    {
        return status;
    }
    HAL_Delay(600);
    return status;
}

/**
 * @brief  RDA5807 设置频道
 * @param  channelDesired: 跳转的频道频率（87 - 108 MHz）
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_SetChannel(float channelDesired)
{
    uint8_t searchChannelData[2] = {0x11, 0x90};
    // 计算频道值改变量
    uint16_t channel_add = (uint16_t)round(((channelDesired - 87.0f) * 10.0f));
    // 写入高10位
    searchChannelData[0] = channel_add >> 2;
    searchChannelData[1] = (channel_add & 0x03) << 6;
    // 写入低6位：开启搜索，频率范围 87 - 108 MHz，搜索步进 100 KHz
    searchChannelData[1] |= 0x10;
    status = RDA5807_WriteRegister(0x03, searchChannelData, 2);
    if (status != OK)
    {
        return status;
    }
    HAL_Delay(100);
    return status;
}

/**
 * @brief  判断是否为 FM 电台
 * @retval 1: 是电台 0: 不是电台
 */
uint8_t RDA5807_isStation()
{
    uint8_t read_buf[1];
    // 读取寄存器 0x0b 的高8位数据
    RDA5807_ReadRegister(0x0b, read_buf, 1);
    // 检查 FM TRUE 位
    if (read_buf[0] & 0x01) // FM TRUE位 为 1
    {
        return 1;
    }
    return 0;
}

/**
 * @brief  读取当前频道原始值
 * @retval 当前频道值
 * @note   寄存器 0x0a 的低10位数据为原始数据
 */
float RDA5807_ReadChannelVal()
{
    uint8_t read_buf[2];
    // 读取寄存器 0x0a 数据
    RDA5807_ReadRegister(0x0a, read_buf, 2);
    // 计算频道值，低10位为原始数据
    uint16_t channel_raw = ((uint16_t)(read_buf[0] & 0x03) << 8) | (uint16_t)(read_buf[1]);

    return (channel_raw + 870) / 10.0;
}

/**
 * @brief  RDA5807 自动搜索电台
 * @note   从 87.0 MHz 开始，每隔 0.1 MHz 搜索一次，直到 108.0 MHz
 * @retval 搜索到的电台数量
 */
uint8_t RDA5807_AutoSearch()
{
    double Channel = 87.0f;
    channelTotal = 0;
    while (1)
    {
        RDA5807_SetChannel(Channel);
        HAL_Delay(50);
        if (RDA5807_isStation())
        {
            // 保存当前频道
            channelBuf[channelTotal] = Channel;
            channelTotal++;
        }
        Channel += 0.1;
        if (Channel > 108.0f)
        {
            return channelTotal;
        }
    }
}

/**
 * @brief  RDA5807 设置音量
 * @param  volume: 音量值（0 - 15）
 * @retval OK: 成功 ERR: 失败
 */
uint8_t RDA5807_SetVolume(uint8_t volume)
{
    uint8_t volumeData[2];
    status = RDA5807_ReadRegister(0x05, volumeData, 2);
    if (status != OK)
    {
        return status;
    }
    volumeData[1] = (volumeData[1] & 0xF0) | volume;
    status = RDA5807_WriteRegister(0x05, volumeData, 2);
    return status;
}

/**
 * @brief  RDA5807 换台
 * @param  direction: 换台方向（0: 向下 1: 向上）
 * @retval 当前频道值
 */
float RDA5807_ChangeStation(uint8_t direction)
{
    static uint8_t channelIndex = 0; // 当前频道索引
    if (direction == 0)
    {
        if (channelIndex == 0)
        {
            channelIndex = channelTotal - 1;
        }
        else
        {
            channelIndex--;
        }
    }
    else if (direction == 1)
    {
        if (channelIndex == channelTotal - 1)
        {
            channelIndex = 0;
        }
        else
        {
            channelIndex++;
        }
    }
    RDA5807_SetChannel(channelBuf[channelIndex]);
    return channelBuf[channelIndex];
}