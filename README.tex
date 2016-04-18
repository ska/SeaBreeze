/*!
\file README.txt

This file is formatted with Doxygen markup so that it can be parsed and
rendered directly by "make doc" into our Word-formatted user manuals
and HTML online documentation.

\mainpage SeaBreeze API

\tableofcontents

\section overview Overview

SeaBreeze is a minimalistic, multi-platform (Windows, Linux, MacOS) device driver
for Ocean Optics spectrometers, designed specifically for embedded applications
needing to run in resource-constrained hardware environments.  SeaBreeze provides
a fully working and tested <b>REFERENCE IMPLEMENTATION</b> of the Ocean Optics USB
interface, demonstrating how Ocean Optics spectrometers can be commanded and
controlled from C/C++.   It is provided with full C/C++ source code so that
customers can customize and extend it to support exactly those features and
functions they require.

Unlike the larger OmniDriver/SPAM driver library, SeaBreeze itself does not
contain advanced spectroscopic processing and manipulation, but full sample code
is included showing how these functions can be implemented in client applications
using C++, C#, and a variety of other languages.

\section license License

SeaBreeze is licensed under the MIT License.  Additional information may be found 
in the "LICENSE" file which should accompany this source distribution.

(Also available at http://opensource.org/licenses/MIT)

\section apis Application Programming Interfaces (APIs)

SeaBreeze provides two distinct interfaces to control spectrometers.  You are free
to use and extend either interface, but note that most new development and support
from Ocean Optics will focus on the newer SeaBreezeAPI interface, in preference to
the older legacy SeaBreezeWrapper interface.

- SeaBreeze 2.0: SeaBreezeAPI (C++ class) or SeaBreezeAPI.h (C functions)
- SeaBreeze 1.0: SeaBreezeWrapper (C++ class) or SeaBreezeWrapper.h (C functions)

Note that detailed documentation for the C++ methods can be found in the corresponding
C functions.

Regrettably, at writing the SeaBreezeAPI (2.0) can only be used from a full SeaBreeze
source distribution.  Its headers are not designed in such a way that SeaBreezeAPI
can be called without access to the full SeaBreeze include tree, which currently is
not provided through the binary installers.

\section distribution Distribution Contents

\verbatim
    SeaBreeze/       The driver and key components
        doc/         Documentation relating to SeaBreeze and its API
        include/     headers for building SeaBreeze
          api/       exportable headers for client applications
        os-support/  helpers for specific operating systems
          linux/     provides udev rules allowing non-root users to claim devices
          windows/   provides working Visual Studio 2005, 2010 and 2012 solutions
        src/         core SeaBreeze source for all operating systems
        test/        command-line tests, including seabreeze-util cmd-line utility
        sample-code/ "recipes" demonstrating how to call SeaBreeze for common tasks
    daemons/         server daemons layered atop SeaBreeze for remote / concurrent access
    util/            miscellaneous programs related to spectroscopy which may or may not 
                     require SeaBreeze
\endverbatim

\section docs Generated Documentation

SeaBreeze documentation is now maintained in Doxygen format, and can be
rendered as HTML, RTF (MS Word), Unix 'man' pages, or other styles.  For
convenience, pre-rendered documentation is generated for each customer
release (RTF, converted to Microsoft .docx) which may be found in the
./doc directory.

Assuming you have the "doxygen" command installed (available free at
http://doxygen.org -- optionally with GraphViz for UML), you can generate
documentation automatically in HTML, RTF, and 'man' formats by typing:

\verbatim
    $ make doc
\endverbatim

Open doc/html/index.html in a browser (or rtf/refman.rtf in Word) to navigate
the results.  Note that RTF fields won't self-populate until you "Select All"
then "Update Fields" (F9).

\section build Building SeaBreeze

<i>If you did not receive the SeaBreeze source code, or already have a pre-compiled
SetUp.msi installer, you may skip this section.</i>

- \ref build_win
- \ref build_linux
- \ref build_macos
- \ref build_mingw

\subsection build_win Windows

SeaBreeze is normally built under Windows using Visual Studio 2010, although
we've provided working solution and project directories for 2005, 2010, 2012,
and 2013 in the os-support/windows directory.

Dependencies
- Visual Studio
- <a href="http://www.microsoft.com/en-us/download/details.aspx?id=11800">Microsoft WinDDK</a> (7600.16385.1 recommended)
- (Note that .NET is not required)
- Visual Studio 2013 requires the <a href="https://visualstudiogallery.msdn.microsoft.com/9abe329c-9bba-44a1-be59-0fbf6151054d">Installer Projects extension</a>

\subsubsection visual_studio Visual Studio

- Open os-support\windows\VisualStudio2010\SeaBreeze.sln
- Build (F7)

\subsubsection cygwin Cygwin

The following should work on most Cygwin environments, assuming Visual Studio is installed:
\verbatim
    $ cd seabreeze
    $ export VISUALSTUDIO_PROJ=VisualStudio2010    (or 2005 or 2012)
    $ make
\endverbatim

Note that this is simply using the Cygwin bash shell and GNU toolchain (make) for automation;
the actual compiler invoked is Visual Studio.  At this time, we have not found a way to support
native Cygwin GCC or MinGW (submissions/solutions welcome!)

\subsubsection errors Common Build or Runtime Errors

\verbatim
Error: NativeUSBWinUSB.c(26): fatal error C1083: Cannot open include file: 'Winusb.h'
Fix:   add os-support\windows\WinDDK_Includes to your configuration's include path
       (SeaBreeze->References->Configuration Properties->VC++ Directories->Include)

Error: fatal error LNK1104: cannot open file 'winusb.lib'
Fix:   Add C:\WinDDK\nnn\lib\$(OS)\$(ARCH) to your configuration's "Library Directories"
       (same process as above).

Error: fatal error C1083: Cannot open include file: 'api/SeaBreezeWrapper.h' (etc)
Fix:   add seabreeze\include to your configuration's include path with
       (Solution 'SeaBreeze' -> Project 'SeaBreeze' -> Properties -> Configuration
        Properties -> C/C++ -> General -> "Additional Include Directories")

Error: Cannot open .sln file
Fix:   Our sources are normally built and tested using Microsoft Visual Studio
       2010. If you require support for other compilers, please let us know and we
       will endeavor to provide appropriate configuration files.
       
Error: LINK : fatal error LNK1123: failure during conversion to COFF: file invalid or corrupt
Fix:   Uninstall .NET 4.5.1 and re-install .NET 4.0
       (cf http://stackoverflow.com/questions/10888391, 6626397, etc)

Error: Runtime: An unhandled exception of type 'System.BadImageFormatException' occured in CSharpDemo.exe
Fix:   This nearly always means that SeaBreeze.dll was compiled in 32-bit mode and linked to 
       a 64-bit CSharpDemo.exe, or vice-versa.  Please ensure that both the library and client 
       application are compiled to the same target and try again.
\endverbatim

\subsection build_linux Linux

Dependencies
- libusb-dev 0.1
- gcc
- g++

To compile SeaBreeze, simply run 'make' on the command line of a POSIX
system.  SeaBreeze is a C++ driver libary with a simplified C interface.  This
will build with a combination of g++ and gcc.  At the moment, only Linux
is fully supported, and requires at least a 2.4.20 kernel for USB support.

Building SeaBreeze requires that libusb-0.1 is installed (with its shared libraries
in /usr/lib and its header files in /usr/include (e.g. usb.h)).  It is also
recommended that the target system have the Ocean Optics rules file for udev
(10-oceanoptics.rules) so that ordinary users can access the devices.  This is
likely the problem if root can connect to devices but nobody else can.

It is necessary to put libseabreeze.so into your library path to run any
programs against this driver.  It should suffice to do this within the
SeaBreeze root directory (where this README.txt is) for testing:

\verbatim
    $ export LD_LIBRARY_PATH="$PWD/lib"
\endverbatim

Alternately, libseabreeze.so could be installed into a system library directory
like /usr/local/lib that ld.so knows about.

Test programs in the 'test' directory should be built alongside SeaBreeze and
can be used as starting points for new development.  As long as the
LD_LIBRARY_PATH above is properly defined, these should work.  If they do not,
then they may need to be updated to reflect the current state of the driver
API.

\subsection build_macos MacOS

Dependencies
- MacOS 6.5 or higher (normally tested with 6.8)
- gcc / g++ (comes with <a href="https://developer.apple.com/xcode">XCode</a>)

Basically, you should be able to follow the \ref build_linux instructions (i.e. \c make).

\subsection build_mingw MinGW

The following was tested with an msys environment using 64bit (mingw-w64) gcc (4.8.2).

Dependencies
- libusb-win32 1.2.6 (https://sourceforge.net/projects/libusb-win32/files/)

With respect to the msys tree:
1. copy libusb0.dll into /local/lib 
2. copy libusb0_usb.h into /local/include 

To use Ocean spectrometers via libusb on Windows, you can create libusb0-based 
drivers using Zadig (http://zadig.akeo.ie/).

\section install Installing SeaBreeze

- \ref install_win
- \ref install_linux
- \ref install_macos

\subsection install_win Windows

Simply double-click SeaBreeze-vX.Y-Setup32.msi (or *64.msi), which should
install:

\verbatim
    SeaBreeze.dll  -> C:\Windows\System32
    *.INF          -> C:\Program Files\Ocean Optics\SeaBreeze\Drivers
    *.h            -> C:\Program Files\Ocean Optics\SeaBreeze\API
    *.lib          -> C:\Program Files\Ocean Optics\SeaBreeze\Library
\endverbatim

Note that the driver installation process won't be complete until you
physically insert an Ocean Optics spectrometer into your computer's USB
port.  At that time, the New Hardware Wizard should pop-up and ask how
you would like to locate an appropriate device driver.  The recommended
process is:

1.  Insert spectrometer into available USB port
2.  Wait for New Hardware Wizard to come up
3.  On "Can Windows connect to Windows Update...?"
    select "No, not this time"
4.  On "What do you want the wizard to do?"
    select "Install from a list or specific location (Advanced)"
5.  Click "Search for the best driver in these locations."
6.  Uncheck "Search removable media"
7.  Click "Include this location in the search:"
8.  Browse to "C:\Program Files\Ocean Optics\SeaBreeze\Drivers"
9.  Click Next
10. Click Finish

If the New Hardware Wizard doesn't come up on its own, you can use the
following procedure instead:

1. From the Start Menu, right-click on Computer, then select "Manage"
2. Select the Device Manager
3. Select "Ocean Optics Spectrometers"
4. Right-click on the desired spectrometer and select "Update Driver" 
5. Select "Browse my computer" (not "Search automatically")
6. Ensure "include subfolders" is checked
7. Browse to C:/Program Files/Ocean Optics/SeaBreeze/Drivers and click "Okay"

You will know that the drivers have been installed correctly if the Device
Manager adds the string "(WinUSB)" at the end of the spectrometer name.

Finally, you can "pre-load" the drivers using Microsoft's 
<a href="http://msdn.microsoft.com/en-us/library/windows/hardware/ff544775%28v=vs.85%29.aspx">dpinst.exe</a>
utility (included), where $ARCH is i386 or amd64 as 
appropriate (see 'dpinst /?' for other options):

\verbatim
  C:> cd /Program Files/Ocean Optics/SeaBreeze/Drivers
  C:> $ARCH/dpinst.exe /q /lm /c
\endverbatim

\subsection install_linux Linux

For Linux computers to recognize Ocean Optics spectrometers and allow non-root
users to claim and control the devices, you'll need to install the provided
"10-oceanoptics.rules" file, found under os-support/linux, copying it to
(usually) /etc/udev/rules.d.  Note that older versions of udev use the "SYSFS"
rule nomenclature; the provided file uses the newer "ATTR" standard.

\subsection install_macos MacOS

\todo Document MacOS installation

\section test Testing SeaBreeze

A number of test programs and sample-code demonstrations are provided with
SeaBreeze.  Note that many of them (the VisualCppConsoleDemo for instance)
are intended for use with a locally-built copy of SeaBreeze, meaning that 
you're expected to compile SeaBreeze first.  Other programs (the CSharpDemo)
should work with a "runtime / redistributable" copy of SeaBreeze installed
through .msi installers.  However, the expectation remains that users would
normally compile SeaBreeze locally.

- \ref test_win
- \ref test_linux
- \ref test_macos

\subsection test_win Windows

To ensure that the device driver has been installed correctly:

 1. Open the Windows Device Manager (Start->Run->compmgmt.msc, or
    Computer->Manage)
 2. Open "Ocean Optics Devices"
 3. Spectrometer->Properties and confirm Device Status includes
    "This device is working properly"
 4. Select the Details Tab, then select "Service" and confirm
    the value is "WinUSB"

To test SeaBreeze from a command-line environment, try building and running
the provided tests and sample_code files:

\verbatim
  $ cd test
  $ make
  $ ./seabreeze-util --help
\endverbatim

To test SeaBreeze from a Windows GUI, build and run the "CSharpDemo" provided
in sample-code.

To debug SeaBreeze.dll from the "CSharpDemo", edit "SeaBreezeWrapper.cs" to
load your debug build of SeaBreeze.dll, then change the following line in 
"SeaBreezeWrapper.cs":

    const string DLL = @"SeaBreeze.dll";

To the path of your debug SeaBreeze.dll, i.e.:

    const string DLL = @"C:\Code\seabreeze-code\trunk\SeaBreeze\os-support\windows\VisualStudio2013\x64\Debug\SeaBreeze.dll";

\subsection test_linux Linux

\verbatim
    $ make new
    $ export LD_LIBRARY_PATH="$PWD/lib"
    $ ./test/seabreeze_test_posix
\endverbatim

\subsection test_macos MacOS

\verbatim
    $ make new
    $ export DYLD_FALLBACK_FRAMEWORK_PATH="$PWD/lib"
    $ export DYLD_LIBRARY_PATH="$PWD/lib"
    $ test/seabreeze_test_posix
\endverbatim

\section samples Sample Programs

\subsection sample_c C/C++

For examples of how to call the newer SeaBreezeAPI.h API from C, please see
test/api_test.c.

For examples of how to call the older SeaBreezeWrapper.h API from C, please see:
- test/seabreeze_test_posix.c
- test/seabreeze_test_windows.c
- sample-code/c/*.c

\subsection sample_csharp C#

When developing a Visual C# program that uses SeaBreeze, in the application
Project, use "Add Existing" to add API\SeaBreezeWrapper.cs to the project.

See sample-code/CSharpDemo for a working example.

\subsection windows Windows Notes

Note the command-line tests are normally built from Cygwin (Windows), Ubuntu or
CentOS (Linux), and MacOS 10.6+.  Future releases may change from Cygwin to
MinGW for better 64-bit support.

When developing a Visual C++ program that uses SeaBreeze, in the application
Project, use "Add Existing" to add Library\SeaBreeze.lib to the Project
Source, as well as API\SeaBreezeWrapper.h.

An application built with SeaBreeze can access the shared DLL in the
Windows/System32 folder. The DLL can alternatively be co-located in your
application directory.

\section bitness 32-bit vs 64-bit

SeaBreeze has been successfully tested under a variety of 32-bit and 64-bit
environments, yet it is not guaranteed that any one particular release of
SeaBreeze will definitely build out-of-the-box on a specific platform,
without some tweaking of configuration settings.

While we are working to make the installation and linking process as
automatic and trouble-free on a wide variety of platforms, we don't yet
have an automated regression testing framework in place to rigorously
check every release against every compiler in every context.

If you have trouble building, linking, or running from a particular
environment, please let us know and we will endeavor to include your
use-case in our test procedures.

\section threading Threading

SeaBreeze is believed to be mostly thread-safe at the level of
SeaBreezeWrapper (the public API).  While SeaBreeze does not currently
create or utilize any threads internally, you should be able to safely
make SeaBreeze calls from different threads.  Fully tested support for
simultaneous access of multiple spectrometers is forthcoming.

If you wish to place a "timeout" on a call to SeaBreeze, such as
seabreeze_get_formatted_spectrum(), you can do so using the multi-
threaded timeout support provided by your host language.  For example,
a simple C program using POSIX threads to enforce a timeout on SeaBreeze
calls can be found in sample_code/c/demo-timeout.c

A multi-threaded C# demo is also available in sample_code/CSharpDemo.

\section support Support

For support with SeaBreeze issues, please contact oem@oceanoptics.com.

\section issues Known Issues

\subsection vst_modules VST Module Conflicts

Under Ubuntu 10.04 LTS and other Linux versions, you may have trouble claiming
certain spectrometers, including the USB4000, due to a conflict with the vstmod
kernel module. Try running "sudo rmmod vstmod", then re-running your SeaBreeze
application.

A similar problem was found between HR4000, Debian 6, and the "vstusb" module.
Again, adding "blacklist vstusb" to /etc/modprobe.d/blacklist.conf and rebooting
resolved the issue. 

\subsection toshiba USB4000 and Flame-T on libusb

Some versions of libusb, including that used by some Linux distributions and
the latest release of MacOS (El Capitan 10.11.3) lead to very slow enumeration of
Toshiba-based spectrometers (the USB4000 and Flame-T).  This is being investigated.

\section backlog Product Backlog

\subsection backlog_high High

- deprecate SeaBreezeWrapper
- update number of user-configurable EEPROM slots from the MAX_COEFFS constants
  in the appropriate device firmware
- EDC pixels for Flame-S and Flame-T need updated to distinguish from USB2000+/USB4000
  via USB Descriptor

\subsection backlog_med Medium

- provide more Visual Basic samples
- provide more Visual C++ samples
- fix Cygwin on Win64
- add Spark *.inf files to other Visual Studio installer projects besides 2010
- enable SeaBreezeAPI from binary-only distribution (provide missing headers, etc)

\subsection backlog_low Low

- [oem-2306] SeaBreezeWrapper::getInstance() returns before fully initialized
- support MinGW if possible
- provide bindings for Python, Ruby, Perl, etc
- generate Feature Matrix showing which features are (N)ot-possible, (P)ossible,
  (I)mplemented or (W)orkaround-available for each spectrometer, for both 
  SeaBreezeWrapper and SeaBreezeAPI.

\section history Appendix A: Version History

\date 2016-04-05 (version 3.0.11)
- fixed NIRQuest EEPROM slot count (was 18, now 20)
- increased USB2000+/Flame-S EEPROM slot count (was 17, now 30)
- added support for Jaz and Blaze over ethernet
- added Flame-NIR

\date 2015-09-16 (version 3.0.10)
- added Spark (tested under MacOS, VS2010)

\date 2015-09-14 (version 3.0.9)
- fixed Maya under Linux
- fixed build under Windows

\date 2015-08-24 (version 3.0.8)
- updated EDC pixel indices for Flame-S/T and USB2000+/4000
- tweaked order of post-processing operations in CSharpDemo
- fixed internationalization in EEPROM reads
- added setTriggerMode to CSharpDemo
- added --eeprom-value-ascii in seabreeze-util
- added read temperature feature
- added read the optical bench parameters feature
- added read firmware and hardware revisions feature
- added read serial numbers features
- added a spectrum protocol feature that sets and reads scans to average and boxcar filter
- renamed wavecal to WaveCalCoeffsEEProm to better describe its functionality
- revised test_api to include the newly added functionality

\date 2015-05-22 (version 3.0.7)
- added seabreeze_set_logfile per customer request
- added EXTERNAL_SYNCHRONIZED trigger mode to Maya2000Pro and Maya-LSL

\date 2015-01-24 (version 3.0.6)
- Added MinGW patch and instructions from Andras Barta

\date 2015-01-23 (version 3.0.5)
- Fixed EDC pixel indices for QE65000 and QE65Pro

\date 2015-01-16 (version 3.0.4)
- Removed phantom shutter feature from STS

\date 2015-01-16 (version 3.0.3)
- Enabled continuous strobe for Maya2000Pro and Maya-LSL

\date 2015-01-15 (version 3.0.2)
- Added Maya-LSL

\date 2014-10-01 (version 3.0.1)
- Fixed header newlines to build on Linux

\date 2014-10-01 (version 3.0)
- Moved to MIT License

\date 2014-08-05 (version 2.2.0)
- added serial-sts-demo.c

\date 2014-06-12 (version 2.1.8)
- Minor tweaks for Cygwin 32-bit

\date 2014-04-28 (version 2.1.7)
- Updated some SeaBreezeWrapper function names for consistency
- Added demo-pthreads per request [#3351]

\date 2014-02-11 (version 2.1.6)
- added data-collection and merge-spectra [#3311]

\date 2013-11-18 (version 2.1.5)
- Added raw USB access to Maya2000Pro
- Fixed error code on openSpectrometer()
- Added support for QE-PRO spectrometer

\date 2013-05-21 (version 2.1.4)
- Fixed STS minimum integration time (changed from 1ms to 10usec) [#2653]
- Fixed Ventana minimum integration time (changed to 22ms)
- Fixed a QE65000 wavelength calibration error [#2916]
- Added laser controls for Ventana 785L [#3045]
- Reorganized protocols into subdirectories
- Added scripts/update-visual-studio-projects [#3069]
- Applied requested customer updates to docs and Makefiles [#3069]
- Fixed support for multiple spectrometers of the same type [#3073]
- Standardized all Visual Studio builds on Unicode [#3073]
- Added filters to VS2010 project [#3073]

\date 2013-03-21 (version 2.1.3)
- Added os-support/windows/VisualStudio2005 and 2012 [#2904]
- Added OBP support for ThermoElectric (including Ventana) [#2902]

\date 2013-01-22 (version 2.1.2)
- Added continuous strobe for STS [#2847]
- Added support for Apex and Ventana [#2842]
- Added seabreeze_get_usb_descriptor_string() [#2806]
- Fixed Feature initializers [#2835]

\date 2012-12-20 (version 2.1.1)
- fixed compile warnings in 64-bit due to size differences

\date 2012-10-12 (version 2.1.0)
- Resolved 64-bit issues (builds, installs, and runs in Win32 on Win7-64)
- Added seabreeze_set_continuous_strobe_period_microseconds()
- Added seabreeze_write_usb() and seabreeze_read_usb()
- Added seabreeze_write_eeprom_slot()
- Added seabreeze_set_verbose() to simplify external testing
- Added seabreeze_get_api_version_string()
- Renamed MayaPro2000 to Maya2000Pro
- Purged unsupported legacy targets

\date 2012-10-03 (version 2.0)
- Merged SeaBreezeOEM back into core distribution (see \ref release_notes_20)
- added Visual Studio solution, signed device drivers and WinUSB co-installers
- simplified Makefiles
- added Doxygen

\date 2011-06-13
- Revised STS protocol interface to be more transaction oriented.  This will
  allow the driver to respond more correctly to unexpected error messages
  from the STS, rather than detecting problems as part of the reply to
  some subsequent request.
- Added initial support for USB devices under MacOSX.  This has been
  tested under Snow Leopard, but should work for any version of OSX.  There
  is one significant limitation at this time: only one type of device can
  be used at once, though any number of a particular device should work.
  The way that OSX probes USB devices is different enough from Windows and
  Linux that the API could not really handle all three at once.  In a
  future release, the USB probing code for all operating systems will be
  revised to remove this limitation.  Building for OSX should be the same
  as building for Linux; it just requires gcc and make to be installed.
  There is no XCode project for SeaBreeze at this time.

\date 2011-03-03
- Added initial support for STS spectrometer.  SeaBreeze can perform basic
  spectrometer operations (set integration time, set trigger mode, get spectra,
  get wavelengths), get the device serial number, and control the shutter.
  Additional features are planned in a subsequent release.
- Refactored the feature layer slightly so that there are now abstract
  feature interfaces similar to the abstract protocol interfaces that were one
  layer down.  This makes dealing with multiple implementations of a given
  feature (which is necessary for the STS) much simpler.

\date 2010-03-24
- Added an exchange to read out the irradiance calibration factors that may
  be stored in a USB2000+.  This feature could also be ported to other devices
  that support the same op-code.
- Added code to report the detector pixels that can be used for electric dark
  correction.  Provided sample code showing how it can be used and how to
  perform nonlinearity correction (see spectral_correction.c in the test
  directory).  Note that only the seabreeze_test_linux.c program calls these
  sample functions, but the same should work under Windows.
- Added support for programmable saturation levels on the MayaPro spectrometer.

\date 2009-11-17
- Fixed bug in how USB descriptors were read in Windows.  This may have
  affected USB4000 and HR4000 functionality for detecting USB1.1 vs. USB2.0.
- Fixed a bug where device handle was not being properly released.
- Fixed a bug in the TEC interface for specifying temperature set point (wrong
  op-code had been used).
- Restructured WinUSB driver to allow multiple devices to be used and added
  index parameter to all SeaBreezeWrapper calls to specify a device.
- Changed the GUIDs specified in the .inf files.  The new WinUSB driver requires
  the updated .inf files.  The WinUSB driver no longer has device-specific
  GUIDs embedded in it.
- Renamed NativeUSB.c to NativeUSBLinux.c to avoid confusion
  (in src/native/usb/linux).

\date 2009-08-25
- Added external trigger mode support.
- Added function to SeaBreezeWrapper to provide spectrometer wavelengths.

\date 2009-07-24
- initial public release

\subsection release_notes_20 SeaBreeze 2.0 Release Notes

This was a significant release of SeaBreeze in that it merged the Ocean Optics
"core" version of the SeaBreeze driver (used for internal projects) with the
"OEM" version (sometimes called SeaBreezeOEM), which was used for external
customer projects.

By merging the two forks back into a unified whole, this ensured that:

1. External OEM customers receive the "latest and greatest" updates which
Ocean Optics is regularly adding for its own use; and

2. Ocean Optics will be "eating its own dog food" as it were, continually
testing and utilizing the same software which we provide to our OEM customers.

Both consequences are anticipated to benefit OEM customers over time, by
improving both timeliness and robustness of future releases.

Key changes in SeaBreeze 2.0 for existing internal (non-OEM users) include:

- build process has been streamlined and simplified for POSIX architectures
(Linux and MacOS)

- automated Doxygen documentation has been added (see doc/)

- a working Visual Studio solution has been added (see os-support/windows)

- Windows device drivers have been added to the distribution

- support for easier memory profiling has been added (see
include/common/globals.h)

- support has been added to completely destroy the SeaBreezeWrapper and
DeviceFactory singletons, which had been found to generate spurious memory
warnings under Visual Studio

- a rudimentary library of sample code has been started to demonstrate how to
implement basic spectral processing functions in application code, add timeouts
to potentially blocking seabreeze functions (like getSpectrum), etc (see
sample-code/)

Key changes in SeaBreeze 2.0 for existing OEM users include:

- support for RS-232 and ethernet buses has been added

- support for the Torus has been added

- support for the Jaz has been added

- support for stray light coefficients has been added

- an entirely new SeaBreezeAPI has been added, which will allow support for many
powerful new capabilities in the Jaz, STS, QE65kPro, and other advanced
spectrometers (see include/api/seabreezeapi)

- previously supported spectral processing functions have been removed from the
core driver, including multi-scan averaging, boxcar filters, non-linearity
correction and electrical dark correction.  All functions can still be
implemented in application code, and we are in the process of documentating how
each of these and other functions can be simply achieved (see sample-code/)

Additional changes will be new for all users:

- Linux udev rules have been updated to work on the latest common distributions

- A C# GUI demonstration application has been added under
sample-code/CSharpDemo.  This demonstrates how to perform a number of
commonly-requested basic functions, such as threaded background spectral
acquisition, real-time graphing using .NET 4.0 DataVisualization, absolute
irradiance calculation, how to build Windows installers, etc.

- updated Windows device drivers have been provided for the HR2000+,
Maya2000Pro, NIRQuest256, NIRQuest512, and USB2000+

\section ezusb Appendix B: Co-Existing with EzUSB (32-bit Windows)

Ocean Optics supports two different USB drivers on 32-bit Windows: WinUSB (used
by Ocean Optics SeaBreeze), and Cypress's ezUSB (used by Ocean Optics OmniDriver,
SpectraSuite, OceanView and USB Programmer).

It has been found that the two drivers can coexist on the same system, although
the user or a script is required to switch between one and another.  This can be
done manually:

\verbatim
    Computer -> Manage -> Device Manager -> Update Driver ->
        No Not This Time -> Install From a List -> Don't Search I Will Choose ->
            "USB2000+" (ezUSB) or "USB2000+ (WinUSB)" (SeaBreeze)
\endverbatim

Alternately, this can be scripted using the "devcon.exe" executable which comes
with the WinDDK (C:/WinDDK/7600.16385.1/tools/devcon/i386).  The following
Cygwin transcript shows drivers for a pair of connected USB2000+ being flipped
between ezUSB (OmniDriver) and WinUSB (SeaBreeze) and back again:

\verbatim
    $ devcon update C:\\WINDOWS\\inf\\oem13.inf 'USB\Vid_2457&Pid_101e'
    Updating drivers for USB\Vid_2457&Pid_101e from C:\WINDOWS\inf\oem13.inf.
    Drivers installed successfully.

    $ devcon update C:\\WINDOWS\\inf\\ooi_usb.inf 'USB\Vid_2457&Pid_101e'
    Updating drivers for USB\Vid_2457&Pid_101e from C:\WINDOWS\inf\ooi_usb.inf.
    Drivers installed successfully.
\endverbatim

Note that the correct "oemXX.inf" driver name will be specific to your system,
and can be established through something like this:

\verbatim
    $ devcon driverfiles '=OceanOpticsUSBDevice'
    USB\VID_2457&PID_101E\6&22194588&0&1
        Name: Ocean Optics USB2000+ (WinUSB)
        Driver installed from c:\windows\inf\oem13.inf [OOIUSB_Install]. 2 file(s) used by driver:
            C:\WINDOWS\system32\WinUSBCoInstaller.dll
            C:\WINDOWS\system32\WdfCoInstaller01007.dll
    1 matching device(s) found.

    $ devcon driverfiles '=OceanOpticsUSBDevice' | grep inf | awk '{print $4}'
    c:\windows\inf\oem13.inf
\endverbatim

...or...

\verbatim
    $ grep -l "Ocean Optics USB Devices" /cygdrive/c/windows/inf/*.inf | xargs cygpath -d
    C:\windows\inf\oem13.inf
\endverbatim

Likewise, the "hardware id" which devcon expects to refer to a spectrometer
model will need to be ascertained for your units.  This can typically be found
in the Device Manager (Properties -> Details -> Hardware IDs), or by running:

\verbatim
    $ devcon hwids '*Vid_2457*'
    USB\VID_2457&PID_101E\6&22194588&0&1
        Name: Ocean Optics USB2000+
        Hardware IDs:
            USB\Vid_2457&Pid_101e&Rev_0002
            USB\Vid_2457&Pid_101e
        Compatible IDs:
            USB\Class_ff&SubClass_00&Prot_00
            USB\Class_ff&SubClass_00
            USB\Class_ff
    USB\VID_2457&PID_101E\6&22194588&0&2
        Name: Ocean Optics USB2000+
        Hardware IDs:
            USB\Vid_2457&Pid_101e&Rev_0002
            USB\Vid_2457&Pid_101e
        Compatible IDs:
            USB\Class_ff&SubClass_00&Prot_00
            USB\Class_ff&SubClass_00
            USB\Class_ff
    2 matching device(s) found.
\endverbatim

\section version Document Version

\verbatim
    $Header: http://gforge.oceanoptics.com/svn/seabreeze/releases/Release_2014_10_01-1730-3.0/README.txt 1221 2014-10-01 21:35:43Z mzieg $
\endverbatim
*/
