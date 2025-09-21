#include "proc_reader.h"

int list_process_directories(void) {
    
    DIR* dir = opendir("/proc");
    if (!dir) {
        perror("opendir failed");
        return -1;
    }

    struct dirent* entry;
    int count = 0;

    printf("Process directories in /proc:\n");
    printf("%-8s %-20s\n", "PID", "Type");
    printf("%-8s %-20s\n", "---", "----");

    while ((entry = readdir(dir)) != NULL) {
        if (is_number(entry->d_name)) {
            printf("%-8s %-20s\n", entry->d_name, "process");
            count++;
        }
    }
    
    if (closedir(dir) == -1) {
        perror("closedir failed");
        return -1;
    }

    printf("Found %d process directories\n", count);
    return 0;
}

int read_process_info(const char* pid) {
    char filepath[256];

    snprintf(filepath, sizeof(filepath), "/proc/%s/status", pid);

    printf("\n--- Process Information for PID %s ---\n", pid);

    if (read_file_with_syscalls(filepath) == -1) {
        printf("ERROR: Could not read %s\n", filepath);
        return -1;
    }

    snprintf(filepath, sizeof(filepath), "/proc/%s/cmdline", pid);

    printf("\n--- Command Line ---\n");

    if (read_file_with_syscalls(filepath) == -1) {
        printf("ERROR: Could not read %s\n", filepath);
        return -1;
    }

    printf("\n"); 
    return 0; 
}

int show_system_info(void) {
    int line_count = 0;
    const int MAX_LINES = 10;
    char buffer[1024];

    printf("\n--- CPU Information (first %d lines) ---\n", MAX_LINES);

    FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
    if (!cpuinfo) {
        perror("fopen cpuinfo failed");
        return -1;
    }

    while (line_count < MAX_LINES && fgets(buffer, sizeof(buffer), cpuinfo)) {
        printf("%s", buffer);
        line_count++;
    }

    if (fclose(cpuinfo) == EOF) {
        perror("fclose cpuinfo failed");
        return -1;
    }

    printf("\n--- Memory Information (first %d lines) ---\n", MAX_LINES);

    FILE* meminfo = fopen("/proc/meminfo", "r");
    if (!meminfo) {
        perror("fopen meminfo failed");
        return -1;
    }

    line_count = 0;
    
    while (line_count < MAX_LINES && fgets(buffer, sizeof(buffer), meminfo)) {
        printf("%s", buffer);
        line_count++;
    }

    if (fclose(meminfo) == EOF) {
        perror("fclose meminfo failed");
        return -1;
    }

    return 0;
}

void compare_file_methods(void) {
    const char* test_file = "/proc/version";

    printf("Comparing file reading methods for: %s\n\n", test_file);

    printf("=== Method 1: Using System Calls ===\n");
    read_file_with_syscalls(test_file);

    printf("\n=== Method 2: Using Library Functions ===\n");
    read_file_with_library(test_file);

    printf("\nNOTE: Run this program with strace to see the difference!\n");
    printf("Example: strace -e trace=openat,read,write,close ./lab2\n");
}

int read_file_with_syscalls(const char* filename) {
    int fd = open(filename, O_RDONLY);
    char buffer[1024];
    ssize_t bytes_read;

    if (fd == -1) {
        perror("open failed");
        return -1;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }

    if (bytes_read == -1) {
        perror("read failed");
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("close failed");
        return -1;
    }

    return 0;
}

int read_file_with_library(const char* filename) {
    
    FILE* file = fopen(filename, "r");   
    if (!file) {                         
        perror("fopen failed");
        return -1;
    }

    char line[256];                      
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);              
    }

    if (fclose(file) == EOF) {           
        perror("fclose failed");
        return -1;
    }

    return 0;
}

int is_number(const char* str) {
    if (!str || *str == '\0') {
        return 0;
    }

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;
        }
        str++;
    }

    return 1;
}