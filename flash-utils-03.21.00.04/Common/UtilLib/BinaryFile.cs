// This module parses a COFF file created by Texas Instruments TMS DSP
// code generation tools and offers a set of easy to use methods for obtaining
// information from the COFF file. This information includes header info.,
// symbol table, section data, etc.

using System;
using System.Text;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using System.Globalization;
using System.Collections;
using TI.UtilLib;
using TI.UtilLib.IO;

namespace TI.UtilLib.ObjectFile
{
  public class BinaryFile : ObjectFile
  {
    #region Public Class constructor(s)
    public BinaryFile(String filename) : base(filename)
    {     
      // Parse the Binary file
      try
      {
        ParseBinaryFile(0x00000000);
      }
      catch (Exception e)
      {
        Console.Write(e.Message);
        throw e;
      }
      
      fileType = ObjectFileType.BINARY;
    }

    public BinaryFile(String filename, UInt64 address) : base(filename)
    {     
      // Parse the Binary file
      try
      {
        ParseBinaryFile(address);
      }
      catch (Exception e)
      {
        Console.Write(e.Message);
        throw e;
      }
      
      fileType = ObjectFileType.BINARY;
    }       
    #endregion

    #region Public Class Methods
    public new String ToString()
    {
      StringBuilder strBuilder = new StringBuilder(512);
      
      strBuilder.Append(base.ToString());

      return strBuilder.ToString();
    }
    
    #endregion

    #region Private parsing functions
    private void ParseBinaryFile(UInt64 address)
    {
      // Output console message
      Console.WriteLine("Parsing the input file, {0}.", fileName);
      
      // Set endian to little
      endian = Endian.LittleEndian;
      
      // Set section count
      sectionCount = 1;
      loadableSectionCount = 1;
      symbolCount  = 0;
      
      binFile.Seek(0, SeekOrigin.Begin);
      EndianBinaryReader ebr = new EndianBinaryReader(binFile, endian);
     
      sections      = new ObjectSection[sectionCount];
      sections[0]   = new ObjectSection();

      sections[0].name         = fileName;
      sections[0].loadAddr     = address;
      sections[0].runAddr      = address;
      sections[0].size         = (UInt64) binFile.Length;
      sections[0].size         = ((sections[0].size + 3) >> 2) << 2;
      sections[0].binFileAddr  = 0x00000000;
      sections[0].isLoadable   = true;
      
      loadableSections    = new ObjectSection[loadableSectionCount];
      loadableSections[0] = sections[0];
      
    } // end ParseBinaryFile()
    #endregion
      
    #region Public Static Class Methods
    public static Boolean IsBinaryFile(String filename)
    {        
      // Get File Details
      FileInfo fi = new FileInfo(filename);
      if (fi.Exists)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
    #endregion
  } //End BinaryFile class

} //end of namespace
