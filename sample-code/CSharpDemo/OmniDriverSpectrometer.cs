using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

public class OmniDriverSpectrometer : Spectrometer
{
    private static OmniDriver.CCoWrapper wrapper = new OmniDriver.CCoWrapper();
    private Logger logger = null;
    private bool initialized = false;
    private int specIndex = 0;  // for now, hardcode to first enumerated spectrometer
    private int pixels = 0;
    private double[] wavelengths = null;

    public void setLogger(Logger log)
    {
        logger = log;
    }

    public bool isOk() { return initialized; }

    public int getPixelCount() { return pixels; }

    public bool open()
    {
        initialized = false;
        try
        {
            int numFound = wrapper.openAllSpectrometers();
            if (numFound > 0)
            {
                pixels = wrapper.getNumberOfPixels(specIndex);
                if (pixels > 0)
                {
                    wavelengths = wrapper.getWavelengths(specIndex);
                    if (wavelengths != null && wavelengths.Length == pixels)
                    {
                        initialized = true;
                    }
                }
            }
        }
        catch (Exception e)
        {
            logger.display("Error communicating with OmniDriver: " + e.Message);
        }
        return initialized;
    }

    public string getSerialNumber()
    {
        return wrapper.getSerialNumber(specIndex);
    }

    public string getSpectrometerType()
    {
        return wrapper.getName(specIndex);
    }

    public bool setIntegrationTimeMilliseconds(ulong ms)
    {
        if (!initialized)
            return false;

        wrapper.setIntegrationTime(0, (int) ms * 1000);
        return true;
    }

    public double[] getWavelengths()
    {
        return wavelengths;
    }

    public double[] getSpectrum()
    {
        return wrapper.getSpectrum(specIndex);
    }

    public double[] getIrradianceCalibration()
    {
        // load irradiance calibration
        if (0 != wrapper.isFeatureSupportedIrradianceCalibrationFactor(specIndex))
        {
            irradianceController = wrapper.getFeatureControllerIrradianceCalibrationFactor(specIndex);
            if (irradianceController != null)
            {
                logger.display("Loading irradiance calibration");
                return irradianceController.getIrradianceCalibrationFactors();
            }
            else
            {
                logger.display("ERROR: could not instantiate Iradiance Calibration Factor controller");
            }
        }
        else
        {
            logger.display("ERROR: spectrometer claims not support irradiance calibration");
        }
    }

    public double getCollectionArea()
    {
        if (0 != wrapper.isFeatureSupportedIrradianceCalibrationFactor(specIndex))
        {
            irradianceController = wrapper.getFeatureControllerIrradianceCalibrationFactor(specIndex);
            if (irradianceController != null)
            {
                return irradianceController.getCollectionArea();
            }
            else
            {
                logger.display("ERROR: could not instantiate Iradiance Calibration Factor controller");
            }
        }
        else
        {
            logger.display("ERROR: spectrometer claims not support irradiance calibration");
        }
    }

    public bool close()
    {
        if (!initialized)
            return false;

        wrapper.closeAllSpectrometers();
        return true;
    }
}
