#include <windows.h>
#include <stdio.h>

int main()
{
    SYSTEM_POWER_STATUS status;

    if (GetSystemPowerStatus(&status))
    {
        printf("Battery %d%%\n", status.BatteryLifePercent);

        int hours = status.BatteryLifeTime / 3600;
        int minutes = (status.BatteryLifeTime % 3600) / 60;

        printf("Time remaining: %d hours %d minutes (%lu m.)\n", hours, minutes, status.BatteryLifeTime / 60);

        printf("Status: %s\n",
               status.ACLineStatus ? "Connected to AC power" : "Running on battery");

        if (status.BatteryFlag == 1)
        {
            printf("Battery is discharging.\n");
        }
        else if (status.BatteryFlag == 8)
        {
            printf("Battery is charging.\n");
        }
        else if (status.BatteryFlag == 128)
        {
            printf("Battery is fully charged.\n");
        }
        else
        {
            printf("Unknown battery status.\n");
        }
    }
    else
        printf("Error getting power status\n");

    return 0;
}
