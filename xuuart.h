#ifndef XUUART_H
#define XUUART_H

//#include "xucommon.h"

#define FALSE          0
#define TRUE           1	//ownet_xu.c中定义
#define __FALSE        0
#define __TRUE         1	//ownet_xu.c中定义

#define COM1LINUX "/dev/ttyS0"
#define COM2LINUX "/dev/ttyS1"
#define COM3LINUX "/dev/ttyS2"
#define COM4LINUX "/dev/ttyS3"
#define COM5LINUX "/dev/ttyS4"
#define COM6LINUX "/dev/ttyS5"
#define USBLINUX	"USB"

#define COM1WR703N "/dev/ttyATH0"

typedef enum {COM1 = 0, COM2 = 1, COM3 = 2} COM_TypeDef;
#ifndef MAX_PORTNUM
   #define MAX_PORTNUM    16
#endif
// Baud rate bits
#define PARMSET_9600                   0x00
#define PARMSET_19200                  0x02
#define PARMSET_57600                  0x04
#define PARMSET_115200                 0x06
#define PARMSET_2400                   0x08

typedef void (*pfn_string_handler)(unsigned char *pc_string, unsigned short us_len, char cID);

#if 1
int OpenCOMEx(char *port_zstr);
int OpenCOM(int portnum, char *port_zstr);
void CloseCOM(int portnum);
void FlushCOM(int portnum);
int WriteCOM(int portnum, int outlen, unsigned char *outbuf);
int ReadCOM(int portnum, int inlen, unsigned char *inbuf);
int ReadCOM_xu(int portnum, unsigned char *inbuf);
void BreakCOM(int portnum);
void SetBaudCOM(int portnum, unsigned char new_baud);

#endif

void timeout_put(int num);
int timeout_get(void);

char com_tx_rx_full(char *pcReceive, char *port_zstr, char *pcSend);
int com_tx_rx(char *pcReceive, int portnum, char *pcSend, int arg_len);

char com_rx_callback(char *port_zstr, char *pcSend);

void UARTSetHandler(pfn_string_handler pfncallback);

#endif
