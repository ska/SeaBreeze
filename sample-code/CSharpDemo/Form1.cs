using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Reflection;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;
using System.Threading;

namespace CSharpDemo
{
    public partial class Form1 : Form
    {
        ////////////////////////////////////////////////////////////////////////
        // Spectrometer
        ////////////////////////////////////////////////////////////////////////

        Spectrometer spectrometer = null;

        int pixels = 0;

        double[] wavelengths;
        double[] rawSpectrum;
        double[] procSpectrum;
        double[] dark;

        int[] edcIndices;
        double[] nlcCoeffs;
        bool scanning = false;

        double integrationTimeMillisec = 100; 
        int triggerMode = 0;

        double[] irradCal;
        double collectionArea = 1.0;    // cm^2

        string serialNumber;
        string spectrometerType;
        uint scansToAverage = 1;
        uint intensityDelta = 50;

        const uint MAX_NUMBER_OF_PEAKS = 100;
        uint persistenceWindow = 5;

        double currentYAxisValue = 0.0f;
        double lastMousePositionX;
        double lastMousePositionY;

        private Logger logger = Logger.getInstance();

        bool closePending = false;

        ////////////////////////////////////////////////////////////////////////
        // Initialization
        ////////////////////////////////////////////////////////////////////////

        public Form1()
        {
            InitializeComponent();

            this.Text = String.Format("SeaBreeze C# Demo (Demo v{0})",
                Assembly.GetExecutingAssembly().GetName().Version.ToString());

            // initialize chart mouseOver events
            this.chartSpectrum.MouseMove += new MouseEventHandler(chartSpectrum_onMouseMove);
            this.chartSpectrum.MouseLeave += new EventHandler(chartSpectrum_onMouseLeave);
            this.chartSpectrum.PostPaint += new EventHandler<ChartPaintEventArgs>(chartSpectrum_PostPaint);

            // initialize logger
            logger.setTextBox(textBoxEventLog);

            initialize();
            updateSpectrometerFromGUI();
            updateGUIFromSpectrometer();
        }

        private void initialize()
        {
            // assume initialization will fail
            btnScan.Enabled = false;
            checkBoxEnableIrradiance.Enabled = false;

            if (spectrometer != null && !backgroundWorkerAcquisition.IsBusy)
            {
                spectrometer.close();
                spectrometer = null;
            }

            if ((spectrometer == null || !spectrometer.isOk()) && !backgroundWorkerAcquisition.IsBusy)
            {
                logger.display("Initializing spectrometer...");
                spectrometer = new SeaBreezeSpectrometer();

                spectrometer.setLogger(logger);

                string version = spectrometer.getVersion();
                if (version != null)
                    this.Text = String.Format("SeaBreeze C# Demo v{0} (SeaBreeze v{1})",
                        Assembly.GetExecutingAssembly().GetName().Version.ToString(),
                        version);

                spectrometer.open();
                if (spectrometer.isOk())
                {
                    serialNumber = spectrometer.getSerialNumber();
                    spectrometerType = spectrometer.getSpectrometerType();
                    pixels = spectrometer.getPixelCount();
                    wavelengths = spectrometer.getWavelengths();
                    rawSpectrum = new double[pixels];

                    logger.display("Successfully claimed {0} with {1} pixels", spectrometerType, pixels);

                    collectionArea = spectrometer.getCollectionArea();
                    irradCal = spectrometer.getIrradianceCalibration();
                    if (collectionArea <= 0 || irradCal == null)
                    {
                        checkBoxEnableIrradiance.Checked = false;
                        checkBoxEnableIrradiance.Enabled = false;
                    }
                    else
                    {
                        checkBoxEnableIrradiance.Enabled = true;
                        logger.display("Irradiance Collection Area = {0} cm^2", collectionArea);
                    }

                    edcIndices = spectrometer.getEDCIndices();
                    if (edcIndices != null && edcIndices.Length > 0)
                    {
                        logger.display("Electrically dark pixel indices:");
                        for (int i = 0; i < edcIndices.Length; i++)
                            logger.display("  [{0}] = {1}", i, edcIndices[i]);
                    }
                    else
                        checkBoxEnableEDC.Enabled = false;

                    nlcCoeffs = spectrometer.getNLCCoeffs();
                    if (nlcCoeffs != null && nlcCoeffs.Length > 0)
                    {
                        logger.display("Non-Linearity Correction Coefficients:");
                        for (int i = 0; i < nlcCoeffs.Length; i++)
                            logger.display("  [{0}] = {1}", i, nlcCoeffs[i]);
                    }
                    else
                        checkBoxEnableNLC.Enabled = false;

                    // buttonSave.Enabled = true;
                    btnScan.Enabled = true;
                    Refresh();
                }
                else
                {
                    logger.display("Failed to initialize spectrometer");
                    MessageBox.Show(
                        "No usable spectrometer available",
                        "SeaBreeze",
                        MessageBoxButtons.OK,
                        MessageBoxIcon.Error,
                        MessageBoxDefaultButton.Button1,
                        MessageBoxOptions.DefaultDesktopOnly,
                        false);
                }
            }
            else
            {
                logger.log("Spectrometer already initialized");
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Callbacks
        ////////////////////////////////////////////////////////////////////////

        private void buttonSaveDark_Click(object sender, EventArgs e)
        {
            if (dark == null && procSpectrum != null && procSpectrum.Length > 0)
            {
                dark = new double[pixels];
                Array.Copy(procSpectrum, dark, pixels);
                buttonSaveDark.Text = "Clear Dark";
            }
            else
            {
                dark = null;
                buttonSaveDark.Text = "Take Dark";
            }
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            initialize();
            updateSpectrometerFromGUI();
            updateGUIFromSpectrometer();
        }

        private void checkBoxEnableIrradiance_Click(object sender, EventArgs e)
        {
            logger.display("Irradiance {0}", checkBoxEnableIrradiance.Checked ? "enabled" : "disabled");
            processSpectrum();
        }

        private void numericUpDownTriggerMode_ValueChanged(object sender, EventArgs e)
        {
            triggerMode = (int)numericUpDownTriggerMode.Value;
            updateSpectrometerFromGUI();
        }

        private void numericUpDownIntegrationTimeMillisec_ValueChanged(object sender, EventArgs e)
        {
            integrationTimeMillisec = (double) numericUpDownIntegrationTimeMillisec.Value;
            updateSpectrometerFromGUI();
        }

        private void PersistenceUpDown_ValueChanged(object sender, EventArgs e)
        {
            persistenceWindow = (uint)PersistenceUpDown.Value;
        }

        private void IntensityUpDown_ValueChanged(object sender, EventArgs e)
        {
            intensityDelta = (uint)IntensityUpDown.Value;
        }

        private void numericUpDownScansToAverage_ValueChanged(object sender, EventArgs e)
        {
            scansToAverage = (uint) numericUpDownScansToAverage.Value;
        }

        private void buttonSave_Click(object sender, EventArgs e)
        {
            DialogResult result = saveFileDialogSpectra.ShowDialog();
            if (result != DialogResult.OK)
                return;

            string pathname = saveFileDialogSpectra.FileName;
            using (System.IO.StreamWriter outfile = new System.IO.StreamWriter(pathname))
            {
                outfile.WriteLine("wavelength,counts");
                for (int i = 0; i < pixels; i++)
                    outfile.WriteLine("{0:f2},{1:f2}", wavelengths[i], procSpectrum[i]);
            }

            logger.display("Saved {0}", pathname);
        }

        private void buttonScan_Click(object sender, EventArgs e)
        {
            if (!scanning)
            {
                logger.display("Start scanning");
                btnScan.Text = "Stop";
                buttonReset.Enabled = false;
                scanning = true;
                backgroundWorkerAcquisition.RunWorkerAsync();
            }
            else
            {
                logger.display("Stop scanning");
                backgroundWorkerAcquisition.CancelAsync();
            }
        }

        // These are only used for the mouse-over peakfinding (not sure it's worth the complexity)
        void chartSpectrum_PostPaint(object sender, ChartPaintEventArgs e)
        {
            // Create font and brush.
            SolidBrush SolidBlueBrush   = new SolidBrush(Color.Blue);
            SolidBrush labelBrush       = new SolidBrush(Color.Black);
            Font drawFont               = new Font("Arial", 8);
            Pen LabelPen                = new Pen(Color.Black);
            Point DrawPoint             = new Point(0, 0);
            Point LabelPoint            = new Point(0, 0);

            const float radius = 5.0f;
            ChartGraphics chartGraphics = e.ChartGraphics;
            String drawString = "";    
            
            if (e.ChartElement is Series && PeakFindCheckBox.Checked == true )
            {
                Series spectrum = chartSpectrum.Series.FindByName("seriesSpectrum");

                List<int> SpectrumPeaks = new List<int>();
                int numberOfSpectrumPeaks = FindPeaks(ref SpectrumPeaks);
                for (int i = 0; i < numberOfSpectrumPeaks; i++)
                {
                    // draw a circle at the peak
                    System.Drawing.PointF pos = System.Drawing.Point.Empty;
                    pos.X = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.X, spectrum.Points[SpectrumPeaks[i]].XValue);
                    pos.Y = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.Y, spectrum.Points[SpectrumPeaks[i]].YValues[0]);
                    pos = chartGraphics.GetAbsolutePoint(pos);
                    chartGraphics.Graphics.FillEllipse(
                               SolidBlueBrush,
                               pos.X - (radius / 2),
                               pos.Y - (radius / 2),
                               radius, radius);
  
                    // now label the peak we just found:       
                    DrawPoint.X = (int)(pos.X+10);
                    DrawPoint.Y = (int)(pos.Y-10);
                    LabelPoint.X = DrawPoint.X;
                    LabelPoint.Y = (int)(pos.Y-5);
                    drawString = String.Format("Wavelength = {0}nm , Intensity = {1}", (int)spectrum.Points[SpectrumPeaks[i]].XValue, (int)spectrum.Points[SpectrumPeaks[i]].YValues[0]);

                    // Draw string to screen.
                    chartGraphics.Graphics.DrawLine(LabelPen,pos,LabelPoint);
                    chartGraphics.Graphics.DrawString(drawString, drawFont, labelBrush, DrawPoint);
                }
                
                if (checkBoxEnableIrradiance.Checked)
                {
                    Series Irradiance = chartSpectrum.Series.FindByName("seriesIrradiance");
                    double minPoint = spectrum.Points.FindMinByValue().YValues[0];

                    List<int> IrradiancePeaks = new List<int>();
                    int numberOfIrradiancePeaks = FindPeaks(ref IrradiancePeaks);

                    // MZ: This looks like copy-paste from above...extract into function if worth keeping
                    for (int i = 0; i < numberOfIrradiancePeaks; i++)
                    {
                        // draw a circle at the peak
                        System.Drawing.PointF pos = System.Drawing.Point.Empty;
                        pos.X = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.X, Irradiance.Points[IrradiancePeaks[i]].XValue);
                        pos.Y = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.Y, Irradiance.Points[IrradiancePeaks[i]].YValues[0]);
                        pos = chartGraphics.GetAbsolutePoint(pos);
                        chartGraphics.Graphics.FillEllipse(
                                   SolidBlueBrush,
                                   pos.X - (radius / 2),
                                   pos.Y - (radius / 2),
                                   radius, radius);

                        // now label the peak we just found
                        DrawPoint.X = (int)(pos.X+10);
                        DrawPoint.Y = (int)(pos.Y-10);
                        LabelPoint.X = DrawPoint.X;
                        LabelPoint.Y = (int)(pos.Y-5);
                        drawString = String.Format("Wavelength = {0}nm , Intensity = {1}", (int)Irradiance.Points[SpectrumPeaks[i]].XValue, (int)Irradiance.Points[SpectrumPeaks[i]].YValues[0]);

                        // Draw string to screen.
                        chartGraphics.Graphics.DrawLine(LabelPen, pos, LabelPoint);
                        chartGraphics.Graphics.DrawString(drawString, drawFont, labelBrush, DrawPoint);
                    }
                }
            }

            DrawMousePosition(ref chartGraphics, SolidBlueBrush, radius);
            SolidBlueBrush.Dispose();
            LabelPen.Dispose();
            labelBrush.Dispose();
            Update();
        }

        private void DrawMousePosition(ref ChartGraphics chartGraphics, Brush theBrush, float radius) 
        {
            if (scanning)
            {
                Series spectrum = chartSpectrum.Series.FindByName("seriesSpectrum");
                ChartArea chartArea = chartSpectrum.ChartAreas[0];
                for (int i = 0; i < pixels; i++)
                {
                    if( (lastMousePositionX) == (int)spectrum.Points[i].XValue)
                    {
                        // we've found the cooresponding index into the series that our mouse x position is on
                        // draw a dot on it and label it with wavelength and intensity
                        System.Drawing.PointF pos = System.Drawing.Point.Empty;
                        pos.X = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.X, spectrum.Points[i].XValue);
                        pos.Y = (float)chartGraphics.GetPositionFromAxis("ChartArea1", AxisName.Y, spectrum.Points[i].YValues[0]);
                        pos = chartGraphics.GetAbsolutePoint(pos);
                        chartGraphics.Graphics.FillEllipse(
                                   theBrush,
                                   pos.X - (radius / 2),
                                   pos.Y - (radius / 2),
                                   radius, 
                                   radius);
                    }
                }
            }
        }

        private void chartSpectrum_onMouseMove(object sender, MouseEventArgs e)
        {
            if (scanning)
            {
                ChartArea chartArea = chartSpectrum.ChartAreas[0];
                try
                {
                    lastMousePositionX = chartArea.AxisX.PixelPositionToValue(e.X);
                    lastMousePositionY = chartArea.AxisY.PixelPositionToValue(e.Y);
                    String ttString;
                    ttString = String.Format("Wavelength = {0} , Intensity = {1}, mouseX = {2}, mouseY = {3}",
                        (int)lastMousePositionX, (int)lastMousePositionY, e.X, e.Y);
                    MouseOverTT.Show(ttString, this.chartSpectrum, e.X, e.Y);
                    lastMousePositionX = e.X;
                    lastMousePositionY = e.Y;
                }
                catch (ArgumentException ex)
                {
                    logger.display("{0}", ex.Message);
                }
            }
        }

        private void chartSpectrum_onMouseLeave(object sender, EventArgs e)
        {
            MouseOverTT.Hide(this.chartSpectrum);
            lastMousePositionX = 0;
            lastMousePositionY = 0;
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            if(backgroundWorkerAcquisition.IsBusy || backgroundWorkerAcquisition.CancellationPending)
            {
                // if acquisition worker is running, tell it to quit, then retrigger shutdown
                e.Cancel = true;
                closePending = true;
                logger.log("[OnFormClosing] signalling acquisition worker to quit");
                backgroundWorkerAcquisition.CancelAsync();
            }
            else if (!closePending)
            {
                // otherwise, since nothing is running, shutdown now
                logger.log("[OnFormClosing] shutting down directly");
                cleanShutdown();
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // Methods
        ////////////////////////////////////////////////////////////////////////

        private void updateGUIFromSpectrometer()
        {
            labelSerialNumber.Text = serialNumber;
            labelType.Text = spectrometerType;
            labelPixels.Text = String.Format("{0}", pixels);
        }

        private void updateSpectrometerFromGUI()
        {
            spectrometer.setTriggerMode(triggerMode);
            spectrometer.setIntegrationTimeMilliseconds(integrationTimeMillisec);
        }

        // Perform absolute irradiance calibration for spectrometer.
        //
        // S:  Sample spectrum (counts per nanometer)
        // D:  Dark spectrum (counts per nanometer, with the same integration time, 
        //     corrections, and smoothing as sample)
        // C:  Calibration (represented in micro-Joules per count)
        // T:  Integration time (represented here in seconds)
        // A:  Collection area (represented in square centimeters) unless the light 
        //     source is entirely inside an integrating sphere
        // dL: The wavelength spread (how many nanometers a given pixel represents)
        //
        // Absolute irradiance (I) is computed as follows.  Below, the subscript P 
        // will indicate a particular pixel for I, dL, S, D, and C.  Thus, SP refers 
        // to pixel P of the sample spectrum.
        //
        //      IP = (SP - DP) * CP / (T * A * dLP)
        //
        // Note that if the lamp is entirely enclosed within an integrating sphere, 
        // then the A term is omitted.
        //
        // dL is typically computed as follows, where L(P) refers to the wavelength 
        // represented by the center of pixel index P.
        //
        //      dL = [L(P + 1) - L(P - 1)] / 2
        //
        private double[] computeAbsoluteIrradiance()
        {
            if (collectionArea <= 0 || procSpectrum == null || wavelengths == null)
                return null;

            double[] irradiance = new double[pixels];
            double[] dLp = new double[pixels];

            // Calculate dLp
            dLp[0] = wavelengths[1] - wavelengths[0];
            for (int i = 1; i < pixels - 1; i++)
                dLp[i] = (wavelengths[i + 1] - wavelengths[i - 1]) / 2.0;
            dLp[pixels - 1] = wavelengths[pixels - 1] - wavelengths[pixels - 2];

            // compute irradiance
            for (int i = 0; i < pixels; i++)
                irradiance[i] = (procSpectrum[i] * irradCal[i]) 
                              / (integrationTimeMillisec * 1000.0 * collectionArea * dLp[i]);

            return irradiance;
        }

        private void processSpectrum()
        {
            // start with raw spectrum we just acquired
            procSpectrum = new double[pixels];
            Array.Copy(rawSpectrum, procSpectrum, pixels);

            // perform EDC if requested 
            // Note that this should be done BEFORE ODC, if both EDC and ODC are
            // used, and that EDC should be applied to BOTH the dark and sample
            // measurements (or to neither).
            if (checkBoxEnableEDC.Checked)
            {
                double value = 0.0;
                int usableDarkCount = 0;
                for (int i = 0; i < edcIndices.Length; i++)
                {
                    int edcPixelIndex = edcIndices[i];
                    if (edcPixelIndex < pixels)
                    {
                        value += procSpectrum[edcPixelIndex];
                        usableDarkCount++;
                    }
                }
                if (usableDarkCount > 0)
                {
                    value /= usableDarkCount;
                    for (int i = 0; i < pixels; i++)
                        procSpectrum[i] -= value;
                }
            }

            // perform ODC if requested
            if (dark != null)
                for (int i = 0; i < pixels; i++)
                    procSpectrum[i] -= dark[i];

            // perform NLC if requested (should be done after EDC and ODC)
            if (checkBoxEnableNLC.Checked)
            {   
                for (int i = 0; i < pixels; i++) 
                {   
                    double pixel = procSpectrum[i];
                    double factor = nlcCoeffs[0];
                    double pixelToTheN = pixel;

                    // factor is already initialized with x^0...start at x^1
                    for (int j = 1; j < nlcCoeffs.Length; j++) 
                    {   
                        factor += pixelToTheN * nlcCoeffs[j];
                        pixelToTheN *= pixel;
                    }   

                    if (factor != 0.0) 
                        procSpectrum[i] = pixel / factor;
                }   
            }   

            // perform boxcar if requested (should be done after NLC)
            if (numericUpDownBoxcarHalfWidth.Value > 0)
            {
                int boxcar = (int) numericUpDownBoxcarHalfWidth.Value;
                double[] smoothed = new double[pixels];

                int boxcarLimit = pixels - boxcar - 1;
                int boxcarRange = 2 * boxcar + 1;

                double sum;
                for (int i = boxcar; i <= boxcarLimit; i++)
                {
                    sum = procSpectrum[i];
                    for (int j = 1; j <= boxcar; j++)
                        sum += procSpectrum[i - j] + procSpectrum[i + j];
                    smoothed[i] = sum / boxcarRange;
                }

                for (int i = boxcar; i <= boxcarLimit; i++)
                    procSpectrum[i] = smoothed[i];
            }

            updateGraph();
        }

        private void updateGraph()
        {
            chartSpectrum.ChartAreas["ChartArea1"].AxisX.LabelStyle.Format = "f0";

            Series seriesSpectrum = chartSpectrum.Series.FindByName("seriesSpectrum");
            Series seriesIrradiance = chartSpectrum.Series.FindByName("seriesIrradiance");

            seriesSpectrum.Points.Clear();
            seriesIrradiance.Points.Clear();

            for (int i = 0; i < pixels; i++)
                seriesSpectrum.Points.AddXY(wavelengths[i], procSpectrum[i]);

            if (checkBoxEnableIrradiance.Checked)
            {
                double[] irradiance = computeAbsoluteIrradiance();
                if (irradiance != null)
                    for (int i = 0; i < pixels; i++)
                        seriesIrradiance.Points.AddXY(wavelengths[i], irradiance[i]);
            }
            Refresh();
        }

        private void cleanShutdown()
        {
            logger.log("[CleanShutdown] shutdown initiated");

            // block further user input
            this.Enabled = false;

            // shutdown spectrometer
            spectrometer.close();

            // shutdown app
            if (closePending)
                this.Close();
        }

        ////////////////////////////////////////////////////////////////////////
        // BackgroundWorkerAcquisition
        ////////////////////////////////////////////////////////////////////////

        private void backgroundWorkerAcquisition_DoWork(object sender, DoWorkEventArgs e)
        {
            logger.log("[Acquisition] starting...");
            if (spectrometer == null || !spectrometer.isOk())
            {
                logger.display("Can't start acquisition (missing spectrometer or driver)");
                return;
            }

            BackgroundWorker worker = sender as BackgroundWorker;
            int scanCount = 0;

            while (true)
            {
                DateTime timeStart = DateTime.Now;

                double[] newRaw = spectrometer.getSpectrum();
                if (newRaw == null || newRaw.Length != pixels)
                {
                    logger.display("Error taking acquisition");
                    break;
                }

                // perform multi-scan averaging, if requested
                if (scansToAverage > 1)
                {
                    for (int i = 1; i < scansToAverage; i++)
                    {
                        double[] tmpRaw = spectrometer.getSpectrum();
                        if (tmpRaw == null || tmpRaw.Length != pixels)
                        {
                            logger.display("Error taking acquisition");
                            break;
                        }
                        for (int j = 0; j < pixels; j++)
                            newRaw[j] += tmpRaw[j];
                    }

                    for (int i = 0; i < pixels; i++)
                    {
                        newRaw[i] = newRaw[i] / ((double)scansToAverage);
                    }
                }

                // copy to graphable buffer
                Array.Copy(newRaw, rawSpectrum, pixels);

                // report timing under EXTERNAL_SYNCHRONOUS trigger mode
                if (triggerMode == 2)
                    logger.push("Acquisition {0} took {1} millisec", scanCount, (DateTime.Now - timeStart).TotalMilliseconds);

                // trigger graph update in GUI thread
                worker.ReportProgress(scanCount++);

                // necessary to ensure the GUI stays responsive
                int delay = (int) integrationTimeMillisec;
                if (delay < 200)
                    delay = 200;
                Thread.Sleep(delay);
    
                // end thread if we've been asked to cancel
                if (worker.CancellationPending)
                {
                    logger.log("[Acquisition] closing");
                    e.Cancel = true;
                    break;
                }
            }
            logger.log("[Acquisition] done");
        }

        private void backgroundWorkerAcquisition_ProgressChanged(object sender, ProgressChangedEventArgs e)
        {
            logger.flush();
            processSpectrum();
        }

        private void backgroundWorkerAcquisition_RunWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            logger.log("[AcquistionComplete] worker cleanup");
            btnScan.Text = "Start";
            buttonReset.Enabled = false;
            scanning = false;
            if (closePending)
                cleanShutdown();
        }

        ////////////////////////////////////////////////////////////////////////
        // TODO: Move to PeakFinding class
        ////////////////////////////////////////////////////////////////////////

        private void PeakFindCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            bool boxIsChecked = PeakFindCheckBox.Checked;
            logger.display("Peak Finding {0}", boxIsChecked ? "enabled" : "disabled");
            if (boxIsChecked)
            {
                PersistenceUpDown.Enabled = true;
                IntensityUpDown.Enabled = true;
            }
            else
            {
                PersistenceUpDown.Enabled = false;
                IntensityUpDown.Enabled = false;
            }
        }

        private int FindPeaks(ref List<int>  peakIndexes)
        {
            // get the current persistence value
            int persistence = (int)PersistenceUpDown.Value;

            // isPeak stores a bool for the current pixel being inspected
            bool isPeak = true;

            // get the spectrum series
            Series spectrum = chartSpectrum.Series.FindByName("seriesSpectrum");

            // find the minimum intensity point in the specrum
            // this will be used for a delta value in determining peaks
            double minValue = spectrum.Points.FindMinByValue().YValues[0];   

            try
            {
                // for each pixel in the array
                for (int i = 0; i < pixels; i++)
                {
                    // start with the assumption that the current pixel is a peak
                    isPeak = true;

                    // for each pixel within the persistence window of the current pixel
                    for (int j = i - persistence; j < i + persistence; j++)
                    {
                        // if the index is valid
                        if (j >= 0 && j < pixels)
                        {
                            // if the current pixel we are iterating through
                            // has a higher intensity than the pixel we are examining for 
                            // the posibility of being a "peak", then this pixel (the 'i' one)
                            // cannot be a peak.
                            if (procSpectrum[j] > procSpectrum[i])
                            {
                                isPeak = false;
                                break;
                            }
                        }
                    }

                    // if we got this far, and isPeak is still true, then all the pixels that are +/- persistence
                    // away from the current pixel have less intensity, and therefor this pixel is a peak,
                    // so add it to the list if it is higher than the intensity threshold from the min intensity value:
                    if (true == isPeak && procSpectrum[i] > minValue + (double)IntensityUpDown.Value)
                    {
                        peakIndexes.Add(i);
                    }
                    // make sure we don't have too many peaks:
                    if (peakIndexes.Count > MAX_NUMBER_OF_PEAKS)
                    {
                        throw new TooManyPeaksFoundException("Hey, There are too many peaks in the spectrum");
                    }
                }
            }
            catch (TooManyPeaksFoundException e)
            {
                logger.display("FindPeaks: {0}", e.Message);
                logger.display("Try raising the Intensity Delta, or raising the persistance value");
            }
            catch (IndexOutOfRangeException e)
            {
                logger.display("FindPeaks: {0}", e);
            }
            return peakIndexes.Count;
        }

    }

    public class TooManyPeaksFoundException : Exception
    {
        public TooManyPeaksFoundException()
        {
        }

        public TooManyPeaksFoundException(string message)
            : base(message)
        {
        }

        public TooManyPeaksFoundException(string message, Exception inner)
            : base(message, inner)
        {
        }
    }
}
