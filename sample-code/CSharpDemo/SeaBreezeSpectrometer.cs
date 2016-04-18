using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;

public class SeaBreezeSpectrometer : Spectrometer
{
    static Mutex mut = new Mutex();
    Logger logger = null;
    bool initialized = false;
    bool opened = false;
    int specIndex = 0;              // for now, hardcode to use first enumerated spectrometer
    int pixels = 0;
    double[] wavelengths = null;

    public void setLogger(Logger log) { logger = log; }

    public bool isOk() { return initialized; }

    public int getPixelCount() { return pixels; }

    public bool open()
    {
        initialized = false;
        opened = false;
        wavelengths = null;

        int error = 0;

        SeaBreezeWrapper.seabreeze_open_spectrometer(specIndex, ref error);
        if (!checkSeaBreezeError("open_spectrometer", error))
            return false;
        opened = true;

        pixels = SeaBreezeWrapper.seabreeze_get_formatted_spectrum_length(specIndex, ref error);
        if (!checkSeaBreezeError("get_formatted_spectrum_length", error))
            return false;

        double[] tmp = new double[pixels];
        SeaBreezeWrapper.seabreeze_get_wavelengths(specIndex, ref error, ref tmp[0], pixels);
        if (!checkSeaBreezeError("get_wavelengths", error))
            return false;

        wavelengths = tmp;
        initialized = true;

        return initialized;
    }

    public string getVersion()
    {
        string result = null;

        mut.WaitOne();

        const int MAX_VERSION_LEN = 80;

        try
        {
            byte[] version = new byte[MAX_VERSION_LEN];

            int error = 0;
            SeaBreezeWrapper.seabreeze_get_api_version_string(ref version[0], version.Length);
            if (checkSeaBreezeError("get_api_version_string", error))
                result = byteToString(version);
        }
        catch (Exception e)
        {
            logger.log("Error getting version string: {0}", e.Message);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public string getSerialNumber()
    {
        string result = null;

        if (!initialized)
            return result;

        mut.WaitOne();

        try
        {
            byte[] slot = new byte[SeaBreezeWrapper.SLOT_LENGTH];

            int error = 0;
            SeaBreezeWrapper.seabreeze_get_serial_number(specIndex, ref error, ref slot[0], SeaBreezeWrapper.SLOT_LENGTH);
            if (checkSeaBreezeError("get_serial_number", error))
                result = byteToString(slot);
        }
        catch (Exception e)
        {
            logger.log("Error getting serial number: {0}", e.Message);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public double[] getNLCCoeffs()
    {
        double[] result = null;

        if (!initialized)
            return result;

        mut.WaitOne();

        try
        {
            double[] t = new double[8];
            int error = 0;
            for (int i = 0; i <= 7; i++)
            {
                byte[] buf = new byte[SeaBreezeWrapper.SLOT_LENGTH];
                int bytes_read = SeaBreezeWrapper.seabreeze_read_eeprom_slot(
                    specIndex, ref error, 6 + i, ref buf[0], SeaBreezeWrapper.SLOT_LENGTH);
                t[i] = Convert.ToDouble(byteToString(buf));
            }
            result = t;
        }
        catch (Exception e)
        {
            logger.log("Error getting NLC: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public string getSpectrometerType()
    {
        string result = null;

        if (!initialized)
            return result;

        mut.WaitOne();

        try
        {
            byte[] slot = new byte[SeaBreezeWrapper.SLOT_LENGTH];
            int error = 0;
            SeaBreezeWrapper.seabreeze_get_model(specIndex, ref error, ref slot[0], SeaBreezeWrapper.SLOT_LENGTH);
            if (checkSeaBreezeError("get_spectrometer_type", error))
                result = byteToString(slot);
        }
        catch (Exception e)
        {
            logger.log("Error getting spectrometer type: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public bool setIntegrationTimeMilliseconds(double ms)
    {
        bool result = false;

        if (!initialized)
            return result;

        mut.WaitOne();

        try
        {
            int error = 0;
            SeaBreezeWrapper.seabreeze_set_integration_time_microsec(specIndex, ref error, (long) (ms * 1000));
            result = checkSeaBreezeError("set_integration_time_microsec", error);
        }
        catch (Exception e)
        {
            logger.log("Error setting integration time: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public bool setTriggerMode(int mode)
    {
        bool result = false;

        if (!initialized)
            return result;

        mut.WaitOne();
        try
        {
            int error = 0;
            SeaBreezeWrapper.seabreeze_set_trigger_mode(specIndex, ref error, mode);
            result = checkSeaBreezeError("set_integration_time_microsec", error);
        }
        catch (Exception e)
        {
            logger.log("Error setting trigger mode: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public double[] getWavelengths()
    {
        return wavelengths;
    }

    public double[] getSpectrum()
    {
        double[] result = null;

        mut.WaitOne();

        try
        {
            int error = 0;
            double[] spec = new double[pixels];

            SeaBreezeWrapper.seabreeze_get_formatted_spectrum(specIndex, ref error, ref spec[0], pixels);
            if (checkSeaBreezeError("get_formatted_spectrum", error))
            {
                // KLUDGE: Some spectrometers (e.g. HR4000) insert non-pixel data 
                // into the first few pixels of the spectrum they report, which
                // we can override here to avoid confusing EDC and stray noise on
                // graphs.
                // 
                // TODO: Put in appropriate logic based on spectrometer model.
                for (int i = 0; i < 5; i++)
                    spec[i] = spec[5];

                result = spec;
            }
    
        }
        catch (Exception e)
        {
            logger.log("Error getting spectrum: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public double getCollectionArea()
    {
        double result = -1;

        mut.WaitOne();

        try
        {
            int error = 0;
            double area = SeaBreezeWrapper.seabreeze_read_irrad_collection_area(specIndex, ref error);
            if (checkSeaBreezeError("read_irrad_collection_area", error))
                result = area;
        }
        catch (Exception e)
        {
            logger.log("Error getting collection area: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public double[] getIrradianceCalibration()
    {
        double[] result = null;

        mut.WaitOne();

        try
        {
            int error = 0;
            float[] irrad_f = new float[pixels];
            SeaBreezeWrapper.seabreeze_read_irrad_calibration(specIndex, ref error, ref irrad_f[0], pixels);
            if (checkSeaBreezeError("read_irrad_calibration", error))
            {
                result = new double[pixels];
                Array.Copy(irrad_f, result, pixels);
            }
        }
        catch (Exception e)
        {
            logger.log("Error getting irradiance calibration: {0}", e);
        }
        finally 
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    public bool close()
    {
        bool result = false;
        opened = false;

        if (!initialized)
            return result;

        mut.WaitOne();

        try
        {
            int error = 0;
            SeaBreezeWrapper.seabreeze_close_spectrometer(specIndex, ref error);
            result = checkSeaBreezeError("close_spectrometer", error);
        }
        catch (Exception e)
        {
            logger.log("Error closing spectrometer: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }

    private string byteToString(byte[] buf)
    {
        int len = 0;
        while (buf[len] != 0 && len + 1 < buf.Length)
            len++;  

        byte[] clean = Encoding.Convert(Encoding.GetEncoding("iso-8859-1"), Encoding.UTF8, buf);
        return Encoding.UTF8.GetString(clean, 0, len);
    }

    // returns true if last operation was successful, false if last operation had an error
    private bool checkSeaBreezeError(string operation, int errorCode)
    {
        if (errorCode == SeaBreezeWrapper.ERROR_SUCCESS)
            return true;

        byte[] buffer = new byte[64];
        SeaBreezeWrapper.seabreeze_get_error_string(errorCode, ref buffer[0], 64);
        string msg = byteToString(buffer);

        if (logger != null)
            logger.log("[SeaBreeze] error during {0}: {1}", operation, msg);

        return false;
    }

    public int[] getEDCIndices()
    {
        int[] result = null;

        mut.WaitOne();

        try
        {
            int error = 0;
            int[] buf = new int[64];
            int count = SeaBreezeWrapper.seabreeze_get_electric_dark_pixel_indices(specIndex, ref error, ref buf[0], 64);
            result = new int[count];
            for (int i = 0; i < count; i++)
                result[i] = buf[i];
        }
        catch (Exception e)
        {
            logger.log("Error getting EDC indices: {0}", e);
        }
        finally
        {
            mut.ReleaseMutex();
        }

        return result;
    }
}
