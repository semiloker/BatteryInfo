#ifndef RESOURCE_USAGE_BI_H
#define RESOURCE_USAGE_BI_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cstdint>
#include <pdh.h>
#include <pdhmsg.h>
#include <iphlpapi.h>
#include <comdef.h>
#include <Wbemidl.h>

#define DIV 1048576 // bytes to MB

class resource_usage_bi
{
public:
    resource_usage_bi()
    {
        CoInitializeEx(NULL, COINIT_MULTITHREADED);
        initCpuInfo();
    }
    
    struct CpuInfo 
    {
        std::string UsagePercent;
        std::vector<std::string> CoreUsagePercents;
    
        std::string cpuName;
        std::string architecture;

        bool show_cpuName = false;
        bool show_architecture = false;
        bool show_UsagePercent = false;
        bool show_CoreUsagePercents = false;
    };

    struct RamInfo
    {
        std::string dwMemoryLoad;
        std::string ullTotalPhys;
        std::string ullAvailPhys;
        std::string ullTotalPageFile;
        std::string ullAvailPageFile;
        std::string ullTotalVirtual;
        std::string ullAvailVirtual;
        std::string ullAvailExtendedVirtual;

        bool show_dwMemoryLoad = false;
        bool show_ullTotalPhys = false;
        bool show_ullAvailPhys = false;
        bool show_ullTotalPageFile = false;
        bool show_ullAvailPageFile = false;
        bool show_ullTotalVirtual = false;
        bool show_ullAvailVirtual = false;
        bool show_ullAvailExtendedVirtual = false;

    };

    struct DiskInfo 
    {
        std::string diskLetter;
        std::string totalSpace;
        std::string freeSpace;
        std::string usedSpace;
        std::string usagePercent;

        bool show_diskLetter = false;
        bool show_totalSpace = false;
        bool show_freeSpace = false;
        bool show_usedSpace = false;
        bool show_usagePercent = false;
    };

    struct NetworkInfo 
    {
        std::string interfaceName;
        std::string downloadSpeed;
        std::string uploadSpeed;

        bool show_interfaceName = false;
        bool show_downloadSpeed = false;
        bool show_uploadSpeed = false;
    };

    CpuInfo cpuInfo;
    RamInfo ramInfo;
    std::vector<DiskInfo> disksInfo;    
    std::vector<NetworkInfo> networkInfo;
    
    bool start_With_Windows = false;
    bool minimize_To_Tray = false;
    bool exit_on_key_esc = false;
    
    bool updateRam();
    bool updateCpu();
    void initCpuInfo();
    bool updateDisk();
    bool updateNetwork();
    
    bool updateAll() 
    {
        bool success = false;
        success &= updateRam();
        success &= updateCpu();
        success &= updateDisk();
        success &= updateNetwork();
        return success;
    }
    
    void cleanup();
};

#endif