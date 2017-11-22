using System;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using System.Globalization;
using TI.UtilLib.ConsoleUtility;

[assembly: AssemblyTitle("ConsoleUtilityTest")]
[assembly: AssemblyVersion("1.00.*")]


namespace TIBootAndFlash
{
  /// <summary>
  /// Main program Class
  /// </summary>
  class Program
  {
    /// <summary>
    /// Main entry point of application
    /// </summary>
    /// <param name="args">Array of command-line arguments</param>
    /// <returns>Return code: 0 for correct exit, -1 for unexpected exit</returns>
    static Int32 Main(String[] args)
    {
      ProgressBar progressBar;
      int blockCnt = 0;
      
      Console.WriteLine("ConsoleUtility Test Program.");
      Console.WriteLine("");
          
      // Test default progress Bar
      progressBar = new ProgressBar();
      progressBar.Update(0.0,"Testing Default ProgressBar");
      blockCnt = 15;
      for (int i = 0; i < blockCnt; i++)
      {
        Thread.Sleep(100);
        progressBar.Percent = (((Double)(i+1))/blockCnt);
        //Console.WriteLine(Console.CursorTop);
      }
      
      progressBar.Update(100.0,"Test Default ProgressBar complete.");
      
      
      // Test progress bar of specified length
      progressBar = new ProgressBar(40);
      progressBar.Update(0.0,"Testing ProgressBar(40)");
      blockCnt = 6;
      for (int i = 0; i < blockCnt; i++)
      {
        Thread.Sleep(100);
        progressBar.Percent = (((Double)(i+1))/blockCnt);
        //Console.WriteLine(Console.CursorTop);
        if (i == 4)
          progressBar.Write(i.ToString() + "\n\n\n" + "\n");
      }
      
      progressBar.Update(100.0,"Test ProgressBar(40) complete.");
      
        
      
      // Test progress bar of specified length and position
      progressBar = new ProgressBar(50,Position.RIGHT,Position.TOP);
      progressBar.Update(0.0,"Testing ProgressBar(50, RIGHT, TOP)");
      blockCnt = 10;
      for (int i = 0; i < blockCnt; i++)
      {
        Thread.Sleep(300);
        progressBar.Percent = (((Double)(i+1))/blockCnt);
        //Console.WriteLine(Console.CursorTop);
      }
      
      progressBar.Update(100.0,"Test ProgressBar(50, RIGHT, TOP) complete.");
      
      
      
      return 0;
    }
  }
}