#include "../include/resource_usage_bi.h"
#include <string>

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
    static PDH_HCOUNTER counter = NULL;
    PDH_STATUS status;
    
    if (query == NULL) 
    {
        status = PdhOpenQuery(NULL, 0, &query);
        if (status != ERROR_SUCCESS) 
        {
            return false;
        }
        
        LPCWSTR counterPath = L"\\Processor(_Total)\\% Processor Time";
        
        status = PdhAddEnglishCounterW(query, counterPath, 0, &counter);
        if (status != ERROR_SUCCESS) 
        {
            PdhCloseQuery(query);
            query = NULL;
            return false;
        }
        
        status = PdhCollectQueryData(query);
        if (status != ERROR_SUCCESS) 
        {
            PdhCloseQuery(query);
            query = NULL;
            counter = NULL;
            return false;
        }
        
        cpuInfo.UsagePercent = "0.0";
        return true;
    }
    
    status = PdhCollectQueryData(query);
    if (status != ERROR_SUCCESS) 
    {
        PdhCloseQuery(query);
        query = NULL;
        counter = NULL;
        return false;
    }
    
    PDH_FMT_COUNTERVALUE counterVal;
    DWORD counterType;
    status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, &counterType, &counterVal);
    if (status != ERROR_SUCCESS) 
    {
        return false;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << counterVal.doubleValue;
    cpuInfo.UsagePercent = oss.str();
    
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