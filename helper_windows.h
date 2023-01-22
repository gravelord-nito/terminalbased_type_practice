#pragma once
#include <conio.h>
#include <windows.h>
#include <stdio.h>

DWORD WINAPI keypress_callback( LPVOID args ){
  void (*callback)(unsigned char) = (void (*)(unsigned char))((void *)(args));

  while(1)
    callback(getch());

  return 0;
}

HANDLE start_listening(void (*callback)(unsigned char)){
  HANDLE thread = CreateThread(NULL, 0, keypress_callback, (void *)(callback), 0, NULL);
  return thread;
}