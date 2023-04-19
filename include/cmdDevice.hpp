#pragma once
#include "commander.hpp"
#include "deviceOrderProcess.hpp"
#include "wifiOrderProcess.hpp"
#include <map>
extern DeviceOrder deviceOrder;
extern WiFiOrder wifiOrder;
class CmdDevice : public Commander
{
public:
    CmdDevice(Commander &commander)
    {
        init();
    }
    CmdDevice()
    {
        setCmd("device");
        setDesc("display/set device info.");
        setExample("{\"cmd\":\"device\",\"op\":\"help\"}");
        init();
    }
    ~CmdDevice()
    {
    }
    void cmdProcess(DynamicJsonDocument doc)
    {
        string op = doc["op"];
        if (op.compare("null") == 0 || funcMap.count(op) == 0)
        {
            displayCmds(doc);
        }
        else
        {
            RunFunc(op, doc);
        }
    }

private:
    /**
     * 设备重启
     */
    void restartSystem(DynamicJsonDocument doc)
    {
        cout << ">> 设备即将在 3 秒内重启!" << endl;
        HAL_Delay(3000);
        HAL_NVIC_SystemReset();
    }
    /**
     * 获取 WiFi 模块时间
     */
    void queryDate(DynamicJsonDocument doc)
    {
        cout << ">> 即将获取 WiFi 模块时间!" << endl;
        wifiOrder.queryDate();
    }
    /**
     * 获取 WiFi Mac
     */
    void queryMac(DynamicJsonDocument doc)
    {
        cout << ">> 即将获取 WiFi Mac!" << endl;
        wifiOrder.queryMac();
    }
    /**
     * 获取 WiFi 网络状态
     */
    void queryNetState(DynamicJsonDocument doc)
    {
        cout << ">> 即将获取 WiFi 网络状态!" << endl;
        wifiOrder.queryNetState();
    }
    /**
     * 获取 WiFi Version
     */
    void queryWiFiVersion(DynamicJsonDocument doc)
    {
        cout << ">> 即将获取 WiFi Version!" << endl;
        wifiOrder.queryWiFiVersion();
    }
    /**
     * 重置 WiFi 模块
     */
    void restartWiFi(DynamicJsonDocument doc)
    {
        cout << ">> 即将重置 WiFi 模块!" << endl;
        HAL_Delay(1000);
        wifiOrder.resetWiFi();
    }
    /**
     * 文件操作帮助命令
     */
    void displayCmds(DynamicJsonDocument doc)
    {
        cout << "/***********************************************/" << endl;
        cout << "DeviceOps: " << endl;
        for (auto iter = funcDescMap.begin(); iter != funcDescMap.end(); ++iter)
        {
            cout << "  " << iter->first << ", " << iter->second << endl;
        }
        cout << "/***********************************************/" << endl;
    }
    void saveEnv2File(DynamicJsonDocument doc)
    {
        deviceOrder.saveSystemConfig2File();
    }
    void setDeviceValues(DynamicJsonDocument doc)
    {
        if (!doc["boolCheatA0"].isNull())
        {
            if (doc["boolCheatA0"] == true)
            {
                deviceOrder.setBoolCheatA0(true);
            }
            else
            {
                deviceOrder.setBoolCheatA0(false);
            }
        }
        if (!doc["projectNo"].isNull())
        {
            if (doc["projectNo"] > 0)
            {
                deviceOrder.setProjectNo(doc["projectNo"]);
            }
        }
        if (!doc["boolCheatSN"].isNull())
        {
            if (doc["boolCheatSN"] == true)
            {
                deviceOrder.setBoolCheatSN(true);
            }
        }
        if (!doc["sn"].isNull())
        {
            const char *_sn = doc["sn"];
            string sn = _sn;
            deviceOrder.setSN(sn);
        }
        if (!doc["hotPrefix"].isNull())
        {
            const char *_hotPrefix = doc["hotPrefix"];
            string hotPrefix = _hotPrefix;
            deviceOrder.setHotPrefix(hotPrefix);
        }
        cout << ">> Device 设置完成." << endl;
        deviceOrder.showDeviceConfig();
    }
    typedef void((CmdDevice::*Fun_ptr))(DynamicJsonDocument doc);
    /**
     * 注入操作
     */
    void init()
    {
        funcMap.insert(make_pair("save", &CmdDevice::saveEnv2File));
        funcMap.insert(make_pair("set", &CmdDevice::setDeviceValues));
        funcMap.insert(make_pair("help", &CmdDevice::displayCmds));
        funcMap.insert(make_pair("restart", &CmdDevice::restartSystem));
        funcMap.insert(make_pair("resetWiFi", &CmdDevice::restartWiFi));
        funcMap.insert(make_pair("queryDate", &CmdDevice::queryDate));
        funcMap.insert(make_pair("queryMac", &CmdDevice::queryMac));
        funcMap.insert(make_pair("queryWiFiVersion", &CmdDevice::queryWiFiVersion));
        funcMap.insert(make_pair("queryNetState", &CmdDevice::queryNetState));
        funcDescMap.insert(make_pair("save", "保存设备变量,example: {\"cmd\":\"device\",\"op\":\"save\"}"));
        funcDescMap.insert(make_pair("set", "设置设备变量,example: {\"cmd\":\"device\",\"op\":\"set\",\"boolCheatA0\":true,\"projectNo\":13104,\"sn\":\"0000DB39188888888341800000130000\",\"boolCheatSN\":true,\"hotPrefix\":\"midea\"}"));
        funcDescMap.insert(make_pair("help", "帮助,example: {\"cmd\":\"device\",\"op\":\"help\"}"));
        funcDescMap.insert(make_pair("restart", "重启设备,example: {\"cmd\":\"device\",\"op\":\"restart\"}"));
        funcDescMap.insert(make_pair("resetWiFi", "重置 WiFi 模块,example: {\"cmd\":\"device\",\"op\":\"resetWiFi\"}"));
        funcDescMap.insert(make_pair("queryDate", "获取 WiFi 模块时间,example: {\"cmd\":\"device\",\"op\":\"queryDate\"}"));
        funcDescMap.insert(make_pair("queryMac", "获取 WiFi Mac,example: {\"cmd\":\"device\",\"op\":\"queryMac\"}"));
        funcDescMap.insert(make_pair("queryWiFiVersion", "获取 WiFi 版本,example: {\"cmd\":\"device\",\"op\":\"queryWiFiVersion\"}"));
        funcDescMap.insert(make_pair("queryNetState", "获取 WiFi 网络状态,example: {\"cmd\":\"device\",\"op\":\"queryNetState\"}"));
    }
    // 传入函数名，执行对应的函数
    void RunFunc(string funcName, DynamicJsonDocument doc)
    {
        if (funcMap.count(funcName))
        {
            (this->*funcMap[funcName])(doc);
        }
    }
    map<string, Fun_ptr> funcMap;
    map<string, string> funcDescMap;
};