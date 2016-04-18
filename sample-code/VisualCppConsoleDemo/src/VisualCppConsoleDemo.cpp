/***********************************************************************//**
 * VisualCppConsoleDemo.cpp
 *
 * This file represents a simple command-line SeaBreeze test using Visual C++
 * and the C++ interface to SeaBreezeWrapper.
 *
 * Tested under Win7-32 and Visual Studio 2010 by mzieg on 20-Feb-2014.
 *
 * LICENSE:
 *
 * SeaBreeze Copyright (C) 2014, Ocean Optics Inc
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "api/seabreezewrapper.h"

#include <iostream>

#define MILLISEC_TO_MICROSEC 1000

using std::cout;
using std::cin;
using std::endl;

int _tmain(int argc, _TCHAR* argv[])
{
    int specIndex = 0;
    int errorCode = 0;

    ////////////////////////////////////////////////////////////////////////////
    // Initialization
    ////////////////////////////////////////////////////////////////////////////

    SeaBreezeWrapper* wrapper = SeaBreezeWrapper::getInstance();

    cout << "Connecting to spectrometer...";
    wrapper->openSpectrometer(specIndex, &errorCode);
    if (errorCode)
    {
        cout << "Error: no spectrometers found.\n";
        return 1;
    }
    cout << "done" << endl;

    cout << "Setting integration time...";
    int integrationTimeMillisec = 100;
    wrapper->setIntegrationTimeMicrosec(specIndex, &errorCode, integrationTimeMillisec * MILLISEC_TO_MICROSEC);
    if (errorCode)
    {
        cout << "Error: problem setting integration time\n";
        return 1;
    }
    cout << "done" << endl;

    cout << "Getting number of pixels...";
    int pixels = wrapper->getFormattedSpectrumLength(specIndex, &errorCode);
    if (errorCode)
    {
        cout << "Error: problem getting spectrum length\n";
        return 1;
    }
    cout << "done" << endl;

    // allocate spectrum
    double* spectrum = (double*) malloc(pixels * sizeof(double));

    ////////////////////////////////////////////////////////////////////////////
    // Iterate over spectra
    ////////////////////////////////////////////////////////////////////////////

    char quit = 'n';
    while (quit != 'y' && quit != 'Y')
    {
        wrapper->getFormattedSpectrum(specIndex, &errorCode, spectrum, pixels);
        if (errorCode)
        {
            cout << "Error: problem getting spectrum\n";
        }

        for (int i = 0; i < pixels; i++)
            if (i < 5 || i > pixels - 5)
                cout << "pixel " << i << " = " << spectrum[i] << "\n";

        cout << "Ready to quit? (yN) ";
        cin >> quit;
    }

    cout << "Closing spectrometer...";
    wrapper->closeSpectrometer(specIndex, &errorCode);
    if (errorCode)
    {
        cout << "Error: can't close spectrometer\n";
        return 1;
    }
    cout << "done.\n";

	return 0;
}
