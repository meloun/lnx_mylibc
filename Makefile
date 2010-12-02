# Makefile - makefile for ~/s2e
#
# Copyright (c) 2008, Nissin Systems Co., Ltd. All rights reserved.
#
# modification history
# --------------------
# 01b,15jan10,nss  fixed install permission of s2e-ttySn.conf.
# 01a,05aug08,nss  written.
#

TOP_DIR := $(shell cd ../../../; pwd)
include $(TOP_DIR)/config/config.make

-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)

OFLAGS :=-static

#LIBCGI := ../cgi/libcgi
LIBS2E := libs2e
CFLAGS += -I$(LIBCGI) -I$(LIBS2E)
OFLAGS += -L$(LIBS2E) -ls2e
LIBS += $(LIBS2E)/libs2e.a

ifneq ($(CONFIG_IPV6),)
CFLAGS	+=-DINET6
endif

INS_DIR := $(TOP_DIR)/romfs

##

HDRS = io_common.h pm_common.h
OBJS = io_common.o pm_common.o
EXES = io_common pm_common

##

all:  $(EXES)

$(EXES):  $(OBJS) $(HDRS) $(LIBS)
	$(CC) -o $(EXES) $(OBJS) $(OFLAGS) $(LDFLAGS) $(EXTRALIBS) $(LDLIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	-@rm -f $(EXES) $(EXES).gdb $(OBJS)


io_common.o		: io_common.c		$(HDRS)
#tty.o			: tty.c			$(HDRS)

