#pragma once
// Basic PicoMEM library full include, to use with any C Code

#define PM_ETHDFS 1
#define TEST 1   // 1 for Test Mode (No PicoMEM)

// * Status and Commands definition
#define STAT_READY         0x00  // Ready to receive a command
#define STAT_CMDINPROGRESS 0x01
#define STAT_CMDERROR      0x02
#define STAT_CMDNOTFOUND   0x03
#define STAT_INIT          0x04  // Init in Progress
#define STAT_WAITCOM       0x05  // Wait for the USB Serial to be connected

#define DEFAULT_BASE 0x2A0

#if (PM_ETHDFS==0)
// For ETHDFS : Need to declare in the data segment
unsigned short PM_Base=0;         // PicoMEM I/O Base address
unsigned char  PM_PicoID=0;       // Pi Pico board model/ID 
unsigned char  PM_BoardID=0;      // PicoMEM Board model/ID
unsigned short PM_FW_Rev=0;       // PicoMEM firmware Revision
unsigned short BIOS_Segment=0;    // PicoMEM BIOS segment (Can be 0 if not detected)
unsigned short PM_PCCR_Param=0;   // Commands parameter RAM address (To send/Receive small data to/from command)
#endif


bool pm_wait_cmd_end()
{
#if TEST
 return true;
#else
  while(true)
  {
    uint8_t res=inp(PM_Base);
    switch (res)
        {
     case STAT_READY        : return true;
     case STAT_CMDINPROGRESS: break;   // In progress, Loop
     case STAT_CMDERROR     :  // Status not used for the moment
     case STAT_CMDNOTFOUND  : //printf("CMD Error\n");
                              outp(PM_Base,0);  // Error : Reset and go check again the status
                              break;
     case STAT_INIT         :
     case STAT_WAITCOM      : //printf("Err: PicoMEM Init/Wait\n");
                              return false;
     default                : //printf("Err: Invalid CMD Status (%X)\n",res);
                              return false;
        }
  }
#endif  
}

// Send a command via I/O with argument and return a word
unsigned short pm_io_cmd(unsigned char cmd,unsigned short arg)
{
#if TEST
 return 0;
#else    
  if (pm_wait_cmd_end())
   {
    outpw(PM_Base+1,arg);   // Send the parameters
    outp(PM_Base,cmd);      // Send the command
    pm_wait_cmd_end();
    return inpw(PM_Base+1);
   }     
  return 0;
#endif  
}

/*
;PM BIOS Function 0 : Detect the PicoMEM BIOS and return config  > To use by PMEMM, PMMOUSE ...
;                     Also redirect the Picomem Hardware IRQ if not done
; Input : AH=60h AL=00h
;         DX=1234h
; Return: AX : Base Port
;         BX : BIOS Segment
;         CX : Available devices Bit Mask
;             * Bit 0 : PSRAM Available
;			  * Bit 1 : uSD Available
;			  * Bit 2 : USB Host Enabled
;			  * Bit 3 : Wifi Enabled
;         DX : AA55h (Means Ok)
*/
bool pm_irq_detect()
{
#if TEST    // Return fake PicoMEM Status
 BIOS_Segment=0xD000;
 PM_Base=0x220;
 return true;
#else
bool r;
_asm {
mov ax,0x6000
mov dx,0x1234
int 0x13
mov PM_Base,ax
#if (PM_ETHDFS==0)
inc ax
mov PM_DataL,ax
inc ax
mov PM_DataH,ax
#endif
mov BIOS_Segment,bx
/* mov PM_DeviceMask,cx */
cmp dx,0xAA55
jne @@no_bios
/*PM BIOS Function 3 : Return the PicoMEM Board ID and BIOS RAM Offset
; Added in Sept 2024
; Return AH : Pi Pico Board / chip ID
;        AL : PicoMEM Board ID
;        BX : Firmware Revision
;        CX : PCCR_Param (Offset of commands response)
;        DX : Reserved for the future */
mov ax,0x6003
mov dx,0x1234
mov cx,0xFFFF         //For BIOS fonction detect
int 0x13
// If CX is still 0xFFFF the BIOS Does not support this fonction.
cmp cx,0FFFF
je @no_bios_3
mov PM_BoardID,al     // Collect the different infos
mov PM_PicoID,ah
mov PM_PCCR_Param,cx
@no_bios_3:           // All the linked variables remains at 0
mov al,1              // Return true
@@no_bios:
mov al,0              // Return false
@@end:
mov r,al
};
return r;
#endif
}
