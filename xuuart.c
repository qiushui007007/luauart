/*--- xuuart.c: 串口测试例程
gcc xuuart.c -o xu_uart

scp ./xu_uart root@192.168.2.1:/xutest/

-----------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <linux/reboot.h>

#include "xuuart.h"

#define DBG	//printf

//=== linuxlnk.c
int fd[MAX_PORTNUM];
int fd_init;
struct termios origterm;

//---------------------------------------------------------------------------
// Attempt to open a com port.  Keep the handle in ComID.
// Set the starting baud rate to 9600.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
//
// Returns: the port number if it was succesful otherwise -1
//
int OpenCOMEx(char *port_zstr)
{
   int i;
   int portnum;

   if(!fd_init)
   {
      for(i=0; i<MAX_PORTNUM; i++)
         fd[i] = 0;
      fd_init = 1;
   }

   // check to find first available handle slot
   for(portnum = 0; portnum<MAX_PORTNUM; portnum++)
   {
      if(!fd[portnum])
         break;
   }
   //DBG("OpenCOMEx-->%d, %s\n", portnum, port_zstr);
   //OWASSERT( portnum<MAX_PORTNUM, OWERROR_PORTNUM_ERROR, -1 );

   if(!OpenCOM(portnum, port_zstr))
   {
      return -1;
   }

   return portnum;
}

//---------------------------------------------------------------------------
// Attempt to open a com port.
// Set the starting baud rate to 9600.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
//
// 'port_zstr' - zero terminate port name.  For this platform
//               use format COMX where X is the port number.
//
//
// Returns: TRUE(1)  - success, COM port opened
//          FALSE(0) - failure, could not open specified port
//
int OpenCOM(int portnum, char *port_zstr)
{
   struct termios t;               // see man termios - declared as above
   int rc;

   if(!fd_init)
   {
      int i;
      for(i=0; i<MAX_PORTNUM; i++)
         fd[i] = 0;
      fd_init = 1;
   }

   //OWASSERT( portnum<MAX_PORTNUM && portnum>=0 && !fd[portnum], OWERROR_PORTNUM_ERROR, FALSE );

   fd[portnum] = open(port_zstr, O_RDWR | O_NONBLOCK);	//
   //fd[portnum] = open(port_zstr, O_RDWR | O_NONBLOCK | O_NOCTTY);
   DBG("OpneCOM-->open('%s'), fd = %d\n", port_zstr, fd[portnum]);
   if (fd[portnum] < 0)
   {
      //OWERROR(OWERROR_GET_SYSTEM_RESOURCE_FAILED);
      fprintf(stderr, "OpenCOM %s OWERROR_GET_SYSTEM_RESOURCE_FAILED!\n", port_zstr);
      return FALSE;  // changed (2.00), used to return fd;
   }
   rc = tcgetattr (fd[portnum], &t);	//虚拟机成功返回0, 但在路由器上返回-1
   DBG("OpneCOM-->tcgetatr, rc = %d\n", rc);
   if (rc < 0)
   {
      int tmp;
      tmp = errno;
      close(fd[portnum]);
      errno = tmp;
      //OWERROR(OWERROR_SYSTEM_RESOURCE_INIT_FAILED);
      fprintf(stderr, "OpenCOM %s 1:OWERROR_SYSTEM_RESOURCE_INIT_FAILED!\n", port_zstr);
      return FALSE; // changed (2.00), used to return rc;
   }
   DBG("OpneCOM-->open OK, rc = %d\n", rc);

   cfsetospeed(&t, B9600);
   cfsetispeed (&t, B9600);

   // Get terminal parameters. (2.00) removed raw
   tcgetattr(fd[portnum],&t);
   // Save original settings.
   origterm = t;

   // Set to non-canonical mode, and no RTS/CTS handshaking
   t.c_iflag &= ~(BRKINT|ICRNL|IGNCR|INLCR|INPCK|ISTRIP|IXON|IXOFF|PARMRK);
   t.c_iflag |= IGNBRK|IGNPAR;
   t.c_oflag &= ~(OPOST);
   t.c_cflag &= ~(CRTSCTS|CSIZE|HUPCL|PARENB);
   t.c_cflag |= (CLOCAL|CS8|CREAD);
   t.c_lflag &= ~(ECHO|ECHOE|ECHOK|ECHONL|ICANON|IEXTEN|ISIG);
   t.c_cc[VMIN] = 0;
   t.c_cc[VTIME] = 3;

   rc = tcsetattr(fd[portnum], TCSAFLUSH, &t);
   tcflush(fd[portnum],TCIOFLUSH);

   if (rc < 0)
   {
      int tmp;
      tmp = errno;
      close(fd[portnum]);
      errno = tmp;
      //OWERROR(OWERROR_SYSTEM_RESOURCE_INIT_FAILED);
      fprintf(stderr, "OpenCOM %s 2:OWERROR_SYSTEM_RESOURCE_INIT_FAILED!\n", port_zstr);
      return FALSE; // changed (2.00), used to return rc;
   }

   return TRUE; // changed (2.00), used to return fd;
}

//---------------------------------------------------------------------------
// flush the rx and tx buffers
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void FlushCOM(int portnum)
{
   tcflush(fd[portnum], TCIOFLUSH);
}

//--------------------------------------------------------------------------
// Set the baud rate on the com port.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number was provided to
//               OpenCOM to indicate the port number.
// 'new_baud'  - new baud rate defined as
//
void SetBaudCOM(int portnum, unsigned char new_baud)
{
   struct termios t;
   int rc;
   speed_t baud=0;

   // read the attribute structure
   rc = tcgetattr(fd[portnum], &t);
   if (rc < 0)
   {
      close(fd[portnum]);
      return;
   }

   // convert parameter to linux baud rate
   switch(new_baud)
   {
      case PARMSET_9600:
         baud = B9600;
         break;
      case PARMSET_19200:
         baud = B19200;
         break;
      case PARMSET_57600:
         baud = B57600;
         break;
      case PARMSET_115200:
         baud = B115200;
         break;
      case PARMSET_2400:
         baud = B2400;
         break;
   }

   // set baud in structure
   cfsetospeed(&t, baud);
   cfsetispeed(&t, baud);

   // change baud on port
   rc = tcsetattr(fd[portnum], TCSAFLUSH, &t);
   if (rc < 0)
      close(fd[portnum]);
}

//--------------------------------------------------------------------------
//  Description:
//     Send a break on the com port for at least 2 ms
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void BreakCOM(int portnum)
{
   int duration = 0;              // see man termios break may be
   tcsendbreak(fd[portnum], duration);     // too long
}

//---------------------------------------------------------------------------
// Closes the connection to the port.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
//
void CloseCOM(int portnum)
{
   // restore tty settings
   tcsetattr(fd[portnum], TCSAFLUSH, &origterm);
   FlushCOM(portnum);
   close(fd[portnum]);
   fd[portnum] = 0;
}

//--------------------------------------------------------------------------
// Write an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'portnum'   - number 0 to MAX_PORTNUM-1.  This number provided will
//               be used to indicate the port number desired when calling
//               all other functions in this library.
// Returns 1 for success and 0 for failure
//
int WriteCOM(int portnum, int outlen, unsigned char *outbuf)
{
   long count = outlen;
   int i = write(fd[portnum], outbuf, outlen);

#if 0
   char pcStr[100];
   array_hex(pcStr, outbuf, outlen);
   DBG("WriteCOM-->len = %d, %s\n", outlen, pcStr);
#endif

   tcdrain(fd[portnum]);
   return (i == count);
}

//--- tiemout: 参数为ms. 默认50ms
int g_timeout_ms = 50;
void timeout_put(int num)
{
	g_timeout_ms = num;
}

int timeout_get(void)
{
	return(g_timeout_ms);
}

//--------------------------------------------------------------------------
// Read an array of bytes to the COM port, verify that it was
// sent out.  Assume that baud rate has been set.
//
// 'portnum'  - number 0 to MAX_PORTNUM-1.  This number was provided to
//              OpenCOM to indicate the port number.
// 'outlen'   - number of bytes to write to COM port
// 'outbuf'   - pointer ot an array of bytes to write
//
// Returns:  TRUE(1)  - success
//           FALSE(0) - failure
//inlen:最长的接收数据长度.
int ReadCOM(int portnum, int inlen, unsigned char *inbuf)
{
   fd_set         filedescr;
   struct timeval tval;
   int            cnt=0;
   int toRead ;

   // loop to wait until each byte is available and read it
   //for (cnt = 0; cnt < inlen; cnt++)
   toRead = inlen ;
   while (toRead > 0)
   {
      // set a descriptor to wait for a character available
      FD_ZERO(&filedescr);
      FD_SET(fd[portnum],&filedescr);
      // set timeout to 50ms (changed by SJM - original had 10ms)
      tval.tv_sec = 0;
      //tval.tv_usec = 50000;	//原代码
      tval.tv_usec = timeout_get() * 1000;	//V1.0, 新修改
      //DBG("timeout: %d\n", tval.tv_usec);

      // if bytes available read or return bytes read
      if (select(fd[portnum]+1, &filedescr, NULL, NULL, &tval) != 0)
      {
        int nread ;

        nread = read(fd[portnum], &inbuf[cnt], toRead);
        if (nread < 1) return cnt;
        toRead -= nread;
        cnt += nread;
         /*if (read(fd[portnum],&inbuf[cnt],1) != 1)
            return cnt;*/
      }
      else
         return cnt;
   }

   // success, so return desired length
   return inlen;
}

//--- 以上为公用底层代码(ownet_xu用), 以下为自写代码 ----------------------------------
//以 UARTEOF 为结束符.
#define UARTEOF  0x0d
int ReadCOM_xu(int portnum, unsigned char *inbuf)
{
   fd_set filedescr;
   struct timeval tval;
   int cnt=0, nread;

	while(1) {
		// set a descriptor to wait for a character available
		FD_ZERO(&filedescr);
		FD_SET(fd[portnum],&filedescr);
		// set timeout to 50ms (changed by SJM - original had 10ms)
		tval.tv_sec = 0;
		//tval.tv_usec = 50000;	//原代码
    tval.tv_usec = timeout_get() * 1000;	//V1.0, 新修改
    DBG("timeout: %d\n", tval.tv_usec);

		// if bytes available read or return bytes read
		if (select(fd[portnum]+1, &filedescr, NULL, NULL, &tval) != 0) {
		  nread = read(fd[portnum], &inbuf[cnt], 1);
		  if (nread == 1)	{
		  	cnt++;
		  	if (inbuf[cnt] == UARTEOF) {
		  		DBG("ReadCOM_xu: Len = %d, %s\n", cnt, inbuf);
		  		return(cnt);
		  	}
		  }
		}
		//有时会接收不完整数据而退出,
		else
			return cnt;
	  }
}

//仅发送和接收, 不打开和关闭串口
int com_tx_rx(char *pcReceive, int portnum, char *pcSend, int arg_len)
{
  unsigned short usRetLen, us_len;

   // flush the buffers
   FlushCOM(portnum);

   //if (!WriteCOM(portnum, strlen(pcSend), (unsigned char*)pcSend)) {
   if (!WriteCOM(portnum, arg_len, (unsigned char*)pcSend)) {
      CloseCOM(portnum);
      return FALSE;
   }

#if 0
   usRetLen = ReadCOM_xu(portnum, pcReceive);

#else
   us_len = 100;
   usRetLen = ReadCOM(portnum, us_len, pcReceive);
#endif
   pcReceive[usRetLen] = '\0';

   return usRetLen;
}

//独立的完整读写流程
char com_tx_rx_full(char *pcReceive, char *port_zstr, char *pcSend)
{
	int portnum;
  unsigned short usRetLen, us_len;

   // attempt to open the communications port
   if ((portnum = OpenCOMEx(port_zstr)) < 0) {
      //OWERROR(OWERROR_OPENCOM_FAILED);
      fprintf(stderr, "OPenCOMEx %s Error!\n", port_zstr);
      return -1;
   }
   SetBaudCOM(portnum, PARMSET_115200);

   // send a break to reset the DS2480
   //BreakCOM(portnum);
   // delay to let line settle
   //msDelay(2);

   // flush the buffers
   FlushCOM(portnum);

   if (!WriteCOM(portnum, strlen(pcSend), (unsigned char*)pcSend)) {
      CloseCOM(portnum);
      return FALSE;
   }

#if 0
   usRetLen = ReadCOM_xu(portnum, pcReceive);

#else
   us_len = 100;
   usRetLen = ReadCOM(portnum, us_len, pcReceive);
#endif
   pcReceive[usRetLen] = '\0';

   CloseCOM(portnum);

   return usRetLen;
}

//--- 回调函数的本体.
static pfn_string_handler g_pfnUARTHandler = NULL;

char com_rx_callback(char *port_zstr, char *pcSend)
{
	int portnum;
	unsigned char readbuffer[255];
  unsigned short usLen, usRetLen;

   // attempt to open the communications port
   if ((portnum = OpenCOMEx(port_zstr)) < 0) {
      //OWERROR(OWERROR_OPENCOM_FAILED);
      fprintf(stderr, "OPenCOMEx %s Error!\n", port_zstr);
      return -1;
   }
   SetBaudCOM(portnum, PARMSET_115200);

   // send a break to reset the DS2480
   //BreakCOM(portnum);
   // delay to let line settle
   //msDelay(2);

   // flush the buffers
   FlushCOM(portnum);

   // send the timing byte
   if (!WriteCOM(portnum, strlen(pcSend), (unsigned char *)pcSend)) {
      CloseCOM(portnum);
      //OWERROR(OWERROR_WRITECOM_FAILED);
      fprintf(stderr, "WriteCOM Error!\n");
      return FALSE;
   }
   DBG("WriteCOM OK: %s\n", pcSend);

	 usLen = 100;
   while(1) {
     FlushCOM(portnum);

     //usRetLen = ReadCOM_xu(portnum, readbuffer);
     usRetLen = ReadCOM(portnum, usLen, readbuffer);
     if (usRetLen > 0) {
     	readbuffer[usRetLen] = '\0';
     	if (g_pfnUARTHandler) {
    		g_pfnUARTHandler(readbuffer, usRetLen, COM1);
    	}
    }

     if (usRetLen == 1)
     		break;
	 }

   CloseCOM(portnum);
   return 1;
}

void UARTSetHandler(pfn_string_handler pfncallback)
{
	g_pfnUARTHandler = pfncallback;
}

//---------------- End ----------------------------------------------------------
char array_hex(char *pcret, const char *s, int ByteLen)
{
  int i, cnt = 0 ;

  pcret[0] = '\0';
  //for (i=ByteLen-1; i>=0; --i)
  for (i=0; i<ByteLen; i++)
    cnt += sprintf(&pcret[cnt], "%02X", 0xFF & s[i]);

  return (s[0] != '\0') ;
}

void delay_ms(int len)
{
	struct timeval timeout;

	memset(&timeout, 0, sizeof(struct timeval));
	timeout.tv_sec	= len / 1000;						//ms -> s,
	timeout.tv_usec	= (len % 1000) * 1000;	//ms -> us, 余下的
	select(0, NULL, NULL, NULL, &timeout);
}

//--- sharp2的接收数据
void do_reboot(void)
{
	fprintf(stderr, "Rebooting ...\n");
	fflush(stderr);

	/* try regular reboot method first */
	system("/sbin/reboot");
	sleep(2);

	/* if we're still alive at this point, force the kernel to reboot */
	//syscall(SYS_reboot, LINUX_REBOOT_MAGIC1, LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, NULL);
}

//char sharp2_rx_full(char *pc_rx, char *port_zstr)
int sharp2_rx_full(float *f_data, char *pc_ret, char *port_zstr)
{
	int portnum;
  unsigned short us_len_ret, us_len, i, j;
  unsigned char pc_rx[20];
  char pc_tmp[100];
  float f_tmp;

   // attempt to open the communications port
   if ((portnum = OpenCOMEx(port_zstr)) < 0) {
      //OWERROR(OWERROR_OPENCOM_FAILED);
      fprintf(stderr, "OPenCOMEx %s Error!\n", port_zstr);
      return -1;
   }

   SetBaudCOM(portnum, PARMSET_2400);
   FlushCOM(portnum);
   timeout_put(100);	//超时时间, ms

#if 0
   us_len_ret = ReadCOM_xu(portnum, pc_rx);

#else
   us_len = 14;
   us_len_ret = ReadCOM(portnum, us_len, pc_rx);
#endif
   pc_rx[us_len_ret] = '\0';
   CloseCOM(portnum);

	 array_hex(pc_tmp, pc_rx, us_len);
   DBG("rx: %s\n", pc_tmp);
   //rx: AA0011006E7FFFAA0011006E7FFF

   //--- 解析数据
   for (i=0; i<us_len; i++) {
   	if ( (pc_rx[i] == 0xaa) && (pc_rx[i+6] == 0xff) ) {
   		unsigned char puc_tmp = (pc_rx[i+1] + pc_rx[i+2] + pc_rx[i+3] + pc_rx[i+4]) & 0xff;
   		DBG("i = %d, %d, sum = %d\n", i, pc_rx[i+5], puc_tmp);
   		if (puc_tmp == pc_rx[i+5]) {
   			f_tmp = (pc_rx[i+1] * 256 + pc_rx[i+2]) * 5;
   			f_tmp = f_tmp / 1024;
   			*f_data = f_tmp;	//返回希望数据
   			strcpy(pc_ret, pc_tmp + i*2);
   			pc_ret[14] = '\0';
   			DBG("get: %f, %s\n", *f_data, pc_ret);

				return us_len_ret;
			}
   	}
   }

   return 0;
}

#if 1	//只接收
int main(int argc, char *argv[])
{
  char pc_str[100], pc_ret[100], pc_com[20];
  int len;
  float f_data;

  if (argc != 2) {
  	fprintf(stderr, "usage: %s xu_uart /dev/ttyUSB0\n", argv[0]);
  	exit(1);
  }

  strcpy(pc_com, argv[1]);
  len = sharp2_rx_full(&f_data, pc_ret, pc_com);
  if (len > 0)
  	DBG("com_rx: %d, %f\n", len, f_data);
  else
  	DBG("com_rx: no data ???\n");

  return 1;
}

#else	//发送并接收
void main(int argc, char *argv[])
{
  char pc_str[100], pc_ret[100], pc_com[20];
  int len;

  if (argc != 3) {
  	fprintf(stderr, "usage: %s xu_uart /dev/ttyUSB0 text\n", argv[0]);
  	exit(1);
  }

  strcpy(pc_com, argv[1]);
  strcpy(pc_str, argv[2]);
  com_tx_rx_full(pc_ret, COM1WR703N, pc_str);
  array_hex(pc_str, pc_ret, len);
  DBG("com_rx: %d, %s\n", len, pc_str);
}

#endif
