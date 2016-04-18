[README.TXT]

== OVERVIEW ==

This is a sample Visual C# GUI application which demonstrates how to 
successfully connect to and command a USB Ocean Optics spectrometer 
using SeaBreeze under Windows.

The program assumes the following:

- Visual Studio 2010
- .NET 4.0 (the graphing widget isn't supported under earlier versions)
- SeaBreeze 2.x

Besides demonstrating SeaBreeze calls, the program also shows the following:

- how to acquire spectra from a background thread
- how to graph spectra
- how to compute absolute irradiance
- how to create an application installer

== ARCHITECTURE ==

Spectrometer access is wrapped behind an abstract "Spectrometer" interface
which allows easy switching between SeaBreeze, OmniDriver, and potential
future spectrometer driver layers.  OmniDriver itself is not used or
required in this demo, and in fact the OmniDriverSpectrometer class isn't
even compiled in the solution.  However, it is provided as a useful example
of the benefits of loosely coupling architectural layers.
