
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#endif

void clear_screen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

#ifndef _WIN32
void list_processes_linux() {
    struct dirent *entry;
    DIR *dp = opendir("/proc");

    if (!dp) {
        perror("opendir");
        return;
    }

    printf("%-10s %-30s %s\n", "PID", "Name", "State");
    while ((entry = readdir(dp))) {
        if (isdigit(*entry->d_name)) {
            char path[256], line[256], name[64] = "", state[8] = "";
            FILE *fp;

            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            fp = fopen(path, "r");
            if (!fp) continue;

            while (fgets(line, sizeof(line), fp)) {
                if (strncmp(line, "Name:", 5) == 0)
                    sscanf(line, "Name:\t%63s", name);
                else if (strncmp(line, "State:", 6) == 0)
                    sscanf(line, "State:\t%7s", state);
            }

            fclose(fp);
            printf("%-10s %-30s %s\n", entry->d_name, name, state);
        }
    }
    closedir(dp);
}

void terminate_process_linux(pid_t pid) {
    if (kill(pid, SIGTERM) == 0)
        printf("Process %d terminated.\n", pid);
    else
        perror("Termination failed");
}
#endif

#ifdef _WIN32
void list_processes_windows() {
    PROCESSENTRY32 pe;
    HANDLE hSnap;

    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (hSnap == INVALID_HANDLE_VALUE) {
        printf("Failed to take snapshot\n");
        return;
    }

    printf("%-10s %-30s\n", "PID", "Process Name");

    if (Process32First(hSnap, &pe)) {
        do {
            printf("%-10lu %-30s\n", pe.th32ProcessID, pe.szExeFile);
        } while (Process32Next(hSnap, &pe));
    }

    CloseHandle(hSnap);
}

void terminate_process_windows(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        printf("Failed to open process. Try running as Administrator.\n");
        return;
    }

    if (TerminateProcess(hProcess, 0))
        printf("Process %lu terminated.\n", pid);
    else
        printf("Termination failed.\n");

    CloseHandle(hProcess);
}
#endif

int main() {
    int choice;
    char input[32];

    while (1) {
        clear_screen();
        printf("=== Mini Task Manager ===\n");
        printf("1. View Running Processes\n");
        printf("2. Terminate Process by PID\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        fgets(input, sizeof(input), stdin);
        choice = atoi(input);

        switch (choice) {
            case 1:
#ifdef _WIN32
                list_processes_windows();
#else
                list_processes_linux();
#endif
                break;

            case 2: {
                printf("Enter PID to terminate: ");
                fgets(input, sizeof(input), stdin);
#ifdef _WIN32
                DWORD pid = (DWORD)atoi(input);
                terminate_process_windows(pid);
#else
                pid_t pid = (pid_t)atoi(input);
                terminate_process_linux(pid);
#endif
                break;
            }

            case 3:
                printf("Exiting Mini Task Manager.\n");
                exit(0);

            default:
                printf("Invalid choice. Try again.\n");
        }

        printf("\nPress Enter to continue...");
        getchar();
    }

    return 0;
}
