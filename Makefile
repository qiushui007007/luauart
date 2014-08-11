OPENWRT = 1

ifeq ($(OPENWRT), 1)
	#CC = ~/OpenWrt-SDK-ar71xx-for-linux-i486-gcc-4.6-linaro_uClibc-0.9.33.2/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc
	CC = ~/OpenWrt-Toolchain-ar71xx-for-mips_r2-gcc-4.6-linaro_uClibc-0.9.33.2/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-gcc
	CFLAGS += -I ~/openwrt-lib/include -L ~/openwrt-lib/lib

else
	CC = gcc
endif

CFLAGS += -Wall -O2
#CFLAGS += -g
CFLAGS1 += -fPIC -shared		#仅so 文件需要

#可执行文件名和相关的源码文件. lua的so和bin无法同时生成.
APP_BINARY = xu_uart
SRCS += xuuart.c

#若只编译bin, 则so部分不需要
SHAREOBJ = luauart.so
SRCS += xuuart_lua.c

#以下无需改动
OBJS = $(SRCS:.c=.o)

#all: APP_FILE
all: SO_FILE

APP_FILE: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(APP_BINARY) $(LFLAGS)

SO_FILE: $(OBJS)
	$(CC) $(CFLAGS) $(CFLAGS1) $(SRCS) -o $(SHAREOBJ) $(LFLAGS1)

.PHONY: clean
clean:
	@echo "cleanning project"
	$(RM) *.a $(OBJS) *~ *.so *.lo $(APP_BINARY) $(SHAREOBJ)
	@echo "clean completed"
