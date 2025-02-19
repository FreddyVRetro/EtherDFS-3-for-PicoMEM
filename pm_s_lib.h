#pragma once
// Basic PicoMEM library full include, to use with any C Code

#include <stdint.h>
#include <dos.h> 
#include <i86.h>
#include <conio.h>

#define PM_ETHDFS 1
#define TEST 0   // 1 for Test Mode (No PicoMEM)

// * Status and Commands definition
#define STAT_READY         0x00  // Ready to receive a command
#define STAT_CMDINPROGRESS 0x01
#define STAT_CMDERROR      0x02
#define STAT_CMDNOTFOUND   0x03
#define STAT_INIT          0x04  // Init in Progress
#define STAT_WAITCOM       0x05  // Wait for the USB Serial to be connected

#define DEFAULT_BASE       0x2A0

#define CMD_EthDFS_Send    0x89  // Send a packet to EthDFS server emulator and wait answer

#if (PM_ETHDFS==0)
// For ETHDFS : Need to declare in the data segment
unsigned short PM_Base=0;         // PicoMEM I/O Base address
unsigned char  PM_PicoID=0;       // Pi Pico board model/ID 
unsigned char  PM_BoardID=0;      // PicoMEM Board model/ID
unsigned short PM_FW_Rev=0;       // PicoMEM firmware Revision
unsigned short BIOS_Segment=0;    // PicoMEM BIOS segment (Can be 0 if not detected)
unsigned short PM_PCCR_Param=0;   // Commands parameter RAM address (To send/Receive small data to/from command)
#endif

#ifdef __cplusplus
extern "C" {
#endif

unsigned char p_inp(unsigned short port)
{
  unsigned char value;
  _asm {
    mov dx, port
    in al, dx
    mov value, al
  }
  return value;
}

unsigned short p_inpw(unsigned short port)
{
  unsigned short value;
  _asm {
    mov dx, port
    in ax, dx
    mov value, ax
  }
  return value;
}

void p_outp(unsigned short port, unsigned char value)
{
  _asm {
    mov dx, port
    mov al, value
    out dx, al
  }
}

void p_outpw(unsigned short port, unsigned short value)
{
  _asm {
    mov dx, port
    mov ax, value
    out dx, ax
  }
}

#ifdef __cplusplus
}
#endif

bool pm_wait_cmd_end()
{
#if TEST
 return true;
#else
  while(true)
  {
    uint8_t res=p_inp(PM_Base);
    switch (res)
        {
     case STAT_READY        : return true;
     case STAT_CMDINPROGRESS: break;   // In progress, Loop
     case STAT_CMDERROR     :  // Status not used for the moment
     case STAT_CMDNOTFOUND  : //printf("CMD Error\n");
                              p_outp(PM_Base,0);  // Error : Reset and go check again the status
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


/*
bool pm_wait_cmd_end()
{
  bool r;
  _asm {
  mov dx,PM_Base
@@WaitCMDEnd:
  in ax,dx
  cmp ax,STAT_CMDINPROGRESS
  je @@pm_wait_cmd_end

  cmp ax,STAT_READY
  je @@Ok
  mov al,0   // Error > Return false
  jmp @@end

  @@Ok:
  mov al,1   // STAT_READY > Return true
  @@end:
  mov r,al
  };
  return r;  
}*/

// Send a command via I/O with argument and return a word
unsigned short pm_io_cmd(unsigned char cmd,unsigned short arg)
{
#if TEST
 return 0;
#else    
  if (pm_wait_cmd_end())
   {
    p_outpw(PM_Base+1,arg);   // Send the parameters
    p_outp(PM_Base,cmd);      // Send the command
    pm_wait_cmd_end();
    return p_inpw(PM_Base+1);
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

/*
;PM BIOS Function 4 : Get DFS infos
; Added in January 2024
; Return AL : DFS Code version  (Initial is 1)
;        CX : DFS buffer Offset
; Info : To detect if implemented, just check if BX is not changed after calling it.
*/
unsigned char pm_dfs_detect()
{
#if TEST    // Return fake PicoMEM Status
 BIOS_Segment=0xD000;
 PM_Base=0x220;
 return true;
#else
bool r;
_asm {
mov ax,0x6004
mov dx,0x1234
mov bx,0xFFFF
int 0x13
cmp bx,0xFFFF
je @@no_bios
mov DFS_Buff_Offs,bx
// al contains the DFS code version (Start from 1)
jmp @@end
@@no_bios:
mov al,0              // Return false
@@end:
mov r,al
};
return r;
#endif
}

void pm_bios_cmd(uint16_t cmd)
{
#if TEST    // Return fake PicoMEM Status
 return true;
#else
bool r;
_asm {
mov ah,0x60
mov al,cmd
mov dx,0x1234
mov bx,0xFFFF
int 0x13
};
return r;
#endif
}

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
cmp dx,0xAA55
jne @@no_bios         // > No PicoMEM BIOS
mov PM_Base,ax
mov BIOS_Segment,bx
mov al,1              // Return true
jmp @@end

@@no_bios:
mov al,0              // Return false
@@end:
mov r,al
};
return r;
#endif  
}
