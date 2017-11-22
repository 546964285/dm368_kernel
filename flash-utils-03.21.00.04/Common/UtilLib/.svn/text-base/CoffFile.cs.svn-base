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
  public enum COFF_Version : ushort
  {
    COFF1 = 0x00C1,
    COFF2 = 0x00C2
  }
  
  public enum COFF_Machine: ushort
  {
    CM_TI_ARM           = 0x0097,  // Texas Instruments TMS470 family
    CM_TI_C5400         = 0x0098,  // Texas Instruments TMS320C5400 family
    CM_TI_C6000         = 0x0099,  // Texas Instruments TMS320C6000 family
    CM_TI_C5500         = 0x009C,  // Texas Instruments TMS320C5500 family
    CM_TI_C2800         = 0x009D,  // Texas Instruments TMS320C2800 family
    CM_TI_MSP430        = 0x00A0,  // Texas Instruments MSP430
    CM_TI_C5500p        = 0x00A1,  // Texas Instruments TMS320C5500+ DSP
  }
  
  [Flags]
  public enum COFF_HeaderFlags: ushort
  {
    CF_RELFLG           = 0x0001,
    CF_EXEC             = 0x0002,
    CF_LNNO             = 0x0004,
    CF_LSYMS            = 0x0008,
    CF_LITTLE           = 0x0100,
    CF_BIG              = 0x0200,
    CF_SYMMERGE         = 0x1000
  }
    
  [Flags]
  public enum COFF_SectionType : uint
  {
    REG     = 0x00000000,
    DUMMY   = 0x00000001,
    NOLOAD  = 0x00000002,
    GROUP   = 0x00000004,
    PAD     = 0x00000008,
    COPY    = 0x00000010,
    TEXT    = 0x00000020,
    DATA    = 0x00000040,
    BSS     = 0x00000080,
    BLOCK   = 0x00001000,
    PASS    = 0x00002000,
    CLINK   = 0x00004000,
    VECTOR  = 0x00008000,
    PADDED  = 0x00010000
  }
  
  public struct COFF_Header
  {
    public COFF_Version     c_version;
    public UInt16           c_shnum;
    public UInt32           c_datestamp;
    public UInt32           c_psymtable;
    public UInt32           c_symnum;
    public UInt16           c_ehsize;
    public COFF_HeaderFlags c_flags;
    public COFF_Machine     c_machine;
    public Endian           c_endian;
  }

  /// <summary>
  /// Public class to read and parse a COFF object file
  /// </summary>
  public class CoffFile : ObjectFile
  {
    #region Private constants
    private const UInt32 COFFHeaderSize = 22;
    private const UInt32 symbolTableEntrySize = 18;
    #endregion
  
    #region Private internal members
    private COFF_Header hdr;
    private Hashtable headerRef;
    private Hashtable[] sectionRef;
    private Hashtable[] symRef;
    #endregion

    #region Public properties and indexers      
    public Hashtable Header
    {
      get { return headerRef; }
    }      
    #endregion

    #region Public Class constructor(s)
    public CoffFile(String filename) : base(filename)
    {
      // Init header hashtable
      headerRef = new Hashtable();
      
      // Parse the COFF file
      try
      {
        ParseCoffFile();
      }
      catch (Exception e)
      {
        Console.Write(e.Message);
        throw e;
      }
      
      fileType = ObjectFileType.COFF;
    }   
    #endregion

    #region Public Class Methods
    public void dumpFileHeader()
    {
      foreach (DictionaryEntry de in headerRef)
      {
        Console.WriteLine("header[{0}] = {1}", de.Key, de.Value);
      }
    }

    public new String ToString()
    {
      StringBuilder strBuilder = new StringBuilder(512);
      
      dumpFileHeader();
      
      strBuilder.Append(base.ToString());

      return strBuilder.ToString();
    }
    
    #endregion

    #region Private Methods
    private void ParseCoffFile()
    {
      // Output console message
      Console.WriteLine("Parsing the input object file, {0}.", fileName);
      
      // Parse the COFF header
      try
      {
        hdr = CoffFile.ParseCOFFHeader(binFile);
      }
      catch (Exception e)
      {
        Console.Write(e.Message);
        return;
      }
      
      // Set section count
      sectionCount = hdr.c_shnum;
      symbolCount  = hdr.c_symnum;
      
      // Set endianness
      endian = hdr.c_endian;
      Debug.DebugMSG("Endianness: " + endian.ToString());
      
      binFile.Seek(0, SeekOrigin.Begin);
      EndianBinaryReader ebr = new EndianBinaryReader(binFile, hdr.c_endian);
      
      headerRef["numBootSections"] = (UInt32)0;

      // Read the optional COFF header
      if (hdr.c_ehsize != 0)
      {
        ebr.BaseStream.Seek(COFFHeaderSize,SeekOrigin.Begin);
        headerRef["optMagicNumber"] = (UInt32) ebr.ReadUInt16();
        Debug.DebugMSG("optMagicNumber: " + ((UInt32)headerRef["optMagicNumber"]).ToString("X4"));

        headerRef["optVersionStamp"] = (UInt32)ebr.ReadUInt16();
        Debug.DebugMSG("optVersionStamp: " + ((UInt32)headerRef["optVersionStamp"]).ToString("X4"));

        headerRef["optExeSize"] = ebr.ReadUInt32();
        Debug.DebugMSG("optExeSize: " + ((UInt32)headerRef["optExeSize"]).ToString("X8"));

        headerRef["optInitSize"] = ebr.ReadUInt32();
        Debug.DebugMSG("optInitSize: " + ((UInt32)headerRef["optInitSize"]).ToString("X8"));

        headerRef["optUninitSize"] = ebr.ReadUInt32();
        Debug.DebugMSG("optUninitSize: " + ((UInt32)headerRef["optUninitSize"]).ToString("X8"));

        entryPoint = (UInt64) ebr.ReadUInt32();
        headerRef["optEntryPoint"] = (UInt32) entryPoint;
        Debug.DebugMSG("optEntryPoint: " + ((UInt32)headerRef["optEntryPoint"]).ToString("X8"));
        
        headerRef["optExeAddr"] = ebr.ReadUInt32();
        Debug.DebugMSG("optExeAddr: " + ((UInt32)headerRef["optExeAddr"]).ToString("X8"));

        headerRef["optInitAddr"] = ebr.ReadUInt32();
        Debug.DebugMSG("optInitAddr: " + ((UInt32)headerRef["optInitAddr"]).ToString("X8"));
      }
     
      // Read the section headers
      ParseSectionHdrs();

      // Parse the symbol table
      ParseSymbolTable();
    } // end ParseCOFFFile()

    
    /// <summary>
    /// Parse the section headers.
    /// </summary>
    private void ParseSectionHdrs()
    {
      UInt32 numBytesInSectionHdr;
      EndianBinaryReader ebr = new EndianBinaryReader(binFile, endian);

      if (hdr.c_version == COFF_Version.COFF2)
      {
        numBytesInSectionHdr = 48;
      }
      else if (hdr.c_version == COFF_Version.COFF1)
      {
        numBytesInSectionHdr = 40;
      }
      else
      {
        numBytesInSectionHdr = 0;
      }
                    
      sectionRef  = new Hashtable[sectionCount];
      sections    = new ObjectSection[sectionCount];
      
      for (UInt16 secNum = 0; secNum < sectionCount; secNum++)
      {
        sectionRef[secNum] = new Hashtable();
        sections[secNum]   = new ObjectSection();

        ebr.BaseStream.Seek(numBytesInSectionHdr * secNum + COFFHeaderSize + hdr.c_ehsize, SeekOrigin.Begin);
        sections[secNum].name         = COFF_getName();

        ebr.BaseStream.Seek(numBytesInSectionHdr * secNum + COFFHeaderSize + hdr.c_ehsize + 8, SeekOrigin.Begin);
        sections[secNum].runAddr      = (UInt64) ebr.ReadUInt32();
        sections[secNum].loadAddr     = (UInt64) ebr.ReadUInt32();
        sections[secNum].size         = (UInt64) ebr.ReadUInt32();
        sections[secNum].size         = ((sections[secNum].size + 3) >> 2) << 2;
        sections[secNum].binFileAddr  = (UInt64) ebr.ReadUInt32();

        sectionRef[secNum]["reloPtr"] = ebr.ReadUInt32();
        sectionRef[secNum]["linePtr"] = ebr.ReadUInt32();

        if (hdr.c_version == COFF_Version.COFF2)
        {
          sectionRef[secNum].Add("numRelos", ebr.ReadUInt32());
          sectionRef[secNum].Add("numLines", ebr.ReadUInt32());
          sectionRef[secNum].Add("flags", ebr.ReadUInt32());
          sectionRef[secNum].Add("reserved", (UInt32) ebr.ReadUInt16());
          sectionRef[secNum].Add("memPage", (UInt32) ebr.ReadUInt16());
        }
        else
        {
          sectionRef[secNum].Add("numRelos", (UInt32) ebr.ReadUInt16());
          sectionRef[secNum].Add("numLines", (UInt32) ebr.ReadUInt16());
          sectionRef[secNum].Add("flags", (UInt32) ebr.ReadUInt16());
          sectionRef[secNum].Add("reserved", (UInt32) ebr.ReadByte());
          sectionRef[secNum].Add("memPage", (UInt32) ebr.ReadByte());
        }

        //Check to see if section is bootable
        UInt32 flags = (UInt32)sectionRef[secNum]["flags"];
        sectionRef[secNum]["bootable"] = false;
        if ((flags & ((UInt32)(COFF_SectionType.TEXT | COFF_SectionType.DATA))) != 0)
        {
          if ((flags & ((UInt32)COFF_SectionType.COPY)) == 0)
          {
            if (sections[secNum].size != 0)
            {
              headerRef["numBootSections"] = ((UInt32)headerRef["numBootSections"]) + 1;
              sectionRef[secNum]["bootable"] = true;
            }
          }
        }

        // Check to see if section is loadable
        sections[secNum].isLoadable = false;
        if ( ( sections[secNum].binFileAddr != 0 ) && ( sections[secNum].size != 0 ) )
        {
          if ((flags & ((UInt32)(COFF_SectionType.BSS |        // No BSS sections
                        COFF_SectionType.COPY |                // No COPY sections
                        COFF_SectionType.NOLOAD |              // No NOLOAD sections
                        COFF_SectionType.DUMMY))               // No DUMMY sections
               ) == 0)
          {
            sections[secNum].isLoadable = true;
            loadableSectionCount++;
          }
        }
        
        Debug.DebugMSG("ObjectSection sections[" + secNum + "] = \n{");
        Debug.DebugMSG("\tname = " + sections[secNum].name + ",");
        Debug.DebugMSG("\tsize = " + sections[secNum].size.ToString("X8") + ",");
        Debug.DebugMSG("\trunAddr = " + sections[secNum].runAddr.ToString("X8") + ",");
        Debug.DebugMSG("\tloadAddr = " + sections[secNum].loadAddr.ToString("X8") + ",");
        Debug.DebugMSG("\tisLoadable = " + sections[secNum].isLoadable + ",");
        Debug.DebugMSG("\tbinFileAddr = " + sections[secNum].binFileAddr.ToString("X8"));
        Debug.DebugMSG("}");
      }
      
      // Fill in the loadableSections array
      loadableSections = new ObjectSection[loadableSectionCount];
      for (UInt32 secNum = 0,loadableSecNum=0; secNum < sectionCount; secNum++)
      {
        if (sections[secNum].isLoadable)
        {
          loadableSections[loadableSecNum] = sections[secNum];
          loadableSecNum++;
        }
      }
      
      // Finally, sort the loadable sections array by load address
      Array.Sort<ObjectSection>(loadableSections);
      
      Debug.DebugMSG("Parse Section Headers Done");
    } // end of ParseSectionHdrs()

    
    /// <summary>
    /// Parse the COFF file's symbol table.
    /// </summary>
    private void ParseSymbolTable()
    {
      EndianBinaryReader ebr = new EndianBinaryReader(binFile, endian);

      symRef  = new Hashtable[symbolCount];
      symbols = new ObjectSymbol[symbolCount];

      // Read the symbol table
      for (UInt32 symNum = 0; symNum < symbolCount; symNum++)
      {
        symRef[symNum]  = new Hashtable();
        symbols[symNum] = new ObjectSymbol();
        
        ebr.BaseStream.Seek( ((UInt32)hdr.c_psymtable) + (symbolTableEntrySize * symNum),SeekOrigin.Begin);
        symbols[symNum].name = COFF_getName();
        
        ebr.BaseStream.Seek( ((UInt32)hdr.c_psymtable) + (symbolTableEntrySize * symNum) + 8,SeekOrigin.Begin);
        symbols[symNum].value = (UInt64) ebr.ReadUInt32();
        
        symRef[symNum]["secNum"] = (UInt32) ebr.ReadUInt16();
        symRef[symNum]["type"] = (UInt32) ebr.ReadUInt16();
        symRef[symNum]["class"] = (UInt32) ebr.ReadByte();
        symRef[symNum]["auxNum"] = (UInt32) ebr.ReadByte();
        
        Debug.DebugMSG("symbols[" + symNum + "] = \n{");
        Debug.DebugMSG("\tname = " + symbols[symNum].name + ",");
        Debug.DebugMSG("\tvalue = " + symbols[symNum].value.ToString("X8"));
        Debug.DebugMSG("}");
        
        /*
        Debug.DebugMSG("symRef[" + symNum + "][\"secNum\"] = " + ((UInt32)symRef[symNum]["secNum"]).ToString("X8"));
        Debug.DebugMSG("symRef[" + symNum + "][\"type\"]   = " + ((UInt32)symRef[symNum]["type"]).ToString("X4"));
        Debug.DebugMSG("symRef[" + symNum + "][\"class\"]  = " + ((UInt32)symRef[symNum]["class"]).ToString("X2"));
        Debug.DebugMSG("symRef[" + symNum + "][\"auxNum\"] = " + ((UInt32)symRef[symNum]["auxNum"]).ToString("X2"));
        */
      }
      
      // Finally, sort the symbols by value (address)
      Array.Sort<ObjectSymbol>(symbols);
      
      Debug.DebugMSG("Parse Symbol Table Done");
    } // end of ParseSymbolTable()

    
    /// <summary>
    /// Function to retrieve name string either from string table or from current stream pointer.
    /// </summary>
    /// <returns>String containing name.</returns>
    private String COFF_getName()
    {
      Int64 currPos = binFile.Position;
      UInt32 int0,int1;
      Byte currByte;
      ArrayList name = new ArrayList(128);
                  
      EndianBinaryReader ebr =  new EndianBinaryReader(binFile,endian);
      int0 = ebr.ReadUInt32();
      int1 = ebr.ReadUInt32();
      ebr.BaseStream.Seek(currPos,SeekOrigin.Begin);
      
      if (int0 == 0x00000000)
      {
        // The name is stored in the string table
        UInt32 stringTableAddr = ((UInt32)hdr.c_psymtable) + (symbolTableEntrySize * ((UInt32)hdr.c_symnum));
        ebr.BaseStream.Seek((int1 + stringTableAddr),SeekOrigin.Begin);
        
        // Read characters and build string until terminating null
        currByte = ebr.ReadByte();
        
        while (currByte != 0)
        {
          name.Add(currByte);
          currByte = ebr.ReadByte();
        }
      }
      else
      {
        // Read characters straight from string table and build string until terminating null
        currByte = ebr.ReadByte();
        while ( (currByte != 0) && (name.Count < 8) )
        {
          name.Add(currByte);
          currByte = ebr.ReadByte();
        }
      }
      ebr.BaseStream.Seek(currPos, SeekOrigin.Begin);
      if (name.Count > 128)
          Debug.DebugMSG(ASCIIEncoding.ASCII.GetString((Byte[])name.ToArray(typeof(Byte)), 0, name.Count));
      return ASCIIEncoding.ASCII.GetString((Byte[]) name.ToArray(typeof(Byte)), 0, name.Count);
    }
    #endregion
      
    #region Public Static Class Methods
    public static Boolean IsCoffFile(String filename)
    {        
      // Get File Details
      try
      {
        if (File.Exists(filename))
        {
          // Parse the input file header
          using (FileStream fs = new FileStream(filename, FileMode.Open, FileAccess.Read))
          {
            ParseCOFFHeader(fs);
          }
        }
      }
      catch
      {
        //Console.Write(e.Message);
        return false;
      }
      return true;
    }
    
    public static Endian GetCOFFEndianness(Stream fs)
    {
      Endian endian = null;
      Int64 tempPos = fs.Position;
      
      // Read COFF Header
      fs.Seek(20, SeekOrigin.Begin);
      
      BinaryReader br = new BinaryReader(fs);
      
      {  
        // Check for COFF magic number and endianness
        UInt16 val = br.ReadByte();
        if (val == 0x00)
        {
          // Check for big endian
          val = (UInt16)br.ReadByte();
          if ( Enum.IsDefined(typeof(COFF_Machine),val) )
          {
            endian = Endian.BigEndian;
            if ( (val != (UInt16) COFF_Machine.CM_TI_C6000) && 
                 (val != (UInt16) COFF_Machine.CM_TI_ARM)   &&
                 (val != (UInt16) COFF_Machine.CM_TI_MSP430)   )
            {
              throw new Exception("Big endian detected for invalid machine type: " + Enum.GetName(typeof(COFF_Machine),val));
            }
          }
        }
        else if (br.ReadByte() == 0x00)
        {
          if ( Enum.IsDefined(typeof(COFF_Machine),val) )
          {
            endian = Endian.LittleEndian;
          }
        }
        else
        {
          throw new Exception("Unknown COFF magic number.");
        }        
      }
      
      // Get stream back to where it was
      fs.Seek(tempPos, SeekOrigin.Begin);
      
      return endian;
    }
    
    public static COFF_Header ParseCOFFHeader(Stream fs)
    {
      COFF_Header hdr;
      
      // Go to start of COFF file stream
      fs.Seek(0, SeekOrigin.Begin);

      // Figure out COFF endianness
      hdr.c_endian = GetCOFFEndianness(fs);
      
      // Create binary endian reader
      EndianBinaryReader ebr = new EndianBinaryReader(fs, hdr.c_endian);
      
      { 
        // Read the main COFF header
        // Only COFF type 1 and type 2 supported
        hdr.c_version = (COFF_Version) ebr.ReadUInt16();;
        Debug.DebugMSG("versionID: " + hdr.c_version);
        
        hdr.c_shnum = ebr.ReadUInt16();
        Debug.DebugMSG("numSectionHdrs: " + hdr.c_shnum.ToString("X4"));
        
        hdr.c_datestamp = ebr.ReadUInt32();
        Debug.DebugMSG("datestamp: " + hdr.c_datestamp.ToString("X8"));
        
        hdr.c_psymtable = ebr.ReadUInt32();
        Debug.DebugMSG("symbolTableAddr: " + hdr.c_psymtable.ToString("X8"));
        
        hdr.c_symnum = ebr.ReadUInt32();
        Debug.DebugMSG("numEntriesInSymTable: " + hdr.c_symnum.ToString("X8"));

        hdr.c_ehsize = ebr.ReadUInt16();
        Debug.DebugMSG("hdr.c_ehsize: " + hdr.c_ehsize.ToString("X4"));
        
        hdr.c_flags = (COFF_HeaderFlags) ebr.ReadUInt16();
        Debug.DebugMSG("flags: " + hdr.c_flags);
        
        hdr.c_machine = (COFF_Machine) ebr.ReadUInt16();
        Debug.DebugMSG("machineType: " + hdr.c_machine);
        
        // Check endianness in flags, check for match
        /*
        if ((hdr.c_flags & COFF_HeaderFlags.CF_LITTLE) == COFF_HeaderFlags.CF_LITTLE)
        {
          if (!endian.isLittleEndian())
          {
            throw new Exception("Endianness mismatch in header flags!");
          }
        }
        else if ((hdr.c_flags & COFF_HeaderFlags.CF_BIG) == COFF_HeaderFlags.CF_BIG)
        {
          if (!endian.isBigEndian())
          {
            throw new Exception("Endianness mismatch in header flags!");
          }
        }
        else
        {
          throw new Exception("Endianness flag not set!");
        }*/
      }
      
      return hdr;
    }
    #endregion
    
  } //End CoffFile class

} //end of namespace
