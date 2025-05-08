
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (strcmp(pathname, "/") == 0) {
        return 1; // raíz
    }

    if (pathname[0] != '/') {
        return -1; // solo rutas absolutas
    }

    char path_copy[strlen(pathname) + 1];
    strcpy(path_copy, pathname);

    char *token = strtok(path_copy, "/");
    int current_inumber = 1; // raíz

    struct direntv6 entry;

    while (token != NULL) {
        if (directory_findname(fs, token, current_inumber, &entry) < 0) {
            return -1;
        }

        current_inumber = entry.d_inumber;
        token = strtok(NULL, "/");
    }

    return current_inumber;
}
