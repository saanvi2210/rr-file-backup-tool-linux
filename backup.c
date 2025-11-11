#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define PATH_MAX_LEN 4355

// Function to copy file from src to dest
void copy_file(const char *src, const char *dest) {
    FILE *fsrc = fopen(src, "rb");
    FILE *fdest = fopen(dest, "wb");
    if (!fsrc || !fdest) {
        perror("Error opening files for copy");
        if (fsrc) fclose(fsrc);
        if (fdest) fclose(fdest);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fsrc)) > 0)
        fwrite(buffer, 1, bytes, fdest);

    fclose(fsrc);
    fclose(fdest);
}

// Compare file modification times
int file_changed(const char *src, const char *old) {
    struct stat s_src, s_old;
    if (stat(src, &s_src) < 0) return 1;   // new file
    if (stat(old, &s_old) < 0) return 1;   // no old version
    return difftime(s_src.st_mtime, s_old.st_mtime) > 0; // changed if newer
}

// Create a timestamped filename
void get_timestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    strftime(buffer, size, "version_%Y-%m-%d_%H-%M-%S.txt", localtime(&now));
}

// Count how many versions exist
int count_versions(const char *file_backup_dir) {
    int count = 0;
    DIR *dir = opendir(file_backup_dir);
    if (!dir) return 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, "version_", 8) == 0)
            count++;
    }
    closedir(dir);
    return count;
}

// Find the oldest version for deletion
void find_oldest_version(const char *file_backup_dir, char *oldest_path) {
    DIR *dir = opendir(file_backup_dir);
    if (!dir) return;
    struct dirent *entry;
    time_t oldest_time = time(NULL);
    char oldest_name[PATH_MAX_LEN] = "";

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, "version_", 8) == 0) {
            char path[PATH_MAX_LEN];
            snprintf(path, sizeof(path), "%s/%s", file_backup_dir, entry->d_name);
            struct stat st;
            if (stat(path, &st) == 0 && st.st_mtime < oldest_time) {
                oldest_time = st.st_mtime;
                strcpy(oldest_name, path);
            }
        }
    }
    closedir(dir);
    if (strlen(oldest_name) > 0)
        strcpy(oldest_path, oldest_name);
}

// Find the latest version file path
void find_latest_version(const char *file_backup_dir, char *latest_path) {
    DIR *dir = opendir(file_backup_dir);
    if (!dir) return;
    struct dirent *entry;
    time_t latest_time = 0;
    char latest_name[PATH_MAX_LEN] = "";

    while ((entry = readdir(dir))) {
        if (entry->d_type == DT_REG && strncmp(entry->d_name, "version_", 8) == 0) {
            char path[PATH_MAX_LEN];
            snprintf(path, sizeof(path), "%s/%s", file_backup_dir, entry->d_name);
            struct stat st;
            if (stat(path, &st) == 0 && st.st_mtime > latest_time) {
                latest_time = st.st_mtime;
                strcpy(latest_name, path);
            }
        }
    }
    closedir(dir);
    if (strlen(latest_name) > 0)
        strcpy(latest_path, latest_name);
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <source_dir> <backup_dir> <num_versions>\n", argv[0]);
        return 1;
    }

    const char *src_dir = argv[1];
    const char *backup_dir = argv[2];
    int num_versions = atoi(argv[3]);

    DIR *src = opendir(src_dir);
    if (!src) {
        perror("Error opening source directory");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(src))) {
        if (entry->d_type != DT_REG) continue; // skip non-files

        char src_path[PATH_MAX_LEN];
        snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);

        // Create backup folder for this file (e.g., backup/file1.txt_backup)
        char file_backup_dir[PATH_MAX_LEN];
        snprintf(file_backup_dir, sizeof(file_backup_dir), "%s/%s_backup", backup_dir, entry->d_name);
        mkdir(file_backup_dir, 0755);

        // Get latest version (if exists)
        char latest_file[PATH_MAX_LEN] = "";
        find_latest_version(file_backup_dir, latest_file);

        // If unchanged → skip backup creation
        if (strlen(latest_file) > 0 && !file_changed(src_path, latest_file)) {
            printf("No change in %s, skipping backup.\n", entry->d_name);
            continue;
        }

        // Create new version file name
        char new_file[PATH_MAX_LEN], timestamp[64];
        get_timestamp(timestamp, sizeof(timestamp));
        snprintf(new_file, sizeof(new_file), "%s/%s", file_backup_dir, timestamp);

        // Copy new version
        copy_file(src_path, new_file);
        printf("New version created for %s → %s\n", entry->d_name, timestamp);

        // Delete oldest version if limit exceeded
        int count = count_versions(file_backup_dir);
        if (count > num_versions) {
            char oldest[PATH_MAX_LEN];
            find_oldest_version(file_backup_dir, oldest);
            if (strlen(oldest) > 0) {
                printf("Deleting old version: %s\n", oldest);
                remove(oldest);
            }
        }
    }

    closedir(src);
    printf("\nBackup completed successfully.\n");
    return 0;
}