namespace CSharpDemo
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.chartSpectrum = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.btnScan = new System.Windows.Forms.Button();
            this.checkBoxEnableIrradiance = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.buttonSave = new System.Windows.Forms.Button();
            this.buttonReset = new System.Windows.Forms.Button();
            this.buttonSaveDark = new System.Windows.Forms.Button();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.numericUpDownTriggerMode = new System.Windows.Forms.NumericUpDown();
            this.label8 = new System.Windows.Forms.Label();
            this.numericUpDownScansToAverage = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.numericUpDownBoxcarHalfWidth = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.checkBoxEnableNLC = new System.Windows.Forms.CheckBox();
            this.checkBoxEnableEDC = new System.Windows.Forms.CheckBox();
            this.numericUpDownIntegrationTimeMillisec = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.labelN = new System.Windows.Forms.Label();
            this.labelPixels = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.labelType = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.labelSerialNumber = new System.Windows.Forms.Label();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.textBoxEventLog = new System.Windows.Forms.TextBox();
            this.backgroundWorkerAcquisition = new System.ComponentModel.BackgroundWorker();
            this.saveFileDialogSpectra = new System.Windows.Forms.SaveFileDialog();
            this.MouseOverTT = new System.Windows.Forms.ToolTip(this.components);
            this.PersistenceUpDown = new System.Windows.Forms.NumericUpDown();
            this.IntensityUpDown = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.PeakFindCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox4 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.chartSpectrum)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTriggerMode)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScansToAverage)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownBoxcarHalfWidth)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownIntegrationTimeMillisec)).BeginInit();
            this.groupBox3.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.PersistenceUpDown)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.IntensityUpDown)).BeginInit();
            this.groupBox4.SuspendLayout();
            this.SuspendLayout();
            // 
            // chartSpectrum
            // 
            this.chartSpectrum.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            chartArea1.AxisX.LabelStyle.Format = "F0";
            chartArea1.AxisX.Title = "Wavelength (nm)";
            chartArea1.AxisY.Title = "Intensity (Counts)";
            chartArea1.AxisY2.Title = "µWatts / cm² / nm";
            chartArea1.CursorX.IsUserEnabled = true;
            chartArea1.CursorX.IsUserSelectionEnabled = true;
            chartArea1.CursorX.LineColor = System.Drawing.Color.Transparent;
            chartArea1.CursorY.IsUserEnabled = true;
            chartArea1.CursorY.IsUserSelectionEnabled = true;
            chartArea1.CursorY.LineColor = System.Drawing.Color.Transparent;
            chartArea1.Name = "ChartArea1";
            this.chartSpectrum.ChartAreas.Add(chartArea1);
            legend1.Alignment = System.Drawing.StringAlignment.Center;
            legend1.Docking = System.Windows.Forms.DataVisualization.Charting.Docking.Bottom;
            legend1.Name = "Legend1";
            this.chartSpectrum.Legends.Add(legend1);
            this.chartSpectrum.Location = new System.Drawing.Point(9, 12);
            this.chartSpectrum.Name = "chartSpectrum";
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series1.Color = System.Drawing.Color.Red;
            series1.Legend = "Legend1";
            series1.LegendText = "Spectrum";
            series1.Name = "seriesSpectrum";
            series2.ChartArea = "ChartArea1";
            series2.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series2.Color = System.Drawing.Color.FromArgb(((int)(((byte)(0)))), ((int)(((byte)(0)))), ((int)(((byte)(192)))));
            series2.Legend = "Legend1";
            series2.LegendText = "Irradiance";
            series2.Name = "seriesIrradiance";
            series2.YAxisType = System.Windows.Forms.DataVisualization.Charting.AxisType.Secondary;
            this.chartSpectrum.Series.Add(series1);
            this.chartSpectrum.Series.Add(series2);
            this.chartSpectrum.Size = new System.Drawing.Size(664, 377);
            this.chartSpectrum.TabIndex = 0;
            this.chartSpectrum.Text = "Spectrum";
            // 
            // btnScan
            // 
            this.btnScan.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.btnScan.Location = new System.Drawing.Point(6, 19);
            this.btnScan.Name = "btnScan";
            this.btnScan.Size = new System.Drawing.Size(75, 23);
            this.btnScan.TabIndex = 1;
            this.btnScan.Text = "Start";
            this.btnScan.UseVisualStyleBackColor = true;
            this.btnScan.Click += new System.EventHandler(this.buttonScan_Click);
            // 
            // checkBoxEnableIrradiance
            // 
            this.checkBoxEnableIrradiance.AutoSize = true;
            this.checkBoxEnableIrradiance.Enabled = false;
            this.checkBoxEnableIrradiance.Location = new System.Drawing.Point(6, 234);
            this.checkBoxEnableIrradiance.Name = "checkBoxEnableIrradiance";
            this.checkBoxEnableIrradiance.Size = new System.Drawing.Size(73, 17);
            this.checkBoxEnableIrradiance.TabIndex = 2;
            this.checkBoxEnableIrradiance.Text = "Irradiance";
            this.checkBoxEnableIrradiance.UseVisualStyleBackColor = true;
            this.checkBoxEnableIrradiance.Click += new System.EventHandler(this.checkBoxEnableIrradiance_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox1.Controls.Add(this.buttonSave);
            this.groupBox1.Controls.Add(this.buttonReset);
            this.groupBox1.Controls.Add(this.buttonSaveDark);
            this.groupBox1.Controls.Add(this.btnScan);
            this.groupBox1.Location = new System.Drawing.Point(9, 395);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(688, 54);
            this.groupBox1.TabIndex = 3;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Controls";
            // 
            // buttonSave
            // 
            this.buttonSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSave.Location = new System.Drawing.Point(168, 19);
            this.buttonSave.Name = "buttonSave";
            this.buttonSave.Size = new System.Drawing.Size(75, 23);
            this.buttonSave.TabIndex = 5;
            this.buttonSave.Text = "Save";
            this.buttonSave.UseVisualStyleBackColor = true;
            this.buttonSave.Click += new System.EventHandler(this.buttonSave_Click);
            // 
            // buttonReset
            // 
            this.buttonReset.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonReset.Location = new System.Drawing.Point(87, 19);
            this.buttonReset.Name = "buttonReset";
            this.buttonReset.Size = new System.Drawing.Size(75, 23);
            this.buttonReset.TabIndex = 4;
            this.buttonReset.Text = "Reset";
            this.buttonReset.UseVisualStyleBackColor = true;
            this.buttonReset.Click += new System.EventHandler(this.buttonReset_Click);
            // 
            // buttonSaveDark
            // 
            this.buttonSaveDark.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.buttonSaveDark.Location = new System.Drawing.Point(249, 19);
            this.buttonSaveDark.Name = "buttonSaveDark";
            this.buttonSaveDark.Size = new System.Drawing.Size(75, 23);
            this.buttonSaveDark.TabIndex = 3;
            this.buttonSaveDark.Text = "Take Dark";
            this.buttonSaveDark.UseVisualStyleBackColor = true;
            this.buttonSaveDark.Click += new System.EventHandler(this.buttonSaveDark_Click);
            // 
            // groupBox2
            // 
            this.groupBox2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox2.Controls.Add(this.numericUpDownTriggerMode);
            this.groupBox2.Controls.Add(this.label8);
            this.groupBox2.Controls.Add(this.numericUpDownScansToAverage);
            this.groupBox2.Controls.Add(this.label5);
            this.groupBox2.Controls.Add(this.checkBoxEnableIrradiance);
            this.groupBox2.Controls.Add(this.numericUpDownBoxcarHalfWidth);
            this.groupBox2.Controls.Add(this.label4);
            this.groupBox2.Controls.Add(this.checkBoxEnableNLC);
            this.groupBox2.Controls.Add(this.checkBoxEnableEDC);
            this.groupBox2.Controls.Add(this.numericUpDownIntegrationTimeMillisec);
            this.groupBox2.Controls.Add(this.label3);
            this.groupBox2.Controls.Add(this.labelN);
            this.groupBox2.Controls.Add(this.labelPixels);
            this.groupBox2.Controls.Add(this.label2);
            this.groupBox2.Controls.Add(this.labelType);
            this.groupBox2.Controls.Add(this.label1);
            this.groupBox2.Controls.Add(this.labelSerialNumber);
            this.groupBox2.Location = new System.Drawing.Point(679, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(157, 255);
            this.groupBox2.TabIndex = 4;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Spectrometer";
            // 
            // numericUpDownTriggerMode
            // 
            this.numericUpDownTriggerMode.Location = new System.Drawing.Point(6, 181);
            this.numericUpDownTriggerMode.Maximum = new decimal(new int[] {
            4,
            0,
            0,
            0});
            this.numericUpDownTriggerMode.Name = "numericUpDownTriggerMode";
            this.numericUpDownTriggerMode.Size = new System.Drawing.Size(73, 20);
            this.numericUpDownTriggerMode.TabIndex = 15;
            this.numericUpDownTriggerMode.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownTriggerMode.ValueChanged += new System.EventHandler(this.numericUpDownTriggerMode_ValueChanged);
            // 
            // label8
            // 
            this.label8.AutoSize = true;
            this.label8.Location = new System.Drawing.Point(81, 183);
            this.label8.Name = "label8";
            this.label8.Size = new System.Drawing.Size(70, 13);
            this.label8.TabIndex = 16;
            this.label8.Text = "Trigger Mode";
            // 
            // numericUpDownScansToAverage
            // 
            this.numericUpDownScansToAverage.Location = new System.Drawing.Point(6, 155);
            this.numericUpDownScansToAverage.Maximum = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.numericUpDownScansToAverage.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownScansToAverage.Name = "numericUpDownScansToAverage";
            this.numericUpDownScansToAverage.Size = new System.Drawing.Size(73, 20);
            this.numericUpDownScansToAverage.TabIndex = 12;
            this.numericUpDownScansToAverage.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownScansToAverage.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownScansToAverage.ValueChanged += new System.EventHandler(this.numericUpDownScansToAverage_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(81, 157);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(75, 13);
            this.label5.TabIndex = 13;
            this.label5.Text = "Scans To Avg";
            // 
            // numericUpDownBoxcarHalfWidth
            // 
            this.numericUpDownBoxcarHalfWidth.Location = new System.Drawing.Point(6, 129);
            this.numericUpDownBoxcarHalfWidth.Name = "numericUpDownBoxcarHalfWidth";
            this.numericUpDownBoxcarHalfWidth.Size = new System.Drawing.Size(73, 20);
            this.numericUpDownBoxcarHalfWidth.TabIndex = 10;
            this.numericUpDownBoxcarHalfWidth.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(81, 131);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(40, 13);
            this.label4.TabIndex = 11;
            this.label4.Text = "Boxcar";
            // 
            // checkBoxEnableNLC
            // 
            this.checkBoxEnableNLC.AutoSize = true;
            this.checkBoxEnableNLC.Location = new System.Drawing.Point(60, 211);
            this.checkBoxEnableNLC.Name = "checkBoxEnableNLC";
            this.checkBoxEnableNLC.Size = new System.Drawing.Size(47, 17);
            this.checkBoxEnableNLC.TabIndex = 9;
            this.checkBoxEnableNLC.Text = "NLC";
            this.checkBoxEnableNLC.UseVisualStyleBackColor = true;
            // 
            // checkBoxEnableEDC
            // 
            this.checkBoxEnableEDC.AutoSize = true;
            this.checkBoxEnableEDC.Location = new System.Drawing.Point(8, 211);
            this.checkBoxEnableEDC.Name = "checkBoxEnableEDC";
            this.checkBoxEnableEDC.Size = new System.Drawing.Size(48, 17);
            this.checkBoxEnableEDC.TabIndex = 8;
            this.checkBoxEnableEDC.Text = "EDC";
            this.checkBoxEnableEDC.UseVisualStyleBackColor = true;
            // 
            // numericUpDownIntegrationTimeMillisec
            // 
            this.numericUpDownIntegrationTimeMillisec.DecimalPlaces = 3;
            this.numericUpDownIntegrationTimeMillisec.Increment = new decimal(new int[] {
            1,
            0,
            0,
            196608});
            this.numericUpDownIntegrationTimeMillisec.Location = new System.Drawing.Point(6, 103);
            this.numericUpDownIntegrationTimeMillisec.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMillisec.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMillisec.Name = "numericUpDownIntegrationTimeMillisec";
            this.numericUpDownIntegrationTimeMillisec.Size = new System.Drawing.Size(73, 20);
            this.numericUpDownIntegrationTimeMillisec.TabIndex = 6;
            this.numericUpDownIntegrationTimeMillisec.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.numericUpDownIntegrationTimeMillisec.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numericUpDownIntegrationTimeMillisec.ValueChanged += new System.EventHandler(this.numericUpDownIntegrationTimeMillisec_ValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(81, 105);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(53, 13);
            this.label3.TabIndex = 7;
            this.label3.Text = "Integ (ms)";
            // 
            // labelN
            // 
            this.labelN.AutoSize = true;
            this.labelN.Location = new System.Drawing.Point(78, 82);
            this.labelN.Name = "labelN";
            this.labelN.Size = new System.Drawing.Size(34, 13);
            this.labelN.TabIndex = 5;
            this.labelN.Text = "Pixels";
            // 
            // labelPixels
            // 
            this.labelPixels.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelPixels.Location = new System.Drawing.Point(6, 81);
            this.labelPixels.Name = "labelPixels";
            this.labelPixels.Size = new System.Drawing.Size(73, 19);
            this.labelPixels.TabIndex = 4;
            this.labelPixels.Text = "0";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 63);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(36, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Model";
            // 
            // labelType
            // 
            this.labelType.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelType.Location = new System.Drawing.Point(6, 48);
            this.labelType.Name = "labelType";
            this.labelType.Size = new System.Drawing.Size(121, 15);
            this.labelType.TabIndex = 2;
            this.labelType.Text = "unknown";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(6, 31);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(73, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Serial Number";
            // 
            // labelSerialNumber
            // 
            this.labelSerialNumber.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.labelSerialNumber.Location = new System.Drawing.Point(6, 16);
            this.labelSerialNumber.Name = "labelSerialNumber";
            this.labelSerialNumber.Size = new System.Drawing.Size(121, 15);
            this.labelSerialNumber.TabIndex = 0;
            this.labelSerialNumber.Text = "unknownSN";
            // 
            // groupBox3
            // 
            this.groupBox3.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox3.Controls.Add(this.textBoxEventLog);
            this.groupBox3.Location = new System.Drawing.Point(9, 455);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(826, 89);
            this.groupBox3.TabIndex = 5;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Event Log";
            // 
            // textBoxEventLog
            // 
            this.textBoxEventLog.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textBoxEventLog.Location = new System.Drawing.Point(12, 19);
            this.textBoxEventLog.Multiline = true;
            this.textBoxEventLog.Name = "textBoxEventLog";
            this.textBoxEventLog.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBoxEventLog.Size = new System.Drawing.Size(808, 64);
            this.textBoxEventLog.TabIndex = 0;
            // 
            // backgroundWorkerAcquisition
            // 
            this.backgroundWorkerAcquisition.WorkerReportsProgress = true;
            this.backgroundWorkerAcquisition.WorkerSupportsCancellation = true;
            this.backgroundWorkerAcquisition.DoWork += new System.ComponentModel.DoWorkEventHandler(this.backgroundWorkerAcquisition_DoWork);
            this.backgroundWorkerAcquisition.ProgressChanged += new System.ComponentModel.ProgressChangedEventHandler(this.backgroundWorkerAcquisition_ProgressChanged);
            this.backgroundWorkerAcquisition.RunWorkerCompleted += new System.ComponentModel.RunWorkerCompletedEventHandler(this.backgroundWorkerAcquisition_RunWorkerCompleted);
            // 
            // saveFileDialogSpectra
            // 
            this.saveFileDialogSpectra.DefaultExt = "csv";
            // 
            // MouseOverTT
            // 
            this.MouseOverTT.AutomaticDelay = 0;
            this.MouseOverTT.UseAnimation = false;
            this.MouseOverTT.UseFading = false;
            // 
            // PersistenceUpDown
            // 
            this.PersistenceUpDown.Enabled = false;
            this.PersistenceUpDown.Location = new System.Drawing.Point(8, 38);
            this.PersistenceUpDown.Maximum = new decimal(new int[] {
            200,
            0,
            0,
            0});
            this.PersistenceUpDown.Minimum = new decimal(new int[] {
            3,
            0,
            0,
            0});
            this.PersistenceUpDown.Name = "PersistenceUpDown";
            this.PersistenceUpDown.Size = new System.Drawing.Size(49, 20);
            this.PersistenceUpDown.TabIndex = 16;
            this.PersistenceUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.MouseOverTT.SetToolTip(this.PersistenceUpDown, "The number of adjacent data points that must be lower");
            this.PersistenceUpDown.Value = new decimal(new int[] {
            20,
            0,
            0,
            0});
            this.PersistenceUpDown.ValueChanged += new System.EventHandler(this.PersistenceUpDown_ValueChanged);
            // 
            // IntensityUpDown
            // 
            this.IntensityUpDown.Enabled = false;
            this.IntensityUpDown.Location = new System.Drawing.Point(8, 64);
            this.IntensityUpDown.Maximum = new decimal(new int[] {
            2000,
            0,
            0,
            0});
            this.IntensityUpDown.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.IntensityUpDown.Name = "IntensityUpDown";
            this.IntensityUpDown.Size = new System.Drawing.Size(47, 20);
            this.IntensityUpDown.TabIndex = 17;
            this.IntensityUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.MouseOverTT.SetToolTip(this.IntensityUpDown, "The intensity (Counts) above which a point is");
            this.IntensityUpDown.Value = new decimal(new int[] {
            200,
            0,
            0,
            0});
            this.IntensityUpDown.ValueChanged += new System.EventHandler(this.IntensityUpDown_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(56, 40);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(62, 13);
            this.label6.TabIndex = 18;
            this.label6.Text = "Persistence";
            this.MouseOverTT.SetToolTip(this.label6, "The number of adjacent data points that must be lower\r\nthan the wavelength in que" +
                    "stion. Higher values mean\r\nmore adjacent points must be below this wavelength to" +
                    "\r\nbe considered a \'Peak\'.\r\n\r\n");
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(56, 66);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(74, 13);
            this.label7.TabIndex = 19;
            this.label7.Text = "Intensity Delta";
            this.MouseOverTT.SetToolTip(this.label7, resources.GetString("label7.ToolTip"));
            // 
            // PeakFindCheckBox
            // 
            this.PeakFindCheckBox.AutoSize = true;
            this.PeakFindCheckBox.Location = new System.Drawing.Point(7, 19);
            this.PeakFindCheckBox.Name = "PeakFindCheckBox";
            this.PeakFindCheckBox.Size = new System.Drawing.Size(59, 17);
            this.PeakFindCheckBox.TabIndex = 15;
            this.PeakFindCheckBox.Text = "Enable";
            this.PeakFindCheckBox.UseVisualStyleBackColor = true;
            this.PeakFindCheckBox.CheckedChanged += new System.EventHandler(this.PeakFindCheckBox_CheckedChanged);
            // 
            // groupBox4
            // 
            this.groupBox4.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.groupBox4.Controls.Add(this.PeakFindCheckBox);
            this.groupBox4.Controls.Add(this.label7);
            this.groupBox4.Controls.Add(this.IntensityUpDown);
            this.groupBox4.Controls.Add(this.PersistenceUpDown);
            this.groupBox4.Controls.Add(this.label6);
            this.groupBox4.Location = new System.Drawing.Point(679, 273);
            this.groupBox4.Name = "groupBox4";
            this.groupBox4.Size = new System.Drawing.Size(157, 98);
            this.groupBox4.TabIndex = 20;
            this.groupBox4.TabStop = false;
            this.groupBox4.Text = "Peak Finding";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(848, 556);
            this.Controls.Add(this.groupBox4);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.chartSpectrum);
            this.Name = "Form1";
            this.Text = "SeaBreeze C# Demo";
            ((System.ComponentModel.ISupportInitialize)(this.chartSpectrum)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownTriggerMode)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownScansToAverage)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownBoxcarHalfWidth)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numericUpDownIntegrationTimeMillisec)).EndInit();
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.PersistenceUpDown)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.IntensityUpDown)).EndInit();
            this.groupBox4.ResumeLayout(false);
            this.groupBox4.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart chartSpectrum;
        private System.Windows.Forms.Button btnScan;
        private System.Windows.Forms.CheckBox checkBoxEnableIrradiance;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.NumericUpDown numericUpDownIntegrationTimeMillisec;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label labelN;
        private System.Windows.Forms.Label labelPixels;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label labelType;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label labelSerialNumber;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.TextBox textBoxEventLog;
        private System.ComponentModel.BackgroundWorker backgroundWorkerAcquisition;
        private System.Windows.Forms.Button buttonSaveDark;
        private System.Windows.Forms.NumericUpDown numericUpDownScansToAverage;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown numericUpDownBoxcarHalfWidth;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.CheckBox checkBoxEnableNLC;
        private System.Windows.Forms.CheckBox checkBoxEnableEDC;
        private System.Windows.Forms.Button buttonReset;
        private System.Windows.Forms.Button buttonSave;
        private System.Windows.Forms.SaveFileDialog saveFileDialogSpectra;
        private System.Windows.Forms.ToolTip MouseOverTT;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.NumericUpDown IntensityUpDown;
        private System.Windows.Forms.NumericUpDown PersistenceUpDown;
        private System.Windows.Forms.CheckBox PeakFindCheckBox;
        private System.Windows.Forms.GroupBox groupBox4;
        private System.Windows.Forms.NumericUpDown numericUpDownTriggerMode;
        private System.Windows.Forms.Label label8;
    }
}

