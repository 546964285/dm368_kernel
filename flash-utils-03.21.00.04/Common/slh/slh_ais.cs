/* --------------------------------------------------------------------------
    FILE        : slh.cs
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : Host console app for serial loading
 ----------------------------------------------------------------------------- */

using System;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using System.Globalization;
using TI.UtilLib;
using TI.UtilLib.IO;
using TI.UtilLib.CRC;
using TI.UtilLib.ConsoleUtility;
using TI.AISLib;

[assembly: AssemblyTitle("SerialLoaderHost")]
[assembly: AssemblyVersion("1.65.*")]


namespace TIBootAndFlash
{
  /// <summary>
  /// Enumeration for Magic Flags that the UBL expects to see
  /// </summary>
  public enum MagicFlags : uint
  {
    MAGIC_NUMBER_VALID = 0xA1ACED00,
    MAGIC_NUMBER_INVALID = 0xFFFFFFFF,
    SLT_MAGIC_LOADIMAGE = 0x53465400,  // Download a UBL-like image to execute directly in IRAM
  };

  /// <summary>
  /// Structure to hold command parameters
  /// </summary>
  struct ProgramCmdParams
  {
    /// <summary>
    /// Flag to indicate if command line is valid
    /// </summary>
    public Boolean valid;

    /// <summary>
    /// Boolean to control the verbosity of output
    /// </summary>
    public Boolean verbose;
    
    /// <summary>
    /// Name of serial port used for communications
    /// </summary>
    public String SerialPortName;

    /// <summary>
    /// This should be transmitted alone in response to the BOOTPSP.
    /// </summary>
    public MagicFlags CMDMagicFlag;

    /// <summary>
    /// String containing filename of Application file
    /// </summary>
    public String APPFileName;
    
    /// <summary>
    /// FLag to indicate that we should wait for device to prompt us
    /// </summary>
    public Boolean waitForBOOTME;
  }
  
  /// <summary>
  /// Main program Class
  /// </summary>
  partial class Program
  {
    //**********************************************************************************
    #region Class variables and members

    /// <summary>
    /// Global main Serial Port Object
    /// </summary>
    public static SerialPort MySP;
            
    /// <summary>
    /// The main thread used to actually execute everything
    /// </summary>
    public static Thread workerThread;

    /// <summary>
    /// Global boolean to indicate successful completion of workerThread
    /// </summary>
    public static Boolean workerThreadSucceeded = false;

    /// <summary>
    /// Public variable to hold needed command line and program parameters
    /// </summary>
    public static ProgramCmdParams cmdParams;

    #endregion
    //**********************************************************************************


    //**********************************************************************************
    #region Code for Main thread

    /// <summary>
    /// Help Display Function
    /// </summary>
    private static void DispHelp()
    {
      Console.Write("Usage:");
      Console.Write("\n\tslh_"+devString+" [<Options> [<Optional Params>]] <AIS Image File>");
      Console.Write("\n\t" + "<Options> can be the following: " +
                    "\n\t\t" +"-h                               \tDisplay this help screen."+
                    "\n\t\t" +"-v                               \tDisplay more verbose output returned from the "+devString+"."+
                    "\n\t\t" +"-waitForDevice                   \tWait for the BOOTME prompt from the target device."+
                    "\n\t\t" +"-p <PortName>                    \tUse <PortName> as the serial port (e.g. COM2, /dev/ttyS1).\n\n");
    }   
 
    /// <summary>
    /// Parse the command line into the appropriate internal command structure
    /// </summary>
    /// <param name="args">The array of strings passed to the command line.</param>
    public static ProgramCmdParams ParseCmdLine(String[] args)
    {
      ProgramCmdParams myCmdParams =  new ProgramCmdParams();
      Boolean[] argsHandled = new Boolean[args.Length];
      Int32 numFiles = -1;
      UInt32 numUnhandledArgs,numHandledArgs=0;
      String s;

      if (args.Length == 0)
      {
          myCmdParams.valid = false;
          return myCmdParams;
      }

      // Initialize array of handled argument booleans to false
      for (int i = 0; i < argsHandled.Length; i++ )
        argsHandled[i] = false;

      // Set Defualts for application
      myCmdParams.CMDMagicFlag = MagicFlags.MAGIC_NUMBER_INVALID;
      myCmdParams.valid = true;
      myCmdParams.verbose = false;
      myCmdParams.SerialPortName = null;

      myCmdParams.APPFileName = null;
      
      // For loop for required load type
      myCmdParams.CMDMagicFlag = MagicFlags.SLT_MAGIC_LOADIMAGE;
      numFiles = 1;
      
      if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
      {
        myCmdParams.valid = false;
        return myCmdParams;
      }

      // For loop for all other dash options
      for(int i = 0; i<args.Length; i++)
      {
        s = args[i];
        if ((s.StartsWith("-")) && (argsHandled[i] != true))
        {
          switch (s.Substring(1).ToLower())
          {
            case "p":
              myCmdParams.SerialPortName = args[i + 1];
              argsHandled[i + 1] = true;
              numHandledArgs++;
              break;
            case "v":
              myCmdParams.verbose = true;
              break;
            case "waitfordevice":
              myCmdParams.waitForBOOTME = true;
              break;
            default:
              myCmdParams.valid = false;
              break;
          }
          argsHandled[i] = true;
          numHandledArgs++;
          if (!myCmdParams.valid)
            return myCmdParams;
        }
      } // end of for loop for handling dash params
     
      // Verify that the number of unhandled arguments is equal to numFiles
      // If not, then there is a problem.
      numUnhandledArgs = (UInt32) (args.Length - numHandledArgs);
      if (numUnhandledArgs != numFiles)
      {
        myCmdParams.valid = false;
        return myCmdParams;
      }
                
      // This for loop handles all other params (namely filenames)
      for (int i = 0; i < args.Length; i++)
      {
        if (!argsHandled[i])
        {
          switch (numFiles)
          {
            case 1:
              if (myCmdParams.APPFileName == null)
              {
                myCmdParams.APPFileName = args[i];
              }
              else
                myCmdParams.valid = false;
              break;
            default:
              myCmdParams.valid = false;
              break;
          }
        }    
        argsHandled[i] = true;
        numHandledArgs++;
        if (!myCmdParams.valid) return myCmdParams;
      } // end of for loop handling file name inputs
      
      //Setup default serial port name
      if (myCmdParams.SerialPortName == null)
      {
        int p = (int)Environment.OSVersion.Platform;
        if ((p == 4) || (p == 128)) //Check for unix/linux
        {
          Console.WriteLine("Platform is Unix/Linux.");
          myCmdParams.SerialPortName = "/dev/ttyS0";
        }
        else
        {
          Console.WriteLine("Platform is Windows.");
          myCmdParams.SerialPortName = "COM1";
        }
      }
      return myCmdParams;
    }

    /// <summary>
    /// Main entry point of application
    /// </summary>
    /// <param name="args">Array of command-line arguments</param>
    /// <returns>Return code: 0 for correct exit, -1 for unexpected exit</returns>
    static Int32 Main(String[] args)
    {
      // Assumes that in AssemblyInfo.cs, the version is specified as 1.0.* or the like,
      // with only 2 numbers specified;  the next two are generated from the date.
      System.Version v = System.Reflection.Assembly.GetExecutingAssembly().GetName().Version;
      
      // v.Build is days since Jan. 1, 2000, v.Revision*2 is seconds since local midnight
      Int32 buildYear = new DateTime( v.Build * TimeSpan.TicksPerDay + v.Revision * TimeSpan.TicksPerSecond * 2 ).AddYears(1999).Year;
      
      // Begin main code
      Console.Clear();
      Console.WriteLine("-----------------------------------------------------");
      Console.WriteLine("   TI Serial Loader Host Program for " + devString    );
      Console.WriteLine("   (C) "+buildYear+", Texas Instruments, Inc."        );
      Console.WriteLine("   Ver. "+v.Major+"."+v.Minor.ToString("D2")          );
      Console.WriteLine("-----------------------------------------------------");
      Console.Write("\n\n");

      // Parse command line
      cmdParams = ParseCmdLine(args);
      if (!cmdParams.valid)
      {
        DispHelp();
        return -1;
      }
      
      try
      {
        Console.WriteLine("Attempting to connect to device " + cmdParams.SerialPortName + "...");
        MySP = new SerialPort(cmdParams.SerialPortName, 115200, Parity.None, 8, StopBits.One);
        MySP.Encoding = Encoding.ASCII;
        MySP.Open();
      }
      catch(Exception e)
      {
        if (e is UnauthorizedAccessException)
        {
          Console.WriteLine(e.Message);
          Console.WriteLine("This application failed to open the COM port.");
          Console.WriteLine("Most likely it is in use by some other application.");
          return -1;
        }
        Console.WriteLine(e.Message);
        return -1;
      }

      Console.WriteLine("Press any key to end this program at any time.\n");
      
      // Setup the thread that will actually do all the work of interfacing to
      // the Device boot ROM.  Start that thread.
      workerThread = new Thread(new ThreadStart(Program.WorkerThreadStart));
      workerThread.Start();

      // Wait for a key to terminate the program
      while ((workerThread.IsAlive) && (!Console.KeyAvailable))
      {
        Thread.Sleep(1000);
      }
                 
      // If a key is pressed then abort the worker thread and close the serial port
      try
      {
        if (Console.KeyAvailable)
        {
          Console.ReadKey();
          Console.WriteLine("Aborting program...");
          workerThread.Abort();
        }
        else if (workerThread.IsAlive)
        {
          Console.WriteLine("Aborting program...");
          workerThread.Abort();
        }
        
        while ((workerThread.ThreadState & ThreadState.Stopped) != ThreadState.Stopped){}
      }
      catch (Exception e)
      {
        Console.WriteLine("Abort thread error...");
        Console.WriteLine(e.GetType());
        Console.WriteLine(e.Message);
      }
      
      if (workerThreadSucceeded)
      {
        Console.WriteLine("\nOperation completed successfully.");
        return 0;
      }
      else
      {
        Console.WriteLine("\n\nInterfacing to the "+devString+" via UART failed." +
            "\nPlease reset or power-cycle the board and try again...");
        return -1;
      }
      
    }
    #endregion
    //**********************************************************************************
    

    #region Code for UART interfacing thread
    /// <summary>
    /// The main function of the thread where all the cool stuff happens
    /// to interface with the device
    /// </summary>
    public static void WorkerThreadStart()
    {
      switch (cmdParams.CMDMagicFlag)
      {
        case MagicFlags.SLT_MAGIC_LOADIMAGE:
        {
          TransmitAppToTarget();
          break;
        }
        default:
        {
          Console.WriteLine("Command not recognized!");
          break;
        }
      }
       
      // Everything worked, so change boolean status
      workerThreadSucceeded = true;
    }

    /// <summary>
    /// Function to Transmit the UBL via the device ROM Serial boot
    /// Doesn't need it anymore
    /// </summary>
    private static void TransmitAppToTarget()
    { 
      // Send the UBL/SFT data
      Console.WriteLine("Entering AIS Parser");
      
      AIS_Parser parser = new AIS_Parser(AisHostType.UART, AisSecureType.NONE, UTIL_log, UTIL_uartRead, UTIL_uartWrite);
      parser.waitBOOTME = cmdParams.waitForBOOTME;
      
      // Local Variables for holding file data
      Byte[] imageData;

      // Read the image data we will transmit
      imageData = FileIO.GetFileData(cmdParams.APPFileName);
      
      try
      {
      BOOTMESEQ:
        Console.WriteLine("\nWaiting for the "+devString+"...");
        // Start the ROM booting 
        if (AisStatus.ERROR == parser.boot(imageData)) 
        {
          Console.WriteLine("Booting AIS image failed. Trying again (you may need to reset the target)...");
          // Wait for the device to send the ^BOOTME/0 sequence
          goto BOOTMESEQ;
        }

      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }
    }
    #endregion 
    

    #region UART delegate implementations  
    public static void UTIL_log(String text)
    {
      Console.WriteLine(text);
    }

    private static AisStatus UTIL_uartRead(byte[] rcvBuf, int index, int rcvSize, int timeout)
    {
      int bytesRead = 0;

      MySP.ReadTimeout = timeout;
      try
      {
        while (bytesRead < rcvSize)
        {
          bytesRead += MySP.Read(rcvBuf, index + bytesRead, rcvSize - bytesRead);
        }
      }
      catch (Exception ex)
      {
        Console.WriteLine("(Serial Port): Read error! (" + ex.Message + ")");
        return AisStatus.ERROR;
      }
      return AisStatus.IN_PROGRESS;
    }

    private static AisStatus UTIL_uartWrite(byte[] xmtBuf, int index, int xmtSize, int timeout)
    {
      MySP.WriteTimeout = timeout;
      try
      {
        MySP.Write(xmtBuf, index, xmtSize);
      }
      catch (Exception ex)
      {
        Console.WriteLine("(Serial Port): Write error! (" + ex.Message + ")");
        return AisStatus.ERROR;
      }
      return AisStatus.IN_PROGRESS;
    }
    #endregion
    
  }
  
}
