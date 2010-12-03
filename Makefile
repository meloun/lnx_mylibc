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

ifneq ($(CONFIG_IPV6),)
CFLAGS	+=-DINET6
endif

INS_DIR := $(TOP_DIR)/romfs

##

HDRS = s2e_common.h io_common.h pm_common.h
OBJS = s2e_common.o io_common.o pm_common.o

##

all:  $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	-@rm -f $(EXES) $(EXES).gdb $(OBJS)

