#!/bin/sh
#
# $Id$
#
# Copyright (C) 2004, Jon Gettler
# http://mvpmc.sourceforge.net/
#
# This script will copy the needed files out of a kernel build workarea
# for use by the dongle_build.sh script.
#

WORKAREA=$1
TARGET=$2

BOOT=${WORKAREA}/arch/ppc/boot

CFILES=${BOOT}/common/{dummy,misc-common,ns16550,relocate,string,util}.o
SFILES=${BOOT}/simple/{embed_config,head,misc-embedded}.o
KFILE=${BOOT}/images/vmlinux.gz

FILES=`sh -c "echo $CFILES $SFILES $KFILE ${BOOT}/lib/zlib.a ${BOOT}/ld.script"`

cp $FILES $TARGET
