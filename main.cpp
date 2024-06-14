#include <windows.h>
#include <commctrl.h>
#include <string>
#include "resource.h" // Include your resource file

HINSTANCE hInst;

// Function to calculate CPU load
double CalculateCPULoad(unsigned long long idleTicks, unsigned long long totalTicks) {
    static unsigned long long previousTotalTicks = 0;
    static unsigned long long previousIdleTicks = 0;

    unsigned long long totalTicksSinceLastTime = totalTicks - previousTotalTicks;
    unsigned long long idleTicksSinceLastTime = idleTicks - previousIdleTicks;

    double ret = 1.0 - ((totalTicksSinceLastTime > 0) ? ((double)idleTicksSinceLastTime) / totalTicksSinceLastTime : 0);

    previousTotalTicks = totalTicks;
    previousIdleTicks = idleTicks;
    return ret;
}

// Function to convert FILETIME to unsigned long long
unsigned long long FileTimeToInt64(const FILETIME &ft) {
    return (((unsigned long long)(ft.dwHighDateTime)) << 32) | ((unsigned long long)ft.dwLowDateTime);
}

// Function to get CPU load
double GetCPULoad() {
    FILETIME idleTime, kernelTime, userTime;
    return GetSystemTimes(&idleTime, &kernelTime, &userTime) ?
           CalculateCPULoad(FileTimeToInt64(idleTime), FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime)) : -1.0;
}

// Function to get memory usage
std::string GetMemoryUsage() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    std::string memoryInfo;
    memoryInfo += "Memory Load: " + std::to_string(statex.dwMemoryLoad) + " percent\n";
    memoryInfo += "Total Physical Memory: " + std::to_string(statex.ullTotalPhys / 1024 / 1024 / 1024 ) + " GB\n";
    memoryInfo += "Free Physical Memory: " + std::to_string(statex.ullAvailPhys / 1024 / 1024 / 1024) + " GB\n";
    memoryInfo += "Total Page File: " + std::to_string(statex.ullTotalPageFile / 1024 / 1024 / 1024) + " GB\n";
    memoryInfo += "Free Page File: " + std::to_string(statex.ullAvailPageFile / 1024 / 1024 / 1024) + " GB\n";
    memoryInfo += "Total Virtual Memory: " + std::to_string(statex.ullTotalVirtual / 1024 / 1024 / 1024) + " GB\n";
    memoryInfo += "Free Virtual Memory: " + std::to_string(statex.ullAvailVirtual / 1024 / 1024 / 1024) + " GB\n";

    // Memory usage alert
    if (statex.dwMemoryLoad > 75) {
        memoryInfo += "Alert: Memory usage is above 75%!\n";
    }

    return memoryInfo;
}

// Function to get battery status
std::string GetBatteryStatus() {
    SYSTEM_POWER_STATUS spsPwr;
    std::string batteryInfo;

    if (GetSystemPowerStatus(&spsPwr)) {
        batteryInfo += "Battery Life: " + std::to_string((int)spsPwr.BatteryLifePercent) + "%\n";
        if (spsPwr.BatteryLifePercent < 20) {
            batteryInfo += "Alert: Battery is below 20%!\n";
        }
    } else {
        batteryInfo += "Unable to get battery status.\n";
    }

    return batteryInfo;
}

// Function to get system health information
std::string GetSystemHealthInfo() {
    std::string info = "System Health Check for HP ProBook 440 G5\n\n";

    double cpuLoad = GetCPULoad() * 100;
    info += "CPU Load: " + std::to_string(cpuLoad) + "%\n";
    if (cpuLoad > 80) {
        info += "Alert: CPU load is above 80%!\n";
    }

    info += GetMemoryUsage();
    info += GetBatteryStatus();

    info += "System health check completed.\n";
    return info;
}

// Function to convert std::string to std::wstring
std::wstring StringToWString(const std::string &str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Dialog box procedure
BOOL CALLBACK DlgMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_INITDIALOG: {
            // Set dialog box background color
            HBRUSH hBrush = CreateSolidBrush(RGB(0, 191, 255));
            SetClassLongPtr(hwndDlg, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

            // Create a progress bar control
            HWND hwndPB = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
                20, 250, 200, 20, hwndDlg, (HMENU)IDC_PROGRESS, hInst, NULL);

            // Get system health information
            std::string healthInfo = GetSystemHealthInfo();
            std::wstring healthInfoW = StringToWString(healthInfo);

            // Set custom font for the text
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Arial"));
            SendDlgItemMessage(hwndDlg, IDC_TEXT, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Set the health info text in the dialog
            SetDlgItemTextW(hwndDlg, IDC_TEXT, healthInfoW.c_str());

            // Update the progress bar with the CPU load
            double cpuLoad = GetCPULoad() * 100;
            SendMessage(hwndPB, PBM_SETPOS, (WPARAM)cpuLoad, 0);
        }
        return TRUE;


/*

         case WM_CTLCOLORDLG: {
            // Change the background color of the dialog box
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, RGB(0, 191, 255)); // Set background color to bright blue
            return (INT_PTR)CreateSolidBrush(RGB(0, 191, 255)); // Return a bright blue brush
        }


*/




        case WM_CLOSE: {
            EndDialog(hwndDlg, 0);
        }
        return TRUE;
        case WM_COMMAND: {
            // Handle any additional commands (if needed)
        }
        return TRUE;
    }
    return FALSE;
}

// Entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    hInst = hInstance;
    InitCommonControls(); // Initialize common controls (required for dialog boxes)

    // Create the dialog box
    DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN_DIALOG), NULL, (DLGPROC)DlgMain);

    return 0;
}
