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
    };
    
    CpuInfo cpuInfo;
    RamInfo ramInfo;

    bool updateRam();
    bool updateCpu();

    void cleanup();
};

#endif