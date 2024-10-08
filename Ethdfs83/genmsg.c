/*
 * This file is part of the EtherDFS project.
 * http://etherdfs.sourceforge.net
 *
 * Copyright (C) 2017 Mateusz Viste
 *
 * genmsg generates C files that contain assembly for outputting to screen
 * every string that etherdfs might need to output.
 *
 * The assembly part is based on this model:
 *
 *  push ds       ; save all to-be-modified registers on the stack
 *  push dx
 *  push ax
 *  call getip    ; skip the binary content below (it's my string!)
 *  S000 db 84,104,101,32,114,101,113,117,101,115,116,101,100,32,100,114
 *  S001 db 105,118,101,32,108,101,116,116,101,114,32,105,115,32,97,'$'
 * getip:
 *  pop dx        ; "read" the address following the CALL from the stack
 *  push cs       ; load DS with the value of CS (that's where my data is)
 *  pop ds
 *  mov ah,9h     ; set AH=9 (DOS function "print string")
 *  int 21h
 *  pop ax        ; restore registers to their previous values
 *  pop dx
 *  pop ds
 */

#include <stdio.h>
#include "version.h"

int genmsg(char *fname, char *msg) {
  unsigned short i;
  FILE *fd;
  fd = fopen(fname, "wb");
  if (fd == NULL) return(1);
  fprintf(fd, "/* %s: THIS FILE IS AUTO-GENERATED BY GENMSG.C -- DO NOT MODIFY! */\r\n", fname);
  fprintf(fd, "_asm {\r\n");
  fprintf(fd, "  push ds\r\n");
  fprintf(fd, "  push dx\r\n");
  fprintf(fd, "  push ax\r\n");
  fprintf(fd, "  call getip");
  /* */
  for (i = 0; msg[i] != 0; i++) {
    if ((i & 15) == 0) {
      fprintf(fd, "\r\n  S%03X db ", i >> 4);
    } else {
      fprintf(fd, ",");
    }
    fprintf(fd, "%u", msg[i]);
  }
  fprintf(fd, ",'$'\r\n");
  /* close definition */
  fprintf(fd, " getip:\r\n");
  fprintf(fd, "  pop dx\r\n");
  fprintf(fd, "  push cs\r\n");
  fprintf(fd, "  pop ds\r\n");
  fprintf(fd, "  mov ah,9h\r\n");
  fprintf(fd, "  int 21h\r\n");
  fprintf(fd, "  pop ax\r\n");
  fprintf(fd, "  pop dx\r\n");
  fprintf(fd, "  pop ds\r\n");
  fprintf(fd, "};\r\n");
  fclose(fd);
  return(0);
}

int main(void) {
  int r = 0;

  r += genmsg("msg\\help.c",
    "EtherDFS v" PVER " / Copyright (C) " PDATE " Mateusz Viste\r\n"
    "A network drive for DOS, running over raw ethernet\r\n"
    "\r\n"
    "Usage: etherdfs SRVMAC rdrv-ldrv [rdrv2-ldrv2 ...] [options]\r\n"
    "       etherdfs /u\r\n"
    "\r\n"
    "Options:\r\n"
    "  /p=XX   use packet driver at interrupt XX (autodetect otherwise)\r\n"
    "  /n      disable EtherDFS checksums\r\n"
    "  /q      quiet mode (print nothing if loaded/unloaded successfully)\r\n"
    "  /u      unload EtherDFS from memory\r\n"
    "\r\n"
    "Use '::' as SRVMAC for server auto-discovery.\r\n"
    "\r\n"
    "Examples:  etherdfs 6d:4f:4a:4d:49:52 C-F /q\r\n"
    "           etherdfs :: C-X D-Y E-Z /p=6F\r\n"
    );

  r += genmsg("msg\\unsupdos.c", "Unsupported DOS version! EtherDFS requires MS-DOS 5+.\r\n");

  r += genmsg("msg\\noredir.c", "Redirector installation has been forbidden either by DOS or another process.\r\n");

  r += genmsg("msg\\alrload.c", "EtherDFS is already installed and cannot be loaded twice.\r\n");

  r += genmsg("msg\\notload.c", "EtherDFS is not loaded, so it cannot be unloaded.\r\n");

  r += genmsg("msg\\tsrcomfa.c", "Communication with the TSR failed.\r\n");

  r += genmsg("msg\\nomultpx.c", "Failed to find an available INT 2F multiplex id.\r\nYou may have loaded too many TSRs already.\r\n");

  r += genmsg("msg\\othertsr.c", "EtherDFS cannot be unloaded because another TSR hooked its interrupt handler.\r\n");

  r += genmsg("msg\\unloaded.c", "EtherDFS unloaded successfully.\r\n");

  r += genmsg("msg\\mapfail.c",
    "Unable to activate the local drive mapping. You are either using an\r\n"
    "unsupported operating system, or your LASTDRIVE directive does not permit\r\n"
    "to define the requested drive letter (try LASTDRIVE=Z in your CONFIG.SYS).\r\n"
  );

  r += genmsg("msg\\drvactiv.c",
    "The requested local drive letter is already in use. Please choose another\r\n"
    "drive letter.\r\n"
  );

  r += genmsg("msg\\memfail.c", "Memory alloc error!\r\n");

  r += genmsg("msg\\relfail.c", "DS/SS relocation failed.\r\n");

  r += genmsg("msg\\pktdfail.c", "Packet driver initialization failed.\r\n");

  r += genmsg("msg\\nosrvfnd.c", "No EtherSRV server found on the LAN (not for requested drive at least).\r\n");

  r += genmsg("msg\\instlled.c", "EtherDFS v" PVER " installed (local MAC ");

  r += genmsg("msg\\pktdrvat.c", ", pktdrvr at INT ");

  if (r != 0) puts("genmsg: at least one error occured when compiling messages");

  return(r);
}
