#pragma once
#include "systemConfig.hpp"
/**
 * WiFi 模组指令处理
 */
/* 系统配置类 */
extern SystemConfig systemConfig;
extern "C"
{
#include "main.h"
#include "bsp/bsp_device_usart.h"
#include "bsp/bsp_wifi_usart.h"

    extern uint8_t ucWifiRecvBuffer[];
    extern uint16_t ulWifiRecvSize;
    extern uint8_t ucWifiRecvReady;
}

class WiFiOrder
{
public:
    void coreProcess()
    {
        if (ucWifiRecvReady == TRUE)
        {
            if (systemConfig.getBoolWiFiOutEcho())
            {
                printf(">> WiFi 输出: ");
            }
            for (int i = 0; i < ulWifiRecvSize; ++i)
            {
                if (systemConfig.getBoolWiFiOutEcho())
                {
                    printf("%02x ", ucWifiRecvBuffer[i]);
                }
                HAL_UART_Transmit(&UartDeviceHandle, (uint8_t *)(ucWifiRecvBuffer + i), 1, 1000);
            }
            if (systemConfig.getBoolWiFiOutEcho())
            {
                printf("\n");
            }
            captureOrder(ucWifiRecvBuffer, ulWifiRecvSize);
            ucWifiRecvReady = FALSE;
        }
    }
    void resetWiFi()
    {
        unsigned char buffer[] = {0xAA, 0x0A, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x83, 0x98};
        for (uint16_t i = 0; i < sizeof(buffer); ++i)
        {
            HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)(buffer + i), 1, 1000);
        }
        cout << ">> 重置 WiFi 模块指令发送成功." << endl;
    }

    void queryDate()
    {
        unsigned char buffer[] = {0xAA, 0x0B, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x00, 0xB9};
        for (uint16_t i = 0; i < sizeof(buffer); ++i)
        {
            HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)(buffer + i), 1, 1000);
        }
        cout << ">> WiFi 模块获取时间指令发送成功." << endl;
    }
    void queryMac()
    {
        unsigned char buffer[] = {0xAA, 0x0B, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x00, 0x07};
        for (uint16_t i = 0; i < sizeof(buffer); ++i)
        {
            HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)(buffer + i), 1, 1000);
        }
        cout << ">> WiFi 模块获取 Mac 指令发送成功." << endl;
    }
    void queryWiFiVersion()
    {
        unsigned char buffer[] = {0xAA, 0x0B, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87, 0x00, 0x93};
        for (uint16_t i = 0; i < sizeof(buffer); ++i)
        {
            HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)(buffer + i), 1, 1000);
        }
        cout << ">> WiFi 模块获取版本指令发送成功." << endl;
    }
    void queryNetState()
    {
        unsigned char buffer[] = {0xAA, 0x0B, 0xDB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0x00, 0xB7};
        for (uint16_t i = 0; i < sizeof(buffer); ++i)
        {
            HAL_UART_Transmit(&UartWiFiHandle, (uint8_t *)(buffer + i), 1, 1000);
        }
        cout << ">> WiFi 模块获取网络指令发送成功." << endl;
    }

private:
    void processDate(uint8_t *buffer, int len)
    {
        string weekdayArr[] = {
            "星期日",
            "星期一",
            "星期二",
            "星期三",
            "星期四",
            "星期五",
            "星期六"};
        cout << ">> WiFi 模块时间： 20" << (int)buffer[16] << "-" << (int)buffer[15] << "-" << (int)buffer[14] << " " << (int)buffer[12] << ":" << (int)buffer[11] << ":" << (int)buffer[10] << " " << ((int)buffer[13] > 6 ? " " : weekdayArr[buffer[13]]) << endl;
    }
    void processWiFiVersion(uint8_t *buffer, int len)
    {
        printf(">> WiFi Version: %02x%02x%02x%02x%02x%02x%02x.\n", buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15], buffer[16]);
    }
    void processMac(uint8_t *buffer, int len)
    {
        printf(">> WiFi Mac: %2x:%2x:%2x:%2x:%2x:%2x.\n", buffer[15], buffer[14], buffer[13], buffer[12], buffer[11], buffer[10]);
    }
    void processNetstate(uint8_t *buffer, int len)
    {
        string signalStrengths[] = {
            "无信号",
            "1 档（弱）",
            "2 档",
            "3 档",
            "4 档",
        };
        string routeStates[] = {
            "已经连上路由器",
            "未连接路由器",
            "正在连接路由",
            "密码验证错误",
            "未找到无线路由",
            "未获取到 IP",
            "无线不稳定",
        };
        string mideaSates[] = {
            "已经连上美的云",
            "未连接美的云",
            "互联网连接不稳定",
            "域名解析错误",
            "云服务连接拒绝",
            "云服务维护中",
        };
        cout << ">> WiFi 信号强度: " << ((int)buffer[12] > 4 ? " " : signalStrengths[buffer[12]]);
        cout << "  路由器状态: " << ((int)buffer[18] > 6 ? " " : routeStates[buffer[18]]);
        cout << " 美的云状态: " << ((int)buffer[19] > 5 ? " " : mideaSates[buffer[19]]);
        printf(" 模块IP: %d.%d.%d.%d\n", buffer[16], buffer[15], buffer[14], buffer[13]);
    }
    // 拦截内容
    void captureOrder(uint8_t *buffer, int len)
    {
        // 设备指令小于 10 没必要注入
        if (len >= 10)
        {
            // 指令
            switch ((unsigned char)buffer[9])
            {
            case 0x61:
                processDate(buffer, len);
                break;
            case 0x13:
                processMac(buffer, len);
                break;
            case 0x87:
                processWiFiVersion(buffer, len);
            case 0x63:
                processNetstate(buffer, len);
                break;
            default:
                break;
            }
        }
    }
    /**
     * CRC 校验
     */
    unsigned char orderCheckSum(uint8_t *buffer, int len)
    {
        unsigned char *p = buffer;
        unsigned char checksum = 0;
        p++;
        for (int i = 0; i < len - 2; i++)
        {
            checksum += *p;
            p++;
        }
        checksum = ~checksum + 1;
        return checksum;
    }
};