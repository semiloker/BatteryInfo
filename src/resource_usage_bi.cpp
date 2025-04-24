#include "../include/resource_usage_bi.h"

bool resource_usage_bi::updateRam()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (!GlobalMemoryStatusEx(&statex))
    {
        DWORD error = GetLastError();
        std::cerr << "Memory status failed. Error: " << error << std::endl;
        return false;
    }

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

bool resource_usage_bi::updateDisk()
{
    DWORD drives = GetLogicalDrives();
    disksInfo.clear();

    for (char drive = 'A'; drive <= 'Z'; ++drive)
    {
        if (drives & (1 << (drive - 'A')))
        {
            std::string rootPath = std::string(1, drive) + ":\\";
            DiskInfo disk;
            disk.diskLetter = rootPath;

            ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
            if (GetDiskFreeSpaceExA(rootPath.c_str(), &freeBytes, &totalBytes, &totalFreeBytes))
            {
                disk.totalSpace = std::to_string(totalBytes.QuadPart / DIV) + " MB";
                disk.freeSpace = std::to_string(freeBytes.QuadPart / DIV) + " MB";
                disk.usedSpace = std::to_string((totalBytes.QuadPart - freeBytes.QuadPart) / DIV) + " MB";

                double usage = (1.0 - (static_cast<double>(freeBytes.QuadPart) / totalBytes.QuadPart)) * 100;
                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2) << usage << " %";
                disk.usagePercent = oss.str();

                disksInfo.push_back(disk);
            }
        }
    }
    return !disksInfo.empty();
}

bool resource_usage_bi::updateNetwork()
{
    MIB_IFTABLE *pIfTable = NULL;
    DWORD dwSize = 0;

    if (GetIfTable(pIfTable, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER)
    {
        pIfTable = (MIB_IFTABLE *)malloc(dwSize);
        if (!pIfTable)
            return false;
    }

    if (GetIfTable(pIfTable, &dwSize, TRUE) == NO_ERROR)
    {
        for (DWORD i = 0; i < pIfTable->dwNumEntries; ++i)
        {
            NetworkInfo netInfo;
            MIB_IFROW row = pIfTable->table[i];

            // Конвертируем имя интерфейса из wide-char
            char name[256];
            wcstombs(name, row.wszName, sizeof(name));
            netInfo.interfaceName = name;

            netInfo.downloadSpeed = std::to_string(row.dwInOctets / 1024) + " KB/s";
            netInfo.uploadSpeed = std::to_string(row.dwOutOctets / 1024) + " KB/s";

            networkInfo.push_back(netInfo);
        }
        free(pIfTable);
        return true;
    }

    if (pIfTable)
        free(pIfTable);
    return false;
}

void resource_usage_bi::initCpuInfo()
{
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        char cpuName[256];
        DWORD size = sizeof(cpuName);
        if (RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL,
                            (LPBYTE)cpuName, &size) == ERROR_SUCCESS)
        {
            cpuInfo.cpuName = cpuName;
        }
        RegCloseKey(hKey);
    }

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cpuInfo.architecture =
        (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "x64" : (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM) ? "ARM"
                                                                               : (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)  ? "IA64"
                                                                               : (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) ? "x86"
                                                                                                                                                  : "Unknown";
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

    if (status != ERROR_SUCCESS)
    {
        std::cerr << "Pdh error: " << status << std::endl;
        return false;
    }

    return true;
}

void resource_usage_bi::cleanup()
{
    CoUninitialize();

    static PDH_HQUERY &query = []() -> PDH_HQUERY &
    { static PDH_HQUERY q = NULL; return q; }();

    if (query != NULL)
    {
        PdhCloseQuery(query);
        query = NULL;
    }

    networkInfo.clear();
    disksInfo.clear();
}

bool resource_usage_bi::isStartWithWindowsEnabled() 
{
    HKEY hKey;
    const char* appName = "BatteryInfo";
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    char path[MAX_PATH] = {0};
    DWORD pathSize = sizeof(path);
    
    result = RegQueryValueExA(hKey, appName, 0, NULL, (LPBYTE)path, &pathSize);
    RegCloseKey(hKey);
    
    return (result == ERROR_SUCCESS);
}

bool resource_usage_bi::enableStartWithWindows() 
{
    HKEY hKey;
    const char* appName = "BatteryInfo";
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    char path[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, path, MAX_PATH);
    
    result = RegSetValueExA(hKey, appName, 0, REG_SZ, (BYTE*)path, strlen(path) + 1);
    RegCloseKey(hKey);
    
    start_With_Windows = true;
    return (result == ERROR_SUCCESS);
}

bool resource_usage_bi::disableStartWithWindows() 
{
    HKEY hKey;
    const char* appName = "BatteryInfo";
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);
    
    if (result != ERROR_SUCCESS) 
    {
        return false;
    }
    
    result = RegDeleteValueA(hKey, appName);
    RegCloseKey(hKey);
    
    start_With_Windows = false;
    return (result == ERROR_SUCCESS);
}

bool resource_usage_bi::toggleStartWithWindows() 
{
    if (!isStartWithWindowsEnabled()) 
    {
        return disableStartWithWindows();
    } 
    else 
    {
        return enableStartWithWindows();
    }
}