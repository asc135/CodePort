###############################################################################
#
#   Platform Configuration File
#
###############################################################################
#   2013-04-11  asc Creation.
#   2021-12-16  asc Modified for Solaris.
#   2022-06-10  asc Changed -fpic to -fPIC to support larger table size.
#   2023-02-07  asc Added additional platform variables.
###############################################################################

#
# Platform Configuration for solaris
#
STATIC_LIB_EXT = .a
SHARED_LIB_EXT = .so
PLATFORM_CXXFLAGS =
PLATFORM_CFLAGS = -pthread -fPIC
PLATFORM_LFLAGS =
PLATFORM_INCS =
PLATFORM_LIBS = -lpthread -lrt
PLATFORM_DEPS =
