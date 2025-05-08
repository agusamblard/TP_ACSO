#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode in;
    if (inode_iget(fs, inumber, &in) < 0) {
        return -1;
    }

    int sector = inode_indexlookup(fs, &in, blockNum);
    if (sector == -1) {
        return -1;
    }

    int filesize = inode_getsize(&in);
    int block_start = blockNum * DISKIMG_SECTOR_SIZE;

    if (block_start >= filesize) {
        return 0; // El bloque está fuera del archivo
    }

    int bytes_left = filesize - block_start;
    int bytes_to_read = (bytes_left < DISKIMG_SECTOR_SIZE) ? bytes_left : DISKIMG_SECTOR_SIZE;

    int err = diskimg_readsector(fs->dfd, sector, buf);
    if (err < 0) {
        return -1;
    }

    return bytes_to_read;
}
