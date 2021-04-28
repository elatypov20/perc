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

    return flags_found != 0b111;
}


int main(int argc, char *argv[]) {
    char *username, *groupname, *path;

    if (parse_arguments(argc, argv, &username, &groupname, &path) != 0) {
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

    // set process's effective group to requested group
    setgid(group->gr_gid);
    perror("changing group");

    // set process's effective user uid to requested user
    setuid(user->pw_uid);
    perror("changing userid");


    printf("%s %s %s\n", username, groupname, path);
    printf("%u %u\n", user->pw_gid, group->gr_gid);

}