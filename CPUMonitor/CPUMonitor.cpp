// CPUMonitor.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static void showUsage(std::string name)
{
    std::cerr << "Usage: " << name << " <option(s)>"
              << "Options:\n"
              << "\t-h,--help\t\tShow this help message\n"
              << "\t-d,--duration <time_in_seconds>\tDuration of the monitoring in seconds\n"
              << "\t-i,--interval <time_in_milisecods>\tInterval between captures in ms. Min 50 ms\n"
              << "\t-o,--output <path>\tFile where with captured information\n"
              << std::endl;
}

void saveTrace(std::string path, std::vector<int> cpuValues, SYSTEMTIME st, SYSTEMTIME et) {

	char stMessage[20];
	sprintf(stMessage, "%02d:%02d:%02d.%03d", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	char etMessage[20];
	sprintf(etMessage, "%02d:%02d:%02d.%03d", et.wHour, et.wMinute, et.wSecond, et.wMilliseconds);

    std::ofstream outputFile(path);
	outputFile << stMessage << " - " << etMessage << std::endl;
    std::ostream_iterator<int> outputIterator(outputFile, "\n");
    std::copy(cpuValues.begin(), cpuValues.end(), outputIterator);

}

void getCPUConsumption(int duration, int interval, std::string path){

	std::vector<int> cpuValues;

    duration *= 1000;
    duration /= interval;
    int i = 0;

    SYSTEMTIME st;
    GetSystemTime(&st);

    while (i < duration) {
        FILETIME old_idleTime;
        FILETIME old_kernelTime;
        FILETIME old_userTime;
        BOOL res = GetSystemTimes( &old_idleTime, &old_kernelTime, &old_userTime );
        Sleep(interval);
        FILETIME idleTime;
        FILETIME kernelTime;
        FILETIME userTime;
        res = GetSystemTimes( &idleTime, &kernelTime, &userTime );

        ULARGE_INTEGER idleLow;
        idleLow.QuadPart = idleTime.dwLowDateTime - old_idleTime.dwLowDateTime;

        ULARGE_INTEGER idleHigh;
        idleHigh.QuadPart = idleTime.dwHighDateTime - old_idleTime.dwHighDateTime;

        ULARGE_INTEGER kernelLow;
        kernelLow.QuadPart = kernelTime.dwLowDateTime - old_kernelTime.dwLowDateTime;

        ULARGE_INTEGER kernelHigh;
        kernelHigh.QuadPart = kernelTime.dwHighDateTime - old_kernelTime.dwHighDateTime;

        ULARGE_INTEGER userLow;
        userLow.QuadPart = userTime.dwLowDateTime - old_userTime.dwLowDateTime;

        ULARGE_INTEGER userHigh;
        userHigh.QuadPart = userTime.dwHighDateTime - old_userTime.dwHighDateTime;

        int user = userLow.QuadPart;
        int kernel = kernelLow.QuadPart;
        int idle = idleLow.QuadPart;
        int sys = user + kernel;
        int cpuUse = (sys - idle)*100 / sys;
        //printf("CPU: %d\n", cpuUse);
        cpuValues.push_back(cpuUse);
        i++;
    }

	SYSTEMTIME et;
    GetSystemTime(&et);

	saveTrace(path, cpuValues, st, et);
    
}

int main(int argc, char* argv[])
{
	
    int option = 0;
    int interval = -1, duration = -1;
    std::string outputPath;

    if (argc < 6) {
        showUsage(argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            showUsage(argv[0]);
            return 0;
        } else if ((arg == "-d") || (arg == "--duraton")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                i++;
                duration = atoi(argv[i]);
            } else {
                  std::cerr << "--duration option requires one argument." << std::endl;
                return 1;
            }  
        } else if ((arg == "-i") || (arg == "--interval")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                i++;
                interval = atoi(argv[i]);
            } else {
                  std::cerr << "--interval option requires one argument." << std::endl;
                return 1;
            }  
        } else if ((arg == "-o") || (arg == "--output")) {
            if (i + 1 < argc) { // Make sure we aren't at the end of argv!
                i++;
                outputPath = argv[i];
            } else {
                  std::cerr << "--output option requires one argument." << std::endl;
                return 1;
            }  
        }
    }

	getCPUConsumption(duration, interval, outputPath);

    return 0;
}

