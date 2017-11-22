using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace OMAP_L138_AIS_GUI
{
    static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main(string[] args)
        {
            bool runOnce = false;
            string cfgFileName = "";

            // detect command line usage
            foreach (string arg in args)
            {
                if (arg.Substring(0, 5) == "-cfg=")
                {
                    runOnce = true;
                    cfgFileName = arg.Substring(5);
                    break;
                }
            }

            // launch GUI
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm(runOnce, cfgFileName));
        }
    }
}