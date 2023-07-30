/**
 * @file jlink-rtt.c
 * @author shadow3d (shadowThreeD@gmail.com)
 * @brief
 * @version 0.1
 * @date 2023-07-30
 *
 * @copyright Copyright (c) 2023
 *
 * How to use
 *  1. compiler: gcc ./jlink-rtt.c -o jlink-rtt.exe
 *  2. run: ./jlink-rtt.exe
 */
#include <stdio.h>
#include <windows.h>
#include <stdint.h>

#define JLINKARM_TIF_JTAG 0
#define JLINKARM_TIF_SWD 1

#define LINKARM_RTTERMINAL_CMD_START 0

typedef char *(*open_proc)(void);
typedef void (*close_proc)(void);
typedef void (*get_tif_proc)(uint32_t *);
typedef int (*set_tif_proc)(int);
typedef int (*conn_proc)(void);
typedef int (*ctrl_proc)(uint32_t, void *);
typedef int (*get_dev_idx_proc)(const char *);
typedef void (*set_dev_proc)(int, int);
typedef int (*rtt_read_proc)(uint32_t, char *, uint32_t);

int main(void)
{
  uint32_t mask;
  int devId;
  HMODULE hDll;
  int len;
  char rxBuf[1024];
  char buf[1024];

  hDll = LoadLibrary(("./JLink_x64.dll")); // 64bit lib
  // hDll = LoadLibrary("./JLinkARM.dll");  // 32bit lib

  if (hDll != NULL)
  {
    open_proc open = (open_proc)GetProcAddress(hDll, "JLINKARM_Open");
    close_proc close = (close_proc)GetProcAddress(hDll, "JLINKARM_Close");
    get_tif_proc get_tif = (get_tif_proc)GetProcAddress(hDll, "JLINKARM_TIF_GetAvailable");
    set_tif_proc set_tif = (set_tif_proc)GetProcAddress(hDll, "JLINKARM_TIF_Select");
    conn_proc conn = (conn_proc)GetProcAddress(hDll, "JLINKARM_Connect");
    ctrl_proc ctrl = (ctrl_proc)GetProcAddress(hDll, "JLINK_RTTERMINAL_Control");
    get_dev_idx_proc get_dev_idx = (get_dev_idx_proc)GetProcAddress(hDll, "JLINKARM_DEVICE_GetIndex");
    set_dev_proc set_dev = (set_dev_proc)GetProcAddress(hDll, "JLINKARM_ConfigJTAG");
    rtt_read_proc rtt_read = (rtt_read_proc)GetProcAddress(hDll, "JLINK_RTTERMINAL_Read");

    if (open == NULL || get_tif == NULL || set_tif == NULL || conn == NULL || ctrl == NULL ||
        get_dev_idx == NULL || set_dev == NULL || rtt_read == NULL)
    {
      printf("Err: get open proc error");
      FreeLibrary(hDll);
      return -1;
    }

    if (open() != 0)
    {
      printf("open JLink error\n");
      FreeLibrary(hDll);
      return -1;
    }
    // get_tif(&mask);
    // printf("mask=%d\n", mask);
    set_tif(JLINKARM_TIF_SWD);
    devId = get_dev_idx("STM32H743II");
    printf("devId = %d\n", devId);
    set_dev(0, 0);

    if (conn() >= 0)
    {
      ctrl(LINKARM_RTTERMINAL_CMD_START, NULL);
      while (1)
      {
        len = rtt_read(0, rxBuf, 1024);
        if (len >= 0)
        {
          // printf("%s", rxBuf);
          snprintf(buf, len, "%s", rxBuf);
          printf("%s", buf);
        }
        else
        {
          printf("RTT read error, len[%d]\n", len);
        }
        Sleep(100);
      }
    }
    close();
    FreeLibrary(hDll);
  }
  else
  {
    printf("Err: load dll error\n");
  }

  return 0;
}