/* msg\othertsr.c: THIS FILE IS AUTO-GENERATED BY GENMSG.C -- DO NOT MODIFY! */
_asm {
  push ds
  push dx
  push ax
  call getip
  S000 db 69,116,104,101,114,68,70,83,32,99,97,110,110,111,116,32
  S001 db 98,101,32,117,110,108,111,97,100,101,100,32,98,101,99,97
  S002 db 117,115,101,32,97,110,111,116,104,101,114,32,84,83,82,32
  S003 db 104,111,111,107,101,100,32,105,116,115,32,105,110,116,101,114
  S004 db 114,117,112,116,32,104,97,110,100,108,101,114,46,13,10,'$'
 getip:
  pop dx
  push cs
  pop ds
  mov ah,9h
  int 21h
  pop ax
  pop dx
  pop ds
};
