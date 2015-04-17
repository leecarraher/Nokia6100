#ifndef SSPSCREENDRV_H_
#define SSPSCREENDRV_H_
#if defined( __KERNEL__ )
#   include <linux/types.h>
#else
#   include <inttypes.h>
#endif

#include <linux/ioctl.h>

#define SCREEN_MAJOR            245

// Epson S1D15G10 Command Set
#define DISON       0xaf
#define DISOFF      0xae
#define DISNOR      0xa6
#define DISINV      0xa7
#define COMSCN      0xbb
#define DISCTL      0xca
#define SLPIN       0x95
#define SLPOUT      0x94
#define PASET       0x75
#define CASET       0x15
#define DATCTL      0xbc
#define RGBSET8     0xce
#define RAMWR       0x5c
#define RAMRD       0x5d
#define PTLIN       0xa8
#define PTLOUT      0xa9
#define RMWIN       0xe0
#define RMWOUT      0xee
#define ASCSET      0xaa
#define SCSTART     0xab
#define OSCON       0xd1
#define OSCOFF      0xd2
#define PWRCTR      0x20
#define VOLCTR      0x81
#define VOLUP       0xd6
#define VOLDOWN     0xd7
#define TMPGRD      0x82
#define EPCTIN      0xcd
#define EPCOUT      0xcc
#define EPMWR       0xfc
#define EPMRD       0xfd
#define EPSRRD1     0x7c
#define EPSRRD2     0x7d
#define NOP         0x25
#define DEVICE_NAME "screen"

#define SCREEN_BAUD     (0x001 << 8)
#define DSS_9         8
#define FRF_SPI         0
#define SSE_ENABLED      (1 << 7)
#define SPO            (1 << 3)
#define SPH            (1 << 4)


#endif /*SSPSCREENDRV_H_*/
