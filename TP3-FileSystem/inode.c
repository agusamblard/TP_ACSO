#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "inode.h"
#include "diskimg.h"

int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    if (inumber < 1) return -1;

    int inodes_per_block = DISKIMG_SECTOR_SIZE / sizeof(struct inode);
    int block_num = INODE_START_SECTOR + (inumber - 1) / inodes_per_block;
    int offset = (inumber - 1) % inodes_per_block;

    struct inode inodes[ inodes_per_block ];
    int err = diskimg_readsector(fs->dfd, block_num, &inodes);
    if (err < 0) return -1;

    *inp = inodes[offset];
    return 0;
}

int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp, int blockNum) {
    if ((inp->i_mode & ILARG) == 0) {
        // archivo chico: bloques directos
        if (blockNum < 0 || blockNum >= 8) return -1;
        return inp->i_addr[blockNum];
    }

    // archivo grande: bloques indirectos (0..1791), dobles (1792..)
    if (blockNum < 7 * 256) {
        int indir_block = inp->i_addr[blockNum / 256];
        if (indir_block == 0) return -1;

        unsigned short blocks[256];
        int err = diskimg_readsector(fs->dfd, indir_block, &blocks);
        if (err < 0) return -1;

        return blocks[blockNum % 256];
    } else {
        int doubly_indir_block = inp->i_addr[7];
        if (doubly_indir_block == 0) return -1;

        int block_index = blockNum - 7 * 256;
        int first_level_index = block_index / 256;
        int second_level_index = block_index % 256;

        unsigned short first_level[256];
        if (diskimg_readsector(fs->dfd, doubly_indir_block, &first_level) < 0) return -1;

        int second_block = first_level[first_level_index];
        if (second_block == 0) return -1;

        unsigned short second_level[256];
        if (diskimg_readsector(fs->dfd, second_block, &second_level) < 0) return -1;

        return second_level[second_level_index];
    }
}


int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
