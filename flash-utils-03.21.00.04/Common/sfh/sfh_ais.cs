/* --------------------------------------------------------------------------
    FILE        : sfh_ais.cs
    PROJECT     : TI Booting and Flashing Utilities
    AUTHOR      : Jeff Cobb, Daniel Allred
    DESC        : Host program for flashing via serial port
 ----------------------------------------------------------------------------- */

using System;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using System.Globalization;
using TI.UtilLib;
using TI.UtilLib.CRC;
using TI.UtilLib.IO;
using TI.UtilLib.ConsoleUtility;
using TI.AISLib;

[assembly: AssemblyTitle("SerialFlasherHost")]
[assembly: AssemblyVersion("1.67.*")]


namespace TIBootAndFlash
{
  /// <summary>
  /// Enumeration for Magic Flags that the UBL expects to see
  /// </summary>
  public enum MagicFlags : uint
  {
    MAGIC_NUMBER_INVALID    = 0xFFFFFFFF,
   
    UBL_MAGIC_FLASH_NO_UBL  = 0x55424C00,       /* Download via UART & Burn single boot image only */
    UBL_MAGIC_FLASH         = 0x55424C10,       /* Download via UART & Burn UBL and application image */
    UBL_MAGIC_ERASE         = 0x55424C20,       /* Download via UART & globally erase the flash */
	UBL_MAGIC_FLASH_DSP	    = 0x55424C30,		/* Download via UART & Burn DSP and ARM UBL, and application image */    
    UBL_MAGIC_BINARY_BOOT   = 0x55424CBB,             /* App image is a bootable binary */
    UBL_MAGIC_BINARY_BOOT_LEG = 0xA1ACED00,             /* Legacy Magic Number: App image is a bootable binary */
    UBL_MAGIC_FINISHED      = 0x55424CFF
  };

  /// <summary>
  /// Enumeration of flash types
  /// </summary>
  public enum FlashType : uint
  {
    NONE,
    NOR,
    NAND,
    ONENAND,
    SD_MMC,
    SPI_MEM,
    I2C_MEM
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
    /// Baud Rate of serial port
    /// </summary>
    public Int32 SerialPortBaudRate;

    /// <summary>
    /// This should be transmitted alone in response to the BOOTUBL.
    /// </summary>
    public MagicFlags CMDMagicFlag;

    /// <summary>
    /// Type of flash that the application and UBL are targeted to use.  Selects
    /// which embedded UBL to use.
    /// </summary>
    public FlashType UBLFlashType;

    /// <summary>
    /// Global varibale to hold the desired magic flag
    /// </summary>
    public MagicFlags UBLMagicFlag;
    public MagicFlags DSPUBLMagicFlag;
    public MagicFlags ARMUBLMagicFlag;

    /// <summary>
    /// String containing filename of FLASH UBL file (only needed for flashing)
    /// </summary>
    public String UBLFileName;
    public String DSPUBLFileName;
    public String ARMUBLFileName;

	/// <summary>
    /// Address at which the Flash UBL will begin execution (must be 0x100 or greater)
    /// </summary>
    public UInt32 UBLStartAddr;
    public UInt32 ARMUBLStartAddr;

    /// <summary>
    /// Magic Flag for the application data
    /// </summary>
    public MagicFlags APPMagicFlag;
    public MagicFlags APPMagicFlagLeg;

    /// <summary>
    /// String containing filename of Application file
    /// </summary>
    public String APPFileName;

    /// <summary>
    /// Start address of where the app image should be loaded
    /// </summary>
    public UInt32 APPLoadAddr;

    /// <summary>
    /// Address where the app begin execution 
    /// </summary>
    public UInt32 APPStartAddr;
    
    public String deviceType;
    public String flashType;
  }
    
  struct ImageHeader
  {
    public UInt32 magicNum;      
    public UInt32 startAddr;
    public UInt32 loadAddr;
    public UInt32 byteCnt;
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

    /// <summary>
    /// String to hold the summary of operation program will attempt.
    /// </summary>
    public static String cmdString;

    #endregion
    //**********************************************************************************


    //**********************************************************************************
    #region Code for Main thread

    /// <summary>
    /// Help Display Function
    /// </summary>
    private static void DispHelp()
    {
      String programName = "sfh_" + devString;
      String commandLine = programName+" <Command> ";
      String targetString = "", flashString = "";
      
      if (deviceTypes.Length > 1)
      {
        commandLine = commandLine + "[-targetType <Target>] ";
        targetString = String.Join(", ",deviceTypes) + " (default is " + deviceTypes[0] + ")";
      }
      if (flashTypes.Length > 1)
      {
        commandLine = commandLine + "[-flashType <FlashType>] ";
        flashString  = String.Join(", ",flashTypes)  + " (default is " + flashTypes[0]  + ")";
      }
      commandLine = commandLine +  "[<Options>] [<InputFiles]";
    
      Console.WriteLine("Usage:");
      Console.WriteLine();
      Console.WriteLine(commandLine);
      Console.WriteLine("\t<Command> is required and can be one of the following: ");
      Console.WriteLine("\t\t-erase      \tGlobal erase of the flash memory device (no input files)");
      Console.WriteLine("\t\t-flash_noubl\tPlace single bootable image in the flash memory device (single input file)");
      Console.WriteLine("\t\t-flash      \tPlace a secondary user boot loader (UBL) and application image in the");
      Console.WriteLine("\t\t            \tflash memory device (first input file is UBL binary, second input ");
      Console.WriteLine("\t\t            \tfile is the binary application image)");
	  Console.WriteLine("\t\t-flash_dsp  \tPlace a DSP secondary user boot loader (UBL), an ARM secondary user boot loader");
      Console.WriteLine("\t\t            \tand the application image in the flash memory device (first input file is DSP");
	  Console.WriteLine("\t\t            \tUBL binary, the second file is ARM UBL binary, and the third input file is");
      Console.WriteLine("\t\t            \tthe binary application image). This is only used for OMAP-L137 devices.");
	  
      Console.WriteLine();
      if (deviceTypes.Length > 1)
      {
      Console.WriteLine("\t-targetType <Target>  \tSpecify the exact target type within the "+devString+" family.");
      Console.WriteLine("\t\t<Target>            \tOne of " + targetString );
      Console.WriteLine();
      }
      if (flashTypes.Length > 1)
      {
      Console.WriteLine("\t-flashType <Flash>    \tSpecify exact flash type among supported types for the platform.");
      Console.WriteLine("\t\t<Flash>             \tOne of " + flashString );
      Console.WriteLine();
      }
      Console.WriteLine("\t<Options> can be the following: ");
      Console.WriteLine("\t\t-h                  \tDisplay this help screen.");
      Console.WriteLine("\t\t-v                  \tDisplay more verbose output returned from the target device");
      Console.WriteLine("\t\t-p <PortName>       \tUse <PortName> as the serial port (e.g. COM2, /dev/ttyS1).");
      Console.WriteLine("\t\t-baud <BaudRate     \tUses <BaudRate> as the serial port baud rate (defaults to 115200)");
      Console.WriteLine("\t\t-appStartAddr <Application entry point address>\tSpecify in hex (defaults to 0xC1080000)");
      Console.WriteLine("\t\t-appLoadAddr  <Application image load address> \tSpecify in hex (defaults to 0xC1080000)");      
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
      myCmdParams.UBLFlashType = FlashType.NONE;
      
      myCmdParams.CMDMagicFlag = MagicFlags.MAGIC_NUMBER_INVALID;
      myCmdParams.valid = true;
      myCmdParams.verbose = false;
      myCmdParams.SerialPortName = null;
      myCmdParams.SerialPortBaudRate = 115200;

      myCmdParams.deviceType = null;
      myCmdParams.flashType = null;
      
      myCmdParams.APPMagicFlag = MagicFlags.UBL_MAGIC_BINARY_BOOT;
      myCmdParams.APPMagicFlagLeg = MagicFlags.UBL_MAGIC_BINARY_BOOT_LEG;
      myCmdParams.APPFileName = null;
      myCmdParams.APPLoadAddr = 0xFFFFFFFF;
      myCmdParams.APPStartAddr = 0xFFFFFFFF;

      myCmdParams.UBLMagicFlag = MagicFlags.UBL_MAGIC_BINARY_BOOT;
      myCmdParams.DSPUBLMagicFlag = MagicFlags.UBL_MAGIC_BINARY_BOOT;
      myCmdParams.ARMUBLMagicFlag = MagicFlags.UBL_MAGIC_BINARY_BOOT;


      myCmdParams.UBLFileName = null;
      myCmdParams.DSPUBLFileName = null;
      myCmdParams.ARMUBLFileName = null;

      myCmdParams.UBLStartAddr = 0xFFFFFFFF;

      

      // For loop for required command-line params
      for(int i = 0; i<args.Length; i++)
      {
        s = args[i];
        if (s.StartsWith("-"))
        {
          switch (s.Substring(1).ToLower())
          {
            case "flash":
              if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
              {
                myCmdParams.CMDMagicFlag = MagicFlags.UBL_MAGIC_FLASH;
              }
              else
              {
                myCmdParams.valid = false;
              }
              numFiles = 2;
              cmdString += "      [TYPE] UBL and application image\n";
              break;
            case "flash_noubl":
              if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
              {
                myCmdParams.CMDMagicFlag = MagicFlags.UBL_MAGIC_FLASH_NO_UBL;
              }
              else
              {
                myCmdParams.valid = false;
              }
              numFiles = 1;
			  cmdString += "      [TYPE] Single boot image\n";
			  break;
            case "flash_dsp":
              if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
              {
                myCmdParams.CMDMagicFlag = MagicFlags.UBL_MAGIC_FLASH_DSP;
              }
              else
              {
                myCmdParams.valid = false;
              }
              numFiles = 3;
			  cmdString += "      [TYPE] DSP UBL, ARM UBL, and application image\n";
			  break;
            case "erase":
              if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
              {
                myCmdParams.CMDMagicFlag = MagicFlags.UBL_MAGIC_ERASE;
              }
              else
              {
                myCmdParams.valid = false;
              }
              cmdString += "      [TYPE] Global erase\n";
              numFiles = 0;
              break;
            default:
              continue;
          }
          argsHandled[i] = true;
          numHandledArgs++;
          
          if (!myCmdParams.valid)
            return myCmdParams;
        }
      } // end of for loop for handling dash params
      
      // Check to make sure a command was selected
      if (myCmdParams.CMDMagicFlag == MagicFlags.MAGIC_NUMBER_INVALID)
      {
        myCmdParams.valid = false;
        return myCmdParams;
      }
      
      // For loop for optional command-line params
      try
      {
        for(int i = 0; i<args.Length; i++)
        {
          Int32 index;
          s = args[i];
          if ((s.StartsWith("-")) && (argsHandled[i] != true))
          {
            switch (s.Substring(1).ToLower())
            {
              case "targettype":
                index = Array.IndexOf<String>(deviceTypes,args[i + 1]);
                myCmdParams.deviceType = (index < 0)?deviceTypes[0]:deviceTypes[index];
                if (index < 0)
                {
                  Console.WriteLine("Target type not recongnized");
                  myCmdParams.valid = false;
                }
                argsHandled[i + 1] = true;
                numHandledArgs++;

				break;
              case "flashtype":
                index = Array.IndexOf<String>(flashTypes,args[i + 1]);
                myCmdParams.flashType = (index < 0)?flashTypes[0]:flashTypes[index];
                if (index < 0)
                {
                  Console.WriteLine("Flash type not recongnized, using default ({0}).",myCmdParams.flashType);
                }
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;          
              case "appstartaddr":
                if (args[i + 1].StartsWith("0x"))
                  args[i + 1] = args[i + 1].Substring(2);
                myCmdParams.APPStartAddr = System.UInt32.Parse(args[i + 1],System.Globalization.NumberStyles.AllowHexSpecifier);
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;
              case "apploadaddr":
                if (args[i + 1].StartsWith("0x"))
                  args[i + 1] = args[i + 1].Substring(2);
                myCmdParams.APPLoadAddr = System.UInt32.Parse(args[i + 1],System.Globalization.NumberStyles.AllowHexSpecifier);
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;
              case "ublstartaddr":
                if (args[i + 1].StartsWith("0x"))
                  args[i + 1] = args[i + 1].Substring(2);
                myCmdParams.UBLStartAddr = System.UInt32.Parse(args[i + 1],System.Globalization.NumberStyles.AllowHexSpecifier);
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;              
              case "p":
                myCmdParams.SerialPortName = args[i + 1];
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;
              case "baud":
                myCmdParams.SerialPortBaudRate = System.Int32.Parse(args[i + 1]);
                argsHandled[i + 1] = true;
                numHandledArgs++;
                break;                
              case "v":
                myCmdParams.verbose = true;
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
      }
      catch(Exception e)
      {
        Console.WriteLine(e.Message);
        myCmdParams.valid = false;
        return myCmdParams;
      }
      
      // Verify that the number of unhandled arguments is equal to numFiles
      // If not, then there is a problem.
      numUnhandledArgs = (UInt32) (args.Length - numHandledArgs);
      if (numUnhandledArgs != numFiles)
      {
        myCmdParams.valid = false;
        return myCmdParams;
      }
       
      // This for loop handles all othe params (namely filenames)
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
                cmdString += "[BOOT IMAGE] " + myCmdParams.APPFileName + "\n" ;
              }
              else
                myCmdParams.valid = false;
              break;
            case 2:
              if (myCmdParams.UBLFileName == null)
              {
                myCmdParams.UBLFileName = args[i];
                cmdString += "       [UBL] "+ myCmdParams.UBLFileName + "\n";
              }
              else if (myCmdParams.APPFileName == null)
              {
                myCmdParams.APPFileName = args[i];
                cmdString += " [APP IMAGE] " + myCmdParams.APPFileName + "\n";
              }
              else
                myCmdParams.valid = false;
              break;
			case 3:
              if (myCmdParams.DSPUBLFileName == null)
              {
                myCmdParams.DSPUBLFileName = args[i];
                cmdString += "   [DSP UBL] "+ myCmdParams.DSPUBLFileName + "\n";
              }
              else if (myCmdParams.ARMUBLFileName == null)
              {
                myCmdParams.ARMUBLFileName = args[i];
                cmdString += "   [ARM UBL] "+ myCmdParams.ARMUBLFileName + "\n";
              }
			  else if (myCmdParams.APPFileName == null)
              {
                myCmdParams.APPFileName = args[i];
                cmdString += " [APP IMAGE] " + myCmdParams.APPFileName + "\n";
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
        if (!myCmdParams.valid) 
          return myCmdParams;
      } // end of for loop for handling dash params
	  
      // Set default binary execution address on target device
      if (myCmdParams.APPLoadAddr == 0xFFFFFFFF)
      {
        myCmdParams.APPLoadAddr = externalRAMStart + 0x01080000;
      }

      if (myCmdParams.APPStartAddr == 0xFFFFFFFF)
      {
        myCmdParams.APPStartAddr = externalRAMStart + 0x01080000;
      }
          
      if (myCmdParams.UBLStartAddr == 0xFFFFFFFF)
        myCmdParams.UBLStartAddr = 0x00000100;
      
	  myCmdParams.ARMUBLStartAddr = 0x80000000;
        
      if (myCmdParams.deviceType == null)
      {
        myCmdParams.deviceType = deviceTypes[0];
      }
      
      if (myCmdParams.flashType == null)
      {
        myCmdParams.flashType = flashTypes[0];
      }
	  
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
      
      // Prep command string 
	  cmdString += "    [TARGET] " + myCmdParams.deviceType + "\n";                 
      cmdString = cmdString + "    [DEVICE] " + myCmdParams.flashType;
      
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
      Console.WriteLine("-----------------------------------------------------");
      Console.WriteLine("   TI Serial Flasher Host Program for " + devString   );
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
      else
      {
        Console.Write(cmdString + "\n\n");
      }
                             
      try
      {
        Console.WriteLine("Attempting to connect to device " + cmdParams.SerialPortName + "...");
        MySP = new SerialPort(cmdParams.SerialPortName, cmdParams.SerialPortBaudRate, Parity.None, 8, StopBits.One);
        MySP.ReadTimeout=1;
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
      

    //**********************************************************************************
    #region Code for UART interfacing thread

    /// <summary>
    /// The main function of the thread where all the cool stuff happens
    /// to interface with the device
    /// </summary>
   
    public static void WorkerThreadStart()
    {
      Boolean status;
      
      // Try transmitting the first stage boot-loader (the SFT) via the RBL
      try
      {
        String srchStr;
        Byte[] imageData;

        srchStr = "sft_"+cmdParams.deviceType+"_"+cmdParams.flashType+".bin";

        // Get the embedded SFT image data
        imageData = EmbeddedFileIO.ExtractFileBytes(System.Reflection.Assembly.GetExecutingAssembly(), srchStr); 
        
        TransmitSFT(imageData);
      }
      catch (Exception e)
      {
        if (e is ThreadAbortException)
        {
          Thread.Sleep(1000);
        }
        else
        {
          Console.WriteLine(e.Message);
        }
        return;
      }

      // Sleep in case we need to abort
      Thread.Sleep(500);
        
      // Code to perform specified command
      try
      {      
      BOOTUBLSEQ1:
        // Clear input buffer so we can start looking for BOOTUBL
        MySP.DiscardInBuffer();

		MySP.Write("  START\0");
		
        Console.WriteLine("\nWaiting for SFT on the "+devString+"...");
        
        // Wait for the SFT on the device to send the ^BOOTUBL\0 sequence
        if (!SerialIO.waitForSequence("BOOTUBL\0", "BOOTUBL\0", MySP, cmdParams.verbose))
          goto BOOTUBLSEQ1;
        
        //Console.WriteLine("BOOTUBL commmand received. Returning CMD and command...");

        // 8 bytes acknowledge sequence = "    CMD\0"
        MySP.Write("    CMD\0");
        // 8 bytes of magic number
        MySP.Write(((UInt32)cmdParams.CMDMagicFlag).ToString("X8"));
        
        //Console.WriteLine("CMD value sent.  Waiting for DONE...");
        
        if (!SerialIO.waitForSequence("   DONE\0", "BOOTUBL\0", MySP, cmdParams.verbose))
          goto BOOTUBLSEQ1;
        
        //Console.WriteLine("DONE received. Command was accepted.");

        // Take appropriate action depending on command
        switch (cmdParams.CMDMagicFlag)
        {
          case MagicFlags.UBL_MAGIC_FLASH:
          {
            status = TransmitUBLandAPP();
            break;
          }
          case MagicFlags.UBL_MAGIC_FLASH_NO_UBL:
          {
            status = TransmitAPP();
            break;
          }
          case MagicFlags.UBL_MAGIC_ERASE:
          {
            status = TransmitErase();
            break;
          }
	      case MagicFlags.UBL_MAGIC_FLASH_DSP:
          {
            status = TransmitTwoUBLsandAPP();
            break;
          }
		  default:
          {
            Console.WriteLine("Command not recognized!");
            status = false;
            break;
          }
        }
        if (!status)
        {
          Console.Write("\nCommand failed\n");
          return;
        }
      }
      catch (Exception e)
      {
        if (e is ThreadAbortException)
        {
          Thread.Sleep(1000);
        }
        else
        {
          Console.WriteLine(e.Message);
        }
        return;
      }
        
      // Wait for ^^^DONE that indicates SFT is exiting and so can this host program
      if (!SerialIO.waitForSequence("   DONE\0", "BOOTUBL\0", MySP, cmdParams.verbose))
      {
        throw new Exception("Final DONE not returned.  Operation failed.");
      }
      
      // Clean up any embedded files we extracted
      EmbeddedFileIO.CleanUpEmbeddedFiles();
        
      // Everything worked, so change boolean status
      workerThreadSucceeded = true;
    }

    /// <summary>
    /// Function to Transmit the UBL via the device ROM Serial boot
    /// Doesn't need it anymore
    /// </summary>
    private static void TransmitSFT(Byte[] imageData)
    { 
      // Send the UBL/SFT data
      //Console.WriteLine("Entering AIS_Parser");
      AIS_Parser parser = new AIS_Parser(AisHostType.UART, AisSecureType.NONE, UTIL_log, UTIL_uartRead, UTIL_uartWrite); 
      
      try
      {
      BOOTMESEQ:
        //Console.WriteLine("\nWaiting for the "+devString+"...");
        // Start the ROM booting 
        if (AisStatus.ERROR == parser.boot(imageData)) 
        {
          Console.WriteLine("Booting SFT failed. Trying again (you may need to reset the target)...");
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
    
    /// <summary>
    /// Function to transmit the application code via the SFT, which is now 
    /// running on the device.  This code is specific to the supplied SFT.
    /// If the the TI supplied SFT is modified or a different boot loader is
    /// used, this code will need to be modified.
    /// </summary>
    private static Boolean TransmitImage(Byte[] imageData, ImageHeader ackHeader)
    {
      ProgressBar progressBar;
      UInt32 blockCnt;
      
      try
      {
        // Console.WriteLine("Waiting for SENDIMG sequence...");
        if (!SerialIO.waitForSequence("SENDIMG\0", "BOOTUBL\0", MySP, cmdParams.verbose))
        {
          Console.Write("UART response didnt match\n");
          return false;
        }
        
        // Output 36 Bytes for the ACK sequence and header
        // 8 bytes acknowledge sequence = "    ACK\0"
        MySP.Write("    ACK\0");
        // 8 bytes of magic number
        MySP.Write(String.Format("{0:X8}", ackHeader.magicNum));
        // 8 bytes of binary execution address = ASCII string of 8 hex characters
        MySP.Write(String.Format("{0:X8}", ackHeader.startAddr));
        // 8 bytes of data size = ASCII string of 8 hex characters
        MySP.Write(String.Format("{0:X8}", ackHeader.byteCnt));
        // 8 bytes of binary load address = ASCII string of 8 hex characters
        MySP.Write(String.Format("{0:X8}", ackHeader.loadAddr));
        // 4 bytes of constant zeros = "0000"
        MySP.Write("0000");

        //Console.WriteLine("ACK command sent. Waiting for BEGIN command... ");

        // Wait for the ^^BEGIN\0 sequence
        if (!SerialIO.waitForSequence("  BEGIN\0", "BOOTUBL\0", MySP, cmdParams.verbose))
        {
          return false;
        }
        //Console.WriteLine("BEGIN commmand received.");

        // Send the image data
        progressBar = new ProgressBar();
        progressBar.Update(0.0,"Sending image over UART...");
        blockCnt = ackHeader.byteCnt/128;
        for (int i = 0; i < (blockCnt*128); i+=128)
        {
          MySP.Write(imageData, i, 128);
          progressBar.Percent = (((Double)(i+1))/ackHeader.byteCnt);
        }
        
        // Write last (possibly partial) block
        MySP.Write(imageData, (Int32) (blockCnt*128),(Int32) (ackHeader.byteCnt - (blockCnt*128)) );
        progressBar.Update(100.0,"Image data transmitted over UART.");
        
        //Console.WriteLine("Waiting for DONE...");

        // Wait for first ^^^DONE\0 to indicate data received
        if (!SerialIO.waitForSequence("   DONE\0", "BOOTUBL\0", MySP, cmdParams.verbose))
        {
          return false;
        } 
        //Console.WriteLine("DONE received.  All bytes of image data received...");
      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }
      return true;
    }

    /// <summary>
    /// Send command and wait for erase response. (SPI, NOR and NAND global erase)
    /// </summary>
    private static Boolean TransmitErase()
    {
      string a1;
      uint flashSize;
      try
      {
        // Receive size of flash to be erased. 
        // First get keyword SIZE followed by the number of bytes
	
        if (!SerialIO.waitForSequence("   SIZE\0", "   FAIL\0", MySP, cmdParams.verbose))
        {
          return false;
        }

        a1 = SerialIO.readSequence( MySP, cmdParams.verbose);

        //Remove trailing null character and convert to integer
        flashSize = Convert.ToUInt32(a1.Remove(8,1),16);
        
        Console.Write("\nErasing flash\n");
	   DisplayProgressBar(flashSize, 65536*2, "Erasing chip", "Verifying...", "Erase complete");
      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }

      return true;
    }
    
    /// <summary>
    /// Function to transmit the application for flash
    /// </summary>
    private static Boolean TransmitAPP()
    {         
      Byte[] imageData;
      ImageHeader ackHeader = new ImageHeader();

      try
      {          
        // Now Send the application image that will be written to flash
        //Console.WriteLine("Sending the Application image");
        imageData = FileIO.GetFileData(cmdParams.APPFileName);
        ackHeader.magicNum = ((UInt32)MagicFlags.UBL_MAGIC_BINARY_BOOT);
        ackHeader.startAddr = cmdParams.APPStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = cmdParams.APPLoadAddr;

        Console.WriteLine("\nFlashing application " + cmdParams.APPFileName +" (" + imageData.Length +" bytes)\n");        

        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming application into flash...", "Verifying...", "Application programming complete");

        return true;
      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }
    }
 
    /// <summary>
    /// Function to transmit the UBL and application for flash
    /// </summary>
    private static Boolean TransmitUBLandAPP()
    {         
      Byte[] imageData;
      ImageHeader ackHeader = new ImageHeader();

      try
      {
        // First send the UBL image that will be written to flash
        //Console.WriteLine("Sending the UBL image");
        // Now Send the application image that will be written to flash
        imageData = FileIO.GetFileData(cmdParams.UBLFileName);
        ackHeader.magicNum = ((UInt32)cmdParams.UBLMagicFlag);
        ackHeader.startAddr = cmdParams.UBLStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = 0x0020;

        Console.WriteLine("\nFlashing UBL " + cmdParams.UBLFileName +" (" + imageData.Length +" bytes) at 0x00000000\n");  

        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        // Update status bar 
        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming UBL into flash...", "Verifying...", "UBL programming complete");

        // Now Send the application image that will be written to flash
        imageData = FileIO.GetFileData(cmdParams.APPFileName);
        ackHeader.magicNum = (UInt32)cmdParams.APPMagicFlag;
        ackHeader.startAddr = cmdParams.APPStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = cmdParams.APPLoadAddr;    // Not used here, but this is what RBL assumes
        Console.WriteLine("\nFlashing application " + cmdParams.APPFileName +" (" + imageData.Length +" bytes) at 0x00010000\n");     
   
        // Transmit the data
        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        // Update status bar as SPI is being flashed
        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming application into flash...", "Verifying...", "Application programming complete");

        return true;
      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }
    }
    
    /// <summary>
    /// Function to transmit the DSP and ARM UBL and application for flash
    /// </summary>
    private static Boolean TransmitTwoUBLsandAPP()
    {         
      Byte[] imageData;
      ImageHeader ackHeader = new ImageHeader();

      try
      {
        // First send the DSP UBL image that will be written to flash
        //Console.WriteLine("Sending the UBL image");
        // Now Send the application image that will be written to flash
        imageData = FileIO.GetFileData(cmdParams.DSPUBLFileName);
        ackHeader.magicNum = ((UInt32)cmdParams.UBLMagicFlag);
        ackHeader.startAddr = cmdParams.UBLStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = 0x0020;

        Console.WriteLine("\nFlashing DSPUBL " + cmdParams.DSPUBLFileName +" (" + imageData.Length +" bytes) at 0x00000000\n");  

        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        // Update status bar 
        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming DSPUBL into flash...", "Verifying...", "DSP UBL programming complete");

        // Next  send the ARM UBL image that will be written to flash
        //Console.WriteLine("Sending the UBL image");
        // Now Send the application image that will be written to flash
        imageData = FileIO.GetFileData(cmdParams.ARMUBLFileName);
        ackHeader.magicNum = ((UInt32)cmdParams.UBLMagicFlag);
        ackHeader.startAddr = cmdParams.ARMUBLStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = cmdParams.ARMUBLStartAddr;

        Console.WriteLine("\nFlashing ARM UBL " + cmdParams.ARMUBLFileName +" (" + imageData.Length +" bytes) at 0x00002000\n");  

        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        // Update status bar 
        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming ARM UBL into flash...", "Verifying...", "ARM UBL programming complete");


        // Now Send the application image that will be written to flash
        imageData = FileIO.GetFileData(cmdParams.APPFileName);
        ackHeader.magicNum = (UInt32)cmdParams.APPMagicFlag;
        ackHeader.startAddr = cmdParams.APPStartAddr;
        ackHeader.byteCnt = (UInt32) imageData.Length;
        ackHeader.loadAddr = cmdParams.APPLoadAddr;    // Not used here, but this is what RBL assumes
        Console.WriteLine("\nFlashing application " + cmdParams.APPFileName +" (" + imageData.Length +" bytes) at 0x00008000\n");     
   
        // Transmit the data
        if (!TransmitImage(imageData, ackHeader))
        {
          return false;
        }

        // Update status bar as SPI is being flashed
        DisplayProgressBar(ackHeader.byteCnt, 4096, "Programming application into flash...", "Verifying...", "Application programming complete");

        return true;
      }
      catch (ObjectDisposedException e)
      {
        Console.WriteLine(e.StackTrace);
        throw e;
      }
    }
    /// <summary>
    /// Function to display progress bar when transmitting/erasing via SPI
    /// </summary>      
    private static void DisplayProgressBar(uint totalSize, int chunksize, string loadingText, string verifyingText, string finishedText)
    {  
      ProgressBar progressBar = new ProgressBar();
      int sending, sent;

      progressBar.Update(0.0,loadingText);
      sending = 1;
      sent = 0;
      while(sending==1)
      {
        if (!SerialIO.waitForSequence("   DONE\0", "SENDING\0", MySP, cmdParams.verbose))
        {
          if(totalSize < chunksize)
          {
            progressBar.Percent = 100;
          }
          else
          {
            progressBar.Percent =(Double)sent/(totalSize/chunksize);
          }
          if(progressBar.Percent >=1)
          {
            progressBar.Update(100.0,verifyingText);
          }
          sent++;
        }
        else
        {
          sending = 0;
          progressBar.Update(100,finishedText);
        }
      }
    }
    
    
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

