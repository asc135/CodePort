###############################################################################
#
#   Platform Configuration File
#
###############################################################################
#   2013-04-11  asc Creation.
#   2021-12-16  asc Modified for Solaris.
###############################################################################

#
# Platform Configuration for Solaris
#
STATIC_LIB_EXT = .a
SHARED_LIB_EXT = .so
PLATFORM_CFLAGS = -pthread -fpic
PLATFORM_LFLAGS =
PLATFORM_LIBS = -lpthread -lrt
