###############################################################################
#
#   Platform Configuration File
#
###############################################################################
#   2013-04-11  asc Creation.
#   2021-12-16  asc Modified for AIX.
#   2023-02-07  asc Added additional platform variables.
###############################################################################

#
# Platform Configuration for aix
#
STATIC_LIB_EXT = .a
SHARED_LIB_EXT = .so
PLATFORM_CXXFLAGS =
PLATFORM_CFLAGS = -pthread -fpic
PLATFORM_LFLAGS =
PLATFORM_INCS =
PLATFORM_LIBS = -lpthread -lrt
PLATFORM_DEPS =
