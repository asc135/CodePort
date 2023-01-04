###############################################################################
#
#   Build Platform Configuration File
#
###############################################################################
#
#   2009-10-01  asc Creation.
#   2010-09-27  asc Updated platform detection.
#   2012-08-22  asc Added additional makefile config variables.
#   2012-12-11  asc Added better support for platform configuration.
#   2013-04-10  asc Added support for shared object builds.
#   2013-04-11  asc Restructured to better support platform configuration.
#   2013-12-19  asc Adapted to symlink-free CodePort configuration.
#   2021-11-04  asc Added -std=c++98 to config cflags.
#   2022-02-28  asc Switched to -std=c++20 (-std=c++2a for g++ ver <= 9).
#   2022-04-07  asc Switched to -std=c++17 and separated CPPFLAGS.
#   2022-05-22  asc Added environment variables to inject compiler and linker options.
#   2022-10-06  asc Added LOCAL_INCS and LOCAL_LIBS definitions from env vars.
#   2022-10-06  asc Switched from = to := where possible (expand at assignment).
#   2023-01-04  asc Changed CPPFLAGS to CXXFLAGS.
#
###############################################################################

#
# Target Name
#
NAME := libCodePort

#
# Library Build - 'true' or 'false'
#
LIBRARY_BUILD := true

#
# Library Type - 'static' or 'shared'
#
ifndef LIBRARY_TYPE
LIBRARY_TYPE := static
endif

#
# CodePort Library Path
#
ifndef CODEPORT
CODEPORT := .
endif

#
# CodePort Target Platform
#
ifndef PLATFORM
PLATFORM := linux
PLATFORM_COMMON := posix
endif

#
# Platform Build Settings
#
include $(CODEPORT)/platforms/$(PLATFORM)/platform.inc

#
# Local Build Settings
#
LOCAL_INCS := ${LOCAL_INCLUDES}
LOCAL_LIBS := ${LOCAL_LIBRARIES}

#
# Project Compiler and Linker Flags
#
PROJECT_CXXFLAGS := -std=c++17
PROJECT_CFLAGS := -Wall -Wextra -pedantic -Werror
PROJECT_LFLAGS :=

#
# Local Compiler and Linker Flags
#
LOCAL_CXXFLAGS := ${LOCAL_CXX_FLAGS}
LOCAL_CFLAGS := ${LOCAL_C_FLAGS}
LOCAL_LFLAGS := ${LOCAL_L_FLAGS}

#
# Source Path
#
SRC_PATH := src platforms/$(PLATFORM) platforms/$(PLATFORM_COMMON)

#
# Output Path
#
ifndef OBJ_PATH
OBJ_PATH := obj-$(shell $(BUILD_TOOL)gcc -dumpmachine)
endif

#
# Project Includes
#
PROJECT_INCS :=

#
# Project Libraries
#
PROJECT_LIBS :=

#
# Project Dependencies
#
PROJECT_DEPS :=

#
# Project Make Rules
#
PROJECT_MAKE :=
