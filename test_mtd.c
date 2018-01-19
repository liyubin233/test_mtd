#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <linux/compiler.h>
#include <mtd/mtd-user.h>

#define PATCH_SIZE 27428

struct BSL
{
    char  booslink[PATCH_SIZE+1];  /* string buf */
};

int non_region_erase(int fd, int start, int count, int unlock)
{
    mtd_info_t meminfo;

    if (ioctl(fd, MEMGETINFO, &meminfo) == 0) {
        erase_info_t erase;
        erase.start = start;
        erase.length = meminfo.erasesize;

        for ( ; count > 0; count--) {
            printf("\rPerforming Flash Erase of length %u at offset 0x%x",
                    erase.length, erase.start);
            fflush(stdout);

            if (unlock != 0) {
                /* Unlock the sector first. */
                printf("\rPerforming Flash unlock at offset 0x%x", erase.start);
                if(ioctl(fd, MEMUNLOCK, &erase) != 0) {
                    perror("\nMTD Unlock failure");
                    close(fd);
                    return -1;
                }
            }

            if (ioctl(fd,MEMERASE,&erase) != 0) {
                perror("\nMTD Erase failure");
                close(fd);
                return -1;
            }
            erase.start += meminfo.erasesize;
        }
        printf(" done\n");

        return 0;
    }
    return -1;
}

int main(int argc, char *argv[])
{
    int fd;
    char *cmd;
    struct mtd_info_user info;   
    int regcount;
    int ret;
    int res;

    if(argc < 3) {
        printf("You must choose a device!\n");
        return -1;
    }
    cmd=argv[1];

    /* Open the device */
    if ((fd = open(argv[2], O_RDWR)) < 0) {
        fprintf(stderr,"File open error\n");
        return -1;
    } else {
        ioctl(fd,MEMGETINFO,&info);
    }
   
    if (ioctl(fd,MEMGETREGIONCOUNT,&regcount) == 0) {
        printf("regcount=%d\n",regcount); /* calc the block size */
    }


    /* erase the device */
    if (strcmp(cmd, "erase") == 0) {
        if(regcount == 0) {    
            res = non_region_erase(fd,0,(info.size/info.erasesize),0);    
            if(res < 0) {
                printf("erase err!");
                return -1;
            }
        }
        close(fd);
        printf("erase!\n");
        return 0;
    }

    /*write file to this device*/
    struct BSL *bsl_write =(struct BSL*)malloc(sizeof(struct BSL));
    if (strcmp(cmd, "write") == 0) {
        strcpy(bsl_write->booslink,argv[3]);
        ret = write(fd, bsl_write, sizeof(struct BSL) );
        if (ret < 0) {
            printf("write err!\n");
            close(fd);
            return -1;
        }
        printf("write ok!\n");
        close(fd);
        return 0;
    }

    /*read file from this device*/
    struct BSL *bsl_read =(struct BSL*)malloc(sizeof(struct BSL));
    if (strcmp(cmd, "read") == 0) {
        ret = read(fd, bsl_read, PATCH_SIZE);
        if(ret < 0) {
            printf("read err!\n");
            close(fd);
            return -1;
        }
        FILE *fp;
        fp = fopen("/data/odm.rpm", "wb+");
        if(fp == NULL) {
            printf("open file error! \n");
            return -1;
        }

        fwrite(bsl_read->booslink, PATCH_SIZE, 1, fp);
        fclose(fp);
        //printf("%s\n", bsl_read->booslink);
        printf("read ok!\n");
        close(fd);
        return 0;
    }
    close(fd);  
    return -1;
}
