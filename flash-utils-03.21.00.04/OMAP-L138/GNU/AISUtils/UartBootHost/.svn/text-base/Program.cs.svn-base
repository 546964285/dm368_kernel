using System;
using System.Collections.Generic;
using System.Windows.Forms;

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
      System.Version v = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;

      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);

      FormUartHost mainForm = new FormUartHost();

      mainForm.Text = devString + " Secure UART Boot Host, Ver. "+v.Major+"."+v.Minor.ToString("D2") ;

      Application.Run(mainForm);
    }
  }
}