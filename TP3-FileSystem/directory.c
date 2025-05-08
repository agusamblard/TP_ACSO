#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include "direntv6.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define DIRNAMELEN 14  // Definimos explícitamente el largo del nombre

int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt) {
    int blockNum = 0;
    char block[DISKIMG_SECTOR_SIZE];

    while (1) {
        int nbytes = file_getblock(fs, dirinumber, blockNum, block);
        if (nbytes == -1) return -1;
        if (nbytes == 0) break; // fin del archivo

        int nentries = nbytes / sizeof(struct direntv6);
        struct direntv6 *entry = (struct direntv6 *) block;

        for (int i = 0; i < nentries; i++) {
            if (entry[i].d_inumber == 0) continue;

            // Copiamos y aseguramos terminación nula del nombre
            char filename[DIRNAMELEN + 1];
            strncpy(filename, entry[i].d_name, DIRNAMELEN);
            filename[DIRNAMELEN] = '\0';

            if (strncmp(name, filename, DIRNAMELEN) == 0) {
                *dirEnt = entry[i];
                return 0;
            }
        }

        blockNum++;
    }

    return -1; // no se encontró el nombre
}
