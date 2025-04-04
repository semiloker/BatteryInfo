#ifndef BATTERYINFO_H
#define BATTERYINFO_H

#include <windows.h>
#include <initguid.h>
#include <setupapi.h>
#include <batclass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#pragma comment(lib, "setupapi.lib")

DEFINE_GUID(GUID_DEVINTERFACE_BATTERY,
0x72631e54, 0x78a4, 0x11d0, 0xbc, 0xf7, 0x00, 0xaa, 0x00, 0xb7, 0xb3, 0x2a);

class batteryinfo_bi
{
public:
    struct bi_struct // battery info (bi) _ struct
    {
        std::string Chemistry;
        std::string DesignedCapacity;
        std::string FullChargedCapacity;
        std::string DefaultAlert1;
        std::string DefaultAlert2;
    };

};

#endif