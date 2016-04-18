using System;
using System.Collections.Generic;
using System.Text;

// simplifies switching between SeaBreeze and OmniDriver
interface Spectrometer
{
    bool open();
    bool close();

    void setLogger(Logger log);
    bool isOk();

    int      getPixelCount();
    double[] getWavelengths();
    double[] getSpectrum();
    double[] getIrradianceCalibration();
    double   getCollectionArea();
    string   getVersion();
    string   getSerialNumber();
    string   getSpectrometerType();
    int[]    getEDCIndices();
    double[] getNLCCoeffs();

    bool     setIntegrationTimeMilliseconds(double ms);
    bool     setTriggerMode(int mode);
}
