# Round Robin File Backup Tool - Linux System and Shell Programming
This project implements a Simple Round-Robin Backup Tool in C for Linux systems.
It automatically maintains multiple versions of files from a source directory, ensuring only the latest ‘n’ versions of each file are stored.

If a file changes in the source directory, the tool creates a new backup version.
If it hasn’t changed, the tool skips copying to save time and space.
Each file in the source directory gets its own backup subfolder inside the main backup directory — keeping versions organized as:
```
backup/
 ├── file1.txt_backup/
 │    ├── version_2025-11-11_10-30-22.txt                                                                                                                                                        
 │    ├── version_2025-11-11_11-00-40.txt
 │    └── version_2025-11-11_12-10-10.txt
 ├── file2.txt_backup/
 │    ├── version_2025-11-11_09-50-10.txt
 │    ├── version_2025-11-11_10-10-45.txt
 │    └── version_2025-11-11_10-30-55.txt
```
## Key Features

* Automatic versioning — keeps track of modified files and backs them up with timestamps
* Change detection — only backs up files that have changed since the last version
* Space-efficient — keeps only the latest n versions of each file (oldest versions are deleted automatically)
* Organized structure — each file’s versions are stored neatly in its own directory

 ## How It Works (Concept)
The program scans the source directory for all files. For each file:

* If it has changed since the last backup → a new timestamped version is created.
* If it has not changed → the backup is skipped.

The program ensures that only the latest N versions are kept (oldest are deleted). Each file’s versions are stored in a separate folder inside the backup directory.

## Compile the program using the GCC compiler on Linux.
```
gcc backup.c -o backup
```
## Run the program

```
./backup_tool <source_directory> <backup_directory> <num_versions>
```


## Step-by-Step Testing Guide

### 1. Create a source directory and files
```
mkdir source
echo "This is file1" > source/file1.txt
echo "This is file2" > source/file2.txt
```

### 2. Run the first backup
```
./backup_tool source backup 3
```

* It will create a new backup folder backup/
* Inside it, subdirectories like file1.txt_backup/ and file2.txt_backup/ will be created
* Each will contain a single version file

### 3. Modify one file

``` echo "Updated file1 - version 2" >> source/file1.txt```


### 4. Run the backup again

```./backup source backup 3```

Only file1.txt will get a new version, file2.txt will be skipped since it didn’t change.
### 5. Modify multiple times to test version rotation.
Edit file1.txt a few times and re-run the tool. Once there are more than 3 versions, the oldest one will be deleted automatically.Check backups.

```ls backup/file1.txt_backup/```


You’ll see the latest three versions only.
* Example Output
```
New version created for file1.txt → version_2025-11-11_10-30-22.txt
New version created for file2.txt → version_2025-11-11_10-30-22.txt
No change in file2.txt, skipping backup.
Deleting old version: backup/file1.txt_backup/version_2025-11-10_22-45-50.txt

Backup completed successfully.
```

### Directory Structure After Multiple Runs
```
source/
 ├── file1.txt
 └── file2.txt

backup/
 ├── file1.txt_backup/
 │    ├── version_2025-11-11_10-30-22.txt
 │    ├── version_2025-11-11_11-00-40.txt
 │    └── version_2025-11-11_12-10-10.txt
 └── file2.txt_backup/
      ├── version_2025-11-11_09-50-10.txt
      ├── version_2025-11-11_10-10-45.txt
      └── version_2025-11-11_10-30-55.txt
```

