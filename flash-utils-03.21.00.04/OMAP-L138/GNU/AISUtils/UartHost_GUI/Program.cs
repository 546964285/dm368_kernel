using System;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Windows.Forms;


// General Information about an assembly is controlled through the following 
// set of attributes. Change these attribute values to modify the information
// associated with an assembly.
[assembly: AssemblyTitle("UartHost")]
[assembly: AssemblyDescription("Host PC program to boot device via serial port.")]
[assembly: AssemblyConfiguration("")]
[assembly: AssemblyCompany("Texas Instruments, Inc.")]
[assembly: AssemblyProduct("UartHost")]
[assembly: AssemblyCopyright("Copyright © Texas Instruments, Inc.")]
[assembly: AssemblyTrademark("")]
[assembly: AssemblyCulture("")]

// Version information for an assembly consists of the following four values:
//
//      Major Version
//      Minor Version 
//      Build Number
//      Revision
//
[assembly: AssemblyVersion("1.0.*")]
[assembly: AssemblyFileVersion("1.1.0.0")]

namespace TIBootAndFlash
{
  partial class Program
  {
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main()
    {
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);

      System.Version v = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
      FormUartHost mainForm = new FormUartHost();

      // Set form text
      mainForm.Text = devString + " UART Boot Host, Ver. "+v.Major+"."+v.Minor.ToString() ;
      
      Application.Run(mainForm);
    }
  }
}