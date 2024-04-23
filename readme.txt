Date:        2022-11-18
Project:     CodePort++ Operating System Abstraction Library
Version:     v00.49
Maintainer:  Amardeep S. Chana
Contact:     asc135@yahoo.com
Website:     https://github.com/asc135/CodePort


Build instructions:

To create a native library, set up the environment and build library by executing
from bash prompt:

% export MAKEFILE=Makefile.sys
% make

To set up a cross-compile library, the environment variable OSTYPE must be
set to the target OS type.  Platform names are subdirectories in the ./platforms
directory.  In addition, BUILD_TOOL needs to be set to identify the path and
prefix of the cross toolchain.

Known issues:

Areas that need attention or that are still unimplemented are marked with a comment
that contains "eyeballs" (.)(.) followed by the nature of the issue as understood.

***********************************************************************************************

v00.10	    2012-07-18
    Initial release.
    
v00.11      2012-08-10
    Moved classes into cp namespace.
    Miscellaneous cleanups.

v00.20      2013-02-22
    Added alternative thread function interface support.

v00.21      2013-02-26
    Removed thread looping (it proved more trouble than benefit).
    Added alternative timer function interface support.

v00.22      2013-02-28
    Propagated a return value from Thread's InvokeUserFunc() method.

v00.23      2013-03-06
    Added ability to turn off the exit sync feature of Thread.

v00.24      2013-03-08
    Fixed a length criterion in Buffer's CopyOut method.

v00.25      2013-03-19
    Tweaked buffer algorithm in StreamBase.  Refactored Udp.

v00.26      2013-03-19
    Fixed a mixup in winsock headers in cpUtil_I and cpUdp_I for mswin.

v00.27      2013-05-02
    Restructured project files.  Improved IPC components.  Added TCP support.

v00.28      2013-06-19
    IPC Broadcast support.  Stream IPC Transport.  Various fixes and improvements.

v00.29      2013-06-27
    Removed name requirement for Datums and added array access operator.

v00.30      2013-07-23
    Added better IPC message parameter handling.  Reduced Datum's memory footprint.

v00.31      2013-08-08
    Refactored insertion byte-order for stream base.

v00.32      2013-09-06
    Fixed StreamBase memory allocation.  Added option flags to Tcp.
    Removed ThreadFunc and TimerFunc.  Added clarification to Queue description.

v00.33      2013-10-01
    Added support for IPC node startup synchronization.

v00.34      2013-10-16
    Added support for IPC watchdog.  Improved mklinks.sh and rmlinks.sh.

v00.35      2013-11-07
    Changed IpcRouter transmit timeout from 10mS to 100mS.

v00.36      2013-11-15
    Added IDL serializer/deserializer.  Implemented CRC checksum in streambase.

v00.37      2013-11-15
    Fixed handling of checksum generation and validation.

v00.38      2013-11-22
    Added additional include files to platform headers.

v00.39      2013-12-18
    Implemented posix PathCreate() and added DirCreate().  Changed Tokenize()
    output to pooled vector.

v00.40      2013-12-18
    Fixed a compiler warning.  Added -Werror to force warnings to be treated as
    errors.  Consolidated string containers.  Added Lchomp() and Rchomp() functions.

v00.41      2013-12-19
    Restructured to avoid softlinks and copies for platform common files.

v00.42      2013-12-19
    Removed time delay in SemLite destructor.

v00.43      2014-03-27
    Cleared m_Valid in cp::Base destructor.  Added so_TcpNodelay support.
    Both thanks to Kevin Holmes.

v00.44      2014-03-30
    Added ReadFile() and WriteFile() methods to cpUtil.

v00.45      2014-03-30
    Check for node valid before sending IPC messages.

v00.46      2014-06-06
    Check if IPC queue is valid after object creation.

v00.47      2014-10-22
    Applied patch from K. Holmes to fix Time64() on 32-bit systems.

v00.48      2014-12-03
    Replaced polling with timed read/write in posix queue implementation.
    Cleaned up FD_CLOEXEC in cpUdp and added it to cpTcp.

v00.48.01   2021-11-04
    Updated buildconf.inc to specify compiler flag -std=c++98 due to new and
    delete's exception handling. Plan to update to latest standard soon.

v00.49      2022-11-18
    Added c++17 support, AIX and Solaris platforms, cpSubProcess, several new
    utility functions, and some refinements.

v00.50      2023-09-06
    Updated build system files, made some fixes and cleanups, added thread abort,
    and added a number of utility functions.

v00.51      2023-09-26
    Added hyphen and underscore alphanumeric string check utility function.
