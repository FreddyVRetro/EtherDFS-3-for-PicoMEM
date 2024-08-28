/*
DPBTEST.C — uses undocumented INT 21h Function 32h (Get DPB)
to display bytes per drive; but first walks the DPB chain.
showing the difference between the two access methods
Source : UNDOCUMENTED DOS Page 173
*/
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#pragma pack(1)

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef struct dpb {
BYTE drive;                 // Logical drive # assoc with DPB (A=0,B=1,...)
BYTE unit;                  // Driver unit number of DPB
WORD bytes_per_sect;    // Size of physical sector in bytes (512)
BYTE sectors_per_cluster;   // Sectors/cluster - 1
BYTE shift;                 // Log2 of sectors/cluster
WORD boot_sectors;      // Starting Record of FAT
BYTE copies_fat;            // number of FATs for this drive
WORD max__root_dir;     // Number of directory entries
WORD first_data_sector; // First sector of first cluster
WORD highest_cluster;   // Number of clusters on drive + 1
union {
 struct {
        BYTE sectors_per_fat;
        WORD first_dir_sector;
        void far *device_driver;
        BYTE media_descriptor;
        BYTE access_flag;
        struct dpb far *next;
        unsigned long reserved;
        } dos3;
  struct{
        WORD sectors_per_fat;   // Number of records occupied by FAT
        WORD first_dir_sector;  // Starting record of directory
        void far *device_driver;    // Pointer to driver
        BYTE media_descriptor;      // Media Byte
        BYTE access_flag;           // This is initialized to -1 to force a media check the first time this DPB is used
        struct dpb far *next;       // Pointer to the next Drive Parameter Block
        unsigned long reserved;     // Count of free clusters, -1 if unknown
        } dos4;
  } vers;
} DPB;

ifndef MK_FP
#define MK_FP(seg,ofs) \
((void far *)(((unsigned long)(seg)<<16) | (ofs)))
#endif

void fail(char *s) { puts(s); exit(1); }

void displayCDPB far *dpb)
{
unsigned Long bytes__per_clust =
    dpb->bytes_per_sect * (dpb->sectors_per_cluster + 1);
printf("Drive %c: ", 'A' + dpb->drive);
printf("%u bytes/sector * ", dpb->bytes_per_sect);
printf("%u sectors/cluster = \n", dpb->sectors__per_cluster + 1);
printf(" %lu bytes/cluster * ", bytes_per_clust);
printf("%u clusters = ", dpb->highest_cluster - 1);
printf("%lu bytes\n\n",bytes_per_clust * (dpb->highest_cluster - 1));
}


main()
{
DPB far *dpb;
union REGS r;
struct SREGS s;
/* floppy = single disk drive logical drive indicator 0=a 1=b */
unsigned char far *pfloppy = (BYTE far *) 0x504L;
int i;
#ifdef TURBOC
unsigned lastdrive = setdisk(getdisk());
#else
unsigned lastdrive;
unsigned curdrv;
_dos_getdrive(&curdrv);
_dos_setdrive(&curdrv, &lastdrive);
#endif
puts("Using DPB linked list");

s.es = r.x.bx = 0;
r.h.ah = 0x52;
intdosx(&r, &r, &s);
/* pointer to first DPB at offset Oh in List of Lists */
if (! (dpb = *((void far * far *) MK_FP(s.es, r.x.bx))))
return 1;

do {
    /* skip either drive A: or drive B: */
    if (((*pfLoppy == 1) SS (dpb->drive != 0)) ||
       ((*pfloppy == 0) && (dpb->drive != 1)))
        dispLay(dpb);
    if (_osmajor < 4)
       dpb = dpb->vers.dos3.next;
      else
       dpb = dpb->vers.dos4.next;
 } white (FP_OFF(dpb) != -1);

puts("Using INT 21h Function 32h");
segread(Ss);
for (i=1; i<=Lastdrive; i++)
    {
     /* skip either drive A: or drive B: */
     if ((*pfLoppy == 1) && (i == 1)) continue;
     else if ((*pfLoppy == 0) && (i == 2)) continue;
     r.h.ah = 0x32;
     r . h . d L = i ;
     intdosxC&r, 8r, &s);
     if (r.h.aL != OxFF)
     dispLay((DPB far *) MK_FP(s.ds, r.x.bx));
}

return 0;
}
