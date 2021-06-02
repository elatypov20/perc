/*
Copyright (C) 2021  Emil Latypov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Emil Latypov
(e.latypov@innopolis.university)

All the binaries will be stored in bin/ directory after compilation

You should run the program in following way:
    perc -u <username> -g <groupname> -p <path_to_scan>
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/fsuid.h>
#include <dirent.h>
#include <stdlib.h>

/**
 * This method parses command line arguments,
 * checks if all of the arguments are provided
 * and writes them into variables
 *
 * @param argc number of arguments in command line
 * @param argv arguments
 * @param username username argument will be written here
 * @param groupname groupname argument will be written here
 * @param path path argument will be written here
 * @return 0 if all 3 arguments are provided, otherwise another number
 */
long long parse_arguments(int argc, char *argv[], char **username, char **groupname, char **path) {

    char flags_found = 0;

    for (int i = 1; i < argc - 1; ++i) {
        if (!strcmp(argv[i], "-u")) {
            *username = argv[++i];
            flags_found |= 1;
        } else if (!strcmp(argv[i], "-g")) {
            *groupname = argv[++i];
            flags_found |= (1 << 1);
        } else if (!strcmp(argv[i], "-p")) {
            *path = argv[++i];
            flags_found |= (1 << 2);
        }
    }

    // 0b111 means all 3 flags are set
    return flags_found != 0b111;
}

/**
 * Just merges path and filename
 * For example:
 *      path: /usr
 *      postfix: bin
 *          => /usr/bin (will be written in dest)
 *
 * @param path source path
 * @param postfix file/dir name to be added into path
 * @param dest here the result will be written
 */
void concatenate_path(char *path, char *postfix, char *dest) {
    int path_size = strlen(path), postfix_size = strlen(postfix);

    int current_p = 0;
    for (int i = 0; i < path_size; ++i) {
        dest[current_p++] = path[i];
    }

    if (dest[current_p - 1] != '/')
        dest[current_p++] = '/';

    for (int i = 0; i < postfix_size; ++i) {
        dest[current_p++] = postfix[i];
    }

    dest[current_p] = '\0'; // show end of string
}

/**
 * Recursively scans directory for files/dirs
 * which can be rewritten by user/group.
 * To scan directory recursively it must have flags x and r
 *
 * @param path path of source directory
 * @param uid id of user to check
 * @param gid id of group to check
 */
void scan_directory_access(char *path, int uid, int gid) {
    // explicitly ignore /proc and /sys folders
    if (!strcmp(path, "/proc") || !strcmp(path, "/sys"))
        return;

    DIR *current_directory;
    if (!(current_directory = opendir(path))) {
        fprintf(stderr, "Can't open directory %s\n", path);
        return;
    }

    struct dirent *dir = readdir(current_directory);
    while (dir != NULL) {
        // ignore links to current and previous catalogs
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) {
            dir = readdir(current_directory);
            continue;
        }

        // PATH_MAX (const) - the maximum length of file path in  characters
        char new_path[PATH_MAX];
        concatenate_path(path, dir->d_name, new_path);
        struct stat statbuf;

        if (lstat(new_path, &statbuf)) {
            printf("%s\n", new_path);
            perror("lstat");
            return;
        }


        // firstly, we decide, which permissions we should look at

        // 1) if the user matches with file uid, then check by users permission
        // 2) else if group matches with file gid, then check by group permissions
        // 3) else check by other permissions

        int r_perm, w_perm, x_perm;

        if (statbuf.st_uid == uid) {
            // user matched
            r_perm = statbuf.st_mode & S_IRUSR;
            w_perm = statbuf.st_mode & S_IWUSR;
            x_perm = statbuf.st_mode & S_IXUSR;
        } else if (statbuf.st_gid == gid) {
            // user didn't match, group matched
            r_perm = statbuf.st_mode & S_IRGRP;
            w_perm = statbuf.st_mode & S_IWGRP;
            x_perm = statbuf.st_mode & S_IXGRP;
        } else {
            // user and group didn't match
            r_perm = statbuf.st_mode & S_IROTH;
            w_perm = statbuf.st_mode & S_IWOTH;
            x_perm = statbuf.st_mode & S_IXOTH;
        }

        if (S_ISDIR(statbuf.st_mode) && w_perm) {
            // If it is writable dir, then print path to it
            printf("d %s\n", new_path);
        }

        if (S_ISDIR(statbuf.st_mode) && r_perm && x_perm) {
            // If it is dir with permissions r and x,
            // then recursively scan this folder
            scan_directory_access(new_path, uid, gid);
        }

        if (S_ISREG(statbuf.st_mode) && w_perm) {
            // If it is regular writable file
            // print this file
            printf("f %s\n", new_path);
        }

        dir = readdir(current_directory);
    }
    closedir(current_directory);
}

int main(int argc, char *argv[]) {
    char *username, *groupname, *path;

    if (parse_arguments(argc, argv, &username, &groupname, &path)) {
        fprintf(stderr, "Error parsing arguments: program should contain -u, -g and -p flags. Aborting\n");
        return 1;
    }

    struct passwd *user;
    struct group *group;

    if (!(user = getpwnam(username))) {
        fprintf(stderr, "Error detecting user with name %s. Aborting\n", username);
        return 1;
    }

    if (!(group = getgrnam(groupname))) {
        fprintf(stderr, "Error detecting group with name %s. Aborting\n", groupname);
        return 1;
    }

    // It is important to to firstly change group id
    // and only then change user id, because
    // if we do not do so, after changing user id,
    // we won't have permissions to change group id


    // set process's group id to requested group
    if (setgid(group->gr_gid)) {
        perror("setgid");
        return 1;
    }

    // set process's user id to requested user
    if (setuid(user->pw_uid)) {
        perror("setuid");
        return 1;
    }

    // start scanning
    scan_directory_access(path, user->pw_uid, group->gr_gid);
}