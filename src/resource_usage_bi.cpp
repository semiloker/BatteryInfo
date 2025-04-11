#include "../include/resource_usage_bi.h"

bool resource_usage_bi::updateRam()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex))
    {
        ramInfo.dwMemoryLoad = std::to_string(statex.dwMemoryLoad) + " %";
        ramInfo.ullTotalPhys = std::to_string(statex.ullTotalPhys / DIV) + " MB";
        ramInfo.ullAvailPhys = std::to_string(statex.ullAvailPhys / DIV) + " MB";
        ramInfo.ullTotalPageFile = std::to_string(statex.ullTotalPageFile / DIV) + " MB";
        ramInfo.ullAvailPageFile = std::to_string(statex.ullAvailPageFile / DIV) + " MB";
        ramInfo.ullTotalVirtual = std::to_string(statex.ullTotalVirtual / DIV) + " MB";
        ramInfo.ullAvailVirtual = std::to_string(statex.ullAvailVirtual / DIV) + " MB";
        ramInfo.ullAvailExtendedVirtual = std::to_string(statex.ullAvailExtendedVirtual / DIV) + " MB";
        return true;
    }

    return false;
}

bool resource_usage_bi::updateCpu() 
{
    static PDH_HQUERY query = NULL;
    static std::vector<PDH_HCOUNTER> counters;
    static DWORD coreCount = 0;

    PDH_STATUS status;

    if (query == NULL) 
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        coreCount = sysInfo.dwNumberOfProcessors;

        status = PdhOpenQuery(NULL, 0, &query);
        if (status != ERROR_SUCCESS) 
        {
            return false;
        }

        counters.resize(coreCount);
        cpuInfo.CoreUsagePercents.resize(coreCount);

        for (DWORD i = 0; i < coreCount; ++i)
        {
            std::wostringstream path;
            path << L"\\Processor(" << i << L")\\% Processor Time";

            status = PdhAddEnglishCounterW(query, path.str().c_str(), 0, &counters[i]);
            if (status != ERROR_SUCCESS) 
            {
                PdhCloseQuery(query);
                query = NULL;
                counters.clear();
                return false;
            }
        }

        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS) 
        {
            PdhCloseQuery(query);
            query = NULL;
            counters.clear();
            return false;
        }

        return true;
    }

    status = PdhCollectQueryData(query);
    if (status != ERROR_SUCCESS) 
    {
        PdhCloseQuery(query);
        query = NULL;
        counters.clear();
        return false;
    }

    double total = 0.0;
    int validCoreCount = 0;

    for (DWORD i = 0; i < coreCount; ++i)
    {
        PDH_FMT_COUNTERVALUE counterVal;
        DWORD counterType;

        status = PdhGetFormattedCounterValue(counters[i], PDH_FMT_DOUBLE, &counterType, &counterVal);
        if (status == ERROR_SUCCESS) 
        {
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << counterVal.doubleValue;
            cpuInfo.CoreUsagePercents[i] = oss.str() + " %";

            total += counterVal.doubleValue;
            ++validCoreCount;
        }
        else 
        {
            cpuInfo.CoreUsagePercents[i] = "N/A";
        }
    }

    if (validCoreCount > 0)
    {
        std::ostringstream avg;
        avg << std::fixed << std::setprecision(2) << (total / validCoreCount);
        cpuInfo.UsagePercent = avg.str() + " %";
    }
    else
    {
        cpuInfo.UsagePercent = "N/A";
    }

    return true;
}

void resource_usage_bi::cleanup() 
{
    static PDH_HQUERY& query = []() -> PDH_HQUERY& { static PDH_HQUERY q = NULL; return q; }();
    
    if (query != NULL) 
    {
        PdhCloseQuery(query);
        query = NULL;
    }
}