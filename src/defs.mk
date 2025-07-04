# This file was generated by configure. Edits will disappear if you rerun
# configure. If you find that you need to edit this file to make things
# work, let the course staff know and we'll try to fix the configure script.
#
# The purpose of this file is to hold all the makefile definitions
# needed to adjust the OS/161 build process to any particular
# environment. If I've done it right, all you need to do is rerun the
# configure script and make clean if you start working on a different
# host OS. If I've done it mostly right, you may need to edit this
# file but you still hopefully won't need to edit any of the
# makefiles.
#
# The things that can be set here are documented in mk/os161.config.mk.
#

OSTREE=/home/chosangwon20210591/os161/root
PLATFORM=sys161
MACHINE=mips
COMPAT_CFLAGS= -DNEED_NTOHLL
COMPAT_TARGETS=
HOST_CFLAGS+= -DDECLARE_NTOHLL
