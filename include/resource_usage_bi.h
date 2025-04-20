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

#define DIV 1048576 // bytes to MB

class resource_usage_bi
{
public:
    resource_usage_bi()
    {
        
    }
    
    struct CpuInfo 
    {
        std::string UsagePercent;
        std::vector<std::string> CoreUsagePercents;

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
    
    CpuInfo cpuInfo;
    RamInfo ramInfo;

    bool start_With_Windows = false;
    bool minimize_To_Tray = false;
    bool exit_on_key_esc = false;
    
    bool updateRam();
    bool updateCpu();

    void cleanup();
};

#endif