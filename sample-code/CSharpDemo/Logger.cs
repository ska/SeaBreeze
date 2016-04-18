using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Windows.Forms;
using System.Diagnostics;
using System.Threading;
using System.Globalization;

public class Logger
{
    static Logger instance;

    TextBox textBox = null;
    List<string> bufferedMessages = new List<string>();
    static Mutex mut = new Mutex();

    static public Logger getInstance()
    {
        mut.WaitOne();
        if (instance == null)
            instance = new Logger();
        mut.ReleaseMutex();
        return instance;
    }

    // instantiate a logger with no TextBox (Console only)
    Logger()
    {
    }

    // instantiate a logger with the TextBox form control to receive updates
    public Logger(TextBox tb)
    {
        textBox = tb;
    }

    // assign or change textbox after construction
    public void setTextBox(TextBox tb)
    {
        textBox = tb;
    }

    // write (not append) log to textfile
    public void save(string pathname)
    {
        if (textBox != null)
        {
            try
            {
                TextWriter tw = new StreamWriter(pathname);
                tw.WriteLine(textBox.Text);
                tw.Close();
            }
            catch (Exception e)
            {
                display("Error saving log to {0}: {1}", pathname, e.Message);
            }
        }
    }

    // log to textbox and console
    public void display(string fmt, params Object[] obj)
    {
        string now = getTimestamp();
        string msg = String.Format(fmt, obj) + Environment.NewLine;
        Trace.Write("DISPLAY: " + now + msg);
        if (textBox != null)
            textBox.AppendText(now + msg);
    }

    string getTimestamp()
    {
        return DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff: ", CultureInfo.InvariantCulture);
    }

    public void push(string fmt, params Object[] obj)
    {
        string msg = getTimestamp() + String.Format(fmt, obj);
        mut.WaitOne();
        bufferedMessages.Add(msg);                
        mut.ReleaseMutex();
    }

    public void flush()
    {
        mut.WaitOne();
        foreach (string msg in bufferedMessages)
        {
            Trace.Write("DISPLAY: " + msg + Environment.NewLine);
            if (textBox != null)
                textBox.AppendText(msg + Environment.NewLine);
        }
        bufferedMessages.Clear();
        mut.ReleaseMutex();
    }

    // just log to console
    public void log(string fmt, params Object[] obj)
    {
        string msg = String.Format(fmt, obj) + Environment.NewLine;
        Trace.Write("LOG: ", msg);
    }
}