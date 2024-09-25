#pragma once
// Basic PicoMEM library full include, to use with any C Code

#define TEST 1   // 1 for Test Mode (No PicoMEM)

// * Status and Commands definition
#define STAT_READY         0x00  // Ready to receive a command
#define STAT_CMDINPROGRESS 0x01
#define STAT_CMDERROR      0x02
#define STAT_CMDNOTFOUND   0x03
#define STAT_INIT          0x04  // Init in Progress
#define STAT_WAITCOM       0x05  // Wait for the USB Serial to be connected

#define DEFAULT_BASE 0x2A0

uint16_t PM_Base=0;
uint16_t BIOS_Segment=0;


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
uint16_t pm_io_cmd(uint8_t cmd,uint16_t arg)
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
extern bool pm_irq_detect()
{
#if TEST    // Return fake PicoMEM Status
 BIOS_Segment=0xD000;
 PM_Base=0x220;
 return true;
#else
bool r;
_asm {
mov ah,0x60
mov al,0
mov dx,0x1234
int 0x13
mov PM_Base,ax
/*inc ax
mov PM_DataL,ax
inc ax
mov PM_DataH,ax */
mov BIOS_Segment,bx
/* mov PM_DeviceMask,cx */
mov al,1              // Return true
cmp dx,0xAA55
je @@end
mov al,0              // Return false
@@end:
mov r,al
};
return r;
#endif
}