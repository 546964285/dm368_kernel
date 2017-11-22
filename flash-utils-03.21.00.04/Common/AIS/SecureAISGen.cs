/****************************************************************
 *  TI Secure Extension to Abstract AISGen Class
 *  (C) 2008, Texas Instruments, Inc.                           
 ****************************************************************/

using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Security.Cryptography;
using System.IO;
using System.IO.Ports;
using System.Reflection;
using System.Threading;
using System.Globalization;
using System.Collections;
using System.Collections.Generic;
using TI.UtilLib;
using TI.UtilLib.IO;
using TI.UtilLib.Ini;
using TI.UtilLib.CRC;
using TI.UtilLib.ObjectFile;
using TI.UtilLib.Crypto;

namespace TI.AISLib
{
  public enum BootLoaderExitType : int
  {
    NONSECURE     = 0x0,
    SECUREWITHSK  = 0x1,
    SECURENOSK    = 0x2,
    NONE          = 0x3
  }
  
  public enum SHA_Algorithm : int
  {
    SHA1 = 0x0,
    SHA256 = 0x1,
    SHA384 = 0x2,
    SHA512 = 0x3
  }
  
  public enum SecureLoadMagic : uint
  {
    // Load module magic ID
    LOADMOD_MAGIC   = 0x70ADC0DE,
    // Key certificate magic ID
    CERT_MAGIC      = 0x70ADCE87,
    // Generic user key magic ID
    GENKEY_MAGIC    = 0xBE40C0DE
  }
  
  public enum SecureLegacyMagic : uint
  {
    // Signed only
    SIGNMOD_MAGIC   = 0x5194C0DE,
    // Signed and Encrypted with CEK
    ENCMOD_MAGIC    = 0x034CC0DE
  }
  
  public abstract partial class AISGen
  {
  #region Protected Variables
    // Private security type variable
    protected AisSecureType secureTypeValue;
    
    // Private boot loader exit type
    protected BootLoaderExitType bootLoaderExitTypeValue;
    
    // Secure key data
    protected Byte[] secureKeyData;
  #endregion
  
  #region Public Properties
    public BootLoaderExitType bootLoaderExitType
    {
      get { return bootLoaderExitTypeValue; }
      set 
      {
        bootLoaderExitTypeValue = value;
      }
    }
  #endregion
    
    // Encryption Key
    public Byte[] customerEncryptionKey;
    public Byte[] rootCustomerEncryptionKey;
    public Byte[] CEKInitialValue;
    public Byte[] rootCEKInitialValue;
    public Byte[] keyEncryptionKey;
    public Byte[] genericKeyHeaderData;

    // RSA Object
    public RSACryptoServiceProvider rsaObject;
    public RSACryptoServiceProvider rootRsaObject;
    
    // Encrypted Section List
    public String[] sectionsToEncrypt;
     
    // Variable for generic secure key header
    public Boolean genericJTAGForceOff;
    
    // Variable for current selected Hash algorihtm
    public SHA_Algorithm currHashAlgorithmValue;
    public HashAlgorithm currHashAlgorithm;
    
    
  #region Public Virtual Methods 
    public virtual retType InsertAISSecureObjectFile( String fileName, Boolean encrypt )
    {
      Debug.DebugMSG("Inserting Object File, fileName = " + fileName);
      // Since no load address is provided, we can assume ObjectFile is ElfFile or CoffFile
      if (File.Exists(fileName))
      {
        // Parse the object file
        ObjectFile file;
        if (ElfFile.IsElfFile(fileName))
        {
          file = new ElfFile(fileName);
        }
        else if (CoffFile.IsCoffFile(fileName))
        {
          file = new CoffFile(fileName);
        }
        else
        {
          Console.WriteLine("ERROR: Not a valid object file.");
          return retType.FAIL;
        }
          
        if (file != null)
        {
          // Load the object file contents
          AISSecureObjectFileLoad(this,file);
          file.Close();
        }
        else
        {
          Console.WriteLine("ERROR: Parsing the input file {0} failed!",fileName);
        }
      }
      else
      {
        Console.WriteLine("WARNING: File {0} does not exist. Ignoring insert command.",fileName);
      }
      
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISSecureObjectFile( String fileName, Boolean useEntryPoint, Boolean encrypt)
    {
      // Since no load address is provided, we can assume ObjectFile is ElfFile or CoffFile
      if (InsertAISSecureObjectFile( fileName, encrypt ) != retType.SUCCESS)
      {
        return retType.FAIL;
      }
      
      // Set AIS entry point
      if (useEntryPoint)
      {
        ObjectFile file = FindObjectFile( this, fileName );
      
        this.entryPoint = (UInt32) file.EntryPoint;  
      }
      
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISSecureObjectFile( String fileName, UInt32 loadAddr, Boolean encrypt )
    {
      Debug.DebugMSG("Inserting Object File, fileName = " + fileName);
      // Since a load address is provided, we can assume ObjectFile is BinaryFile
      if (File.Exists(fileName))
      {
        // Parse the object file
        ObjectFile file = new BinaryFile(fileName, loadAddr);
          
        if (file != null)
        {
          // Load the object file contents
          AISSecureObjectFileLoad(this, file);
          file.Close();
        }
        else
        {
          Console.WriteLine("ERROR: Parsing the input file {0} failed!",fileName);
        }
      }
      else
      {
        Console.WriteLine("WARNING: File {0} does not exist. Ignoring insert command.",fileName);
      }
      
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISSecureObjectFile( String fileName, UInt32 loadAddr, UInt32 entryPoint, Boolean encrypt )
    {
      // Since a load address is provided, we can assume ObjectFile is BinaryFile
      if (InsertAISSecureObjectFile( fileName, loadAddr, encrypt) != retType.SUCCESS)
      {
        return retType.FAIL;
      }
      
      if (entryPoint != 0x00000000)
      {
        this.entryPoint = entryPoint;
      }
      
      return retType.SUCCESS;
    }
  
    public virtual retType InsertAISSecureKeyLoad(Byte[] secureKeyData)
    {
      writer.Write((UInt32)AisOps.SecureKeyLoad);
      writer.Write(secureKeyData);
      
      if ( SecureType != AisSecureType.NONE )
      {
        sigWriter.Write((UInt32)AisOps.SecureKeyLoad);
        sigWriter.Write(secureKeyData);
      }
    
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISEncSectionLoad(UInt32 address, UInt32 size, Byte[] unencData, Byte[] encData)
    {
      Debug.DebugMSG("Inserting AIS Encrypted Section Load command.");
      writer.Write((UInt32)AisOps.EncSection_Load);
      writer.Write(address);
      writer.Write(size);
      writer.Write(encData);
      
      if ( this.SecureType != AisSecureType.NONE )
      {
        sigWriter.Write((UInt32)AisOps.EncSection_Load);
        sigWriter.Write(address);
        sigWriter.Write(size);
        sigWriter.Write(unencData);
      }
      
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISSetExitMode(BootLoaderExitType exitMode)
    {
      Debug.DebugMSG("Inserting AIS Set Exit Mode command.");
      writer.Write((UInt32) AisOps.SetSecExitMode);
      writer.Write((UInt32) exitMode);

      if ( this.SecureType != AisSecureType.NONE )
      {
        sigWriter.Write((UInt32) AisOps.SetSecExitMode);
        sigWriter.Write((UInt32) exitMode);
      }
      
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISSetDelegateKey(String rsaKeyFileName, String keyString)
    {
      RSACryptoServiceProvider localRsaObject;
      Byte[] localEncryptionKey = null;
      Byte[] localEKInitialValue = null;
      
      if ( this.SecureType != AisSecureType.CUSTOM)
      {
        Console.WriteLine("WARNING: Delegate Key commands only apply to Custom Secure devices. AIS section ignored.");
        return retType.SUCCESS;
      }
      
      Debug.DebugMSG("Inserting AIS Set Delegate Key command.");
      
      localRsaObject = RSAKey.LoadFromFile(rsaKeyFileName);
      
      if (localRsaObject == null)
      {
        Console.WriteLine("SetDelegateKey: RSA key loading failed!");
        return retType.FAIL;
      }
            
      // Update the hash algo string if RSA key size is 2048 bits
      if (localRsaObject.KeySize != this.rsaObject.KeySize)
      {
        Console.WriteLine("ERROR: SetDelegateKey: Delegate key size cannot differ from current key size.");
        return retType.FAIL;
      }
      
      // Parse Encryption Key Value
      localEncryptionKey = new Byte[16];
      localEKInitialValue = new Byte[16];
      if (keyString.Length != 32)
      {
        Console.WriteLine("SetDelegateKey: AES Encryption Key is wrong length!");
        return retType.FAIL;
      }
      for (int j=0; j<keyString.Length; j+=2)
      {
        localEncryptionKey[(j>>1)] = Convert.ToByte(keyString.Substring(j,2),16);
      }
            
      // Generate IV as encrypted version of AES Key
      using (MemoryStream ms = new MemoryStream(localEKInitialValue))
      {
        Aes myAES = new AesManaged();
        myAES.KeySize = 128;
        myAES.Mode = CipherMode.ECB;
        myAES.Padding = PaddingMode.None;
        ICryptoTransform encryptor = myAES.CreateEncryptor(localEncryptionKey, new Byte[16]);
        CryptoStream cs = new CryptoStream(ms,encryptor,CryptoStreamMode.Write);
        cs.Write(localEncryptionKey,0,localEncryptionKey.Length);
      }
      
      // Create Delegate Key Certificate Format
      //Key cert: 304 bytes
      //  decryptionKey: 16 byte (128 bits) AES key

      //  rsaPublicKey: 8 bytes + 128/256 for modulus data (actual modulus data may only be 128 bytes for RSA1024)
      //    keyExponent: 4 bytes
      //    keyPad: 2 bytes
      //    modLength: 2 bytes
      //    modData: 128 or 256 bytes
      //  keyFlags: 8 bytes
      //  infoData: 16 bytes
      //    loadModMagic: 4 bytes
      //    loadModSize: 4 bytes
      //    randomSeed: 8 bytes
      
      Byte[] delegateKeyData = new Byte[48 + (localRsaObject.KeySize >> 3)];
      
      // Fill with random data
      (new Random()).NextBytes(delegateKeyData);
      
      // Insert load module header data
      BitConverter.GetBytes((UInt32)SecureLoadMagic.CERT_MAGIC).CopyTo(delegateKeyData, 0 );
      BitConverter.GetBytes((UInt32)delegateKeyData.Length).CopyTo(delegateKeyData, 4 );
      
      // Insert decryptionKey at offset 16
      localEncryptionKey.CopyTo(delegateKeyData, 16);
      // Insert rsaPublicKey at offset 32
      RSAKey.CreateCustomSecureKeyVerifyStruct(localRsaObject).CopyTo(delegateKeyData, 32);
      // Insert flags data
      BitConverter.GetBytes((UInt32)0x00000000).CopyTo(delegateKeyData, 40 + (localRsaObject.KeySize >> 3) );
      BitConverter.GetBytes((UInt32)0x00000000).CopyTo(delegateKeyData, 44 + (localRsaObject.KeySize >> 3) );

      #if (DEBUG)
      FileIO.SetFileData("delegatdata.bin",delegateKeyData, true);
      #endif
      
      Byte[] delegateKeyDataSigOrder = new Byte[48 + (localRsaObject.KeySize >> 3)];
      Array.Copy(delegateKeyData, 0 , delegateKeyDataSigOrder, 32 + (localRsaObject.KeySize >> 3), 16);
      Array.Copy(delegateKeyData, 16, delegateKeyDataSigOrder,  0, 32 + (localRsaObject.KeySize >> 3));      
      
      #if (DEBUG)
      FileIO.SetFileData("delegatdata_sigorder.bin",delegateKeyDataSigOrder, true);
      #endif
      
      // Insert AIS command and key data into signature data stream
      if ( this.SecureType == AisSecureType.CUSTOM)
      {
        sigWriter.Write((UInt32) AisOps.SetDelegateKey);
        sigWriter.Write(delegateKeyDataSigOrder);
      }
      
      // Encrypt the data (load header first, then payload)
      Byte[] encDelegateKeyData = AesManagedUtil.AesCBCEncrypt(delegateKeyData, this.customerEncryptionKey, this.CEKInitialValue);
      Array.Copy(encDelegateKeyData, 0 , delegateKeyDataSigOrder, 32 + (localRsaObject.KeySize >> 3), 16);
      Array.Copy(encDelegateKeyData, 16, delegateKeyDataSigOrder,  0, 32 + (localRsaObject.KeySize >> 3));
      
      #if (DEBUG)
      FileIO.SetFileData("encDelegateKeyData.bin",encDelegateKeyData, true);
      FileIO.SetFileData("encDelegateKeyData_sigorder.bin",delegateKeyDataSigOrder, true);
      #endif
      
      // Insert encrypted data into output stream
      writer.Write((UInt32) AisOps.SetDelegateKey);
      writer.Write(delegateKeyDataSigOrder);
      
      // Generate and Insert Signature
      InsertAISSecureSignature(this);
    
      // Make the new keys the installed ones for future AIS generation
      this.rsaObject                  = localRsaObject;
      this.customerEncryptionKey      = localEncryptionKey;
      this.CEKInitialValue            = localEKInitialValue;
    
      return retType.SUCCESS;
    }
    
    public virtual retType InsertAISRemDelegateKey()
    {
      if ( this.SecureType != AisSecureType.CUSTOM)
      {
        Console.WriteLine("WARNING: Delegate Key commands only apply to Custom Secure devices. AIS section ignored.");
        return retType.SUCCESS;
      }
      
      Debug.DebugMSG("Inserting AIS Remove Delegate Key command.");
      writer.Write((UInt32) AisOps.RemDelegateKey);

      if ( this.SecureType != AisSecureType.NONE )
      {
        sigWriter.Write((UInt32) AisOps.RemDelegateKey);
      }
      
      // Restore root keys to use for next parts of image
      this.rsaObject              = this.rootRsaObject;
      this.customerEncryptionKey  = this.rootCustomerEncryptionKey;
      this.CEKInitialValue        = this.rootCEKInitialValue;      
      
      return retType.SUCCESS;
    }
  #endregion
     
  #region Public Static Methods

    /// <summary>
    /// SecureGenAIS command.
    /// </summary>
    /// <param name="inputFileNames">File name of input .out file</param>
    /// <param name="bootMode">AISGen Object for the particular device</param>
    /// <returns>Bytes of the binary or AIS boot image</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      IniFile iniFile)
    {
      UInt32 numWords;
      
      // Set defaults
      devAISGen.bootLoaderExitType      = BootLoaderExitType.NONE;
      devAISGen.currHashAlgorithmValue  = SHA_Algorithm.SHA1;
      devAISGen.sectionsToEncrypt       = null;
      devAISGen.rsaObject               = null;
      devAISGen.customerEncryptionKey   = null;
      devAISGen.keyEncryptionKey        = null;
      devAISGen.genericKeyHeaderData    = null;
      devAISGen.currHashAlgorithm       = null;      
      
      // Setup the binary writer to generate the temp AIS file
      devAISGen.sigStream       = new MemoryStream();
      devAISGen.devAISStream    = new MemoryStream();
      devAISGen.sigWriter       = new EndianBinaryWriter( devAISGen.sigStream, devAISGen.devEndian);
      using ( devAISGen.writer  = new EndianBinaryWriter( devAISGen.devAISStream, devAISGen.devEndian) )
      {
        // List to keep track of loadable sections and their occupied memory ranges
        devAISGen.sectionMemory = new List<MemoryRange>();  
        // Initiate list to keep track of the input files
        devAISGen.objectFiles   = new List<ObjectFile>();
        
        // Get data from the GENERAL INI Section
        GeneralIniSectionParse(devAISGen, iniFile);
        
        // Get data from the SECURITY INI Section
        if (SecurityIniSectionParse(devAISGen, iniFile) != retType.SUCCESS)
        {
          Console.WriteLine("Aborting...");
          return null;
        }
      
      #region Handle the case of Legacy boot mode
        if (devAISGen.bootMode == AisBootModes.LEGACY)
        {
          UInt32 fileSize = 0, secureDataSize = 0, totalImgSize = 0, paddingSize = 0;
          UInt32 loadAddr = 0, fileCount = 0;
          Byte[] fileData;
          
          String fileName = null;
          Boolean encryptSections = false;
          UInt32 entryPointAddr = 0x00000000;
          
          // Check for legacy input file
          foreach( IniSection sec in iniFile.Sections)
          {
            if (sec.sectionName.Equals("LEGACYINPUTFILE", StringComparison.OrdinalIgnoreCase))
            {
              fileCount++;
              foreach (DictionaryEntry de in sec.sectionValues)
              {
                // File name for binary section data
                if (((String)de.Key).Equals("FILENAME", StringComparison.OrdinalIgnoreCase))
                {
                   fileName = (String) sec.sectionValues["FILENAME"];
                }
                
                // Binary section's load address in the memory map
                if (((String)de.Key).Equals("LOADADDRESS", StringComparison.OrdinalIgnoreCase))
                {
                  loadAddr = (UInt32) sec.sectionValues["LOADADDRESS"];
                }
                
                // Binary section's entry point address in the memory map
                if (((String)de.Key).Equals("ENTRYPOINTADDRESS", StringComparison.OrdinalIgnoreCase))
                {
                  entryPointAddr = (UInt32) sec.sectionValues["ENTRYPOINTADDRESS"];
                }
                
                // Binary section's entry point address in the memory map
                if (((String)de.Key).Equals("ENCRYPT", StringComparison.OrdinalIgnoreCase))
                {
                  if (((String)sec.sectionValues["ENCRYPT"]).Equals("YES", StringComparison.OrdinalIgnoreCase))
                    encryptSections = true;
                  if (((String)sec.sectionValues["ENCRYPT"]).Equals("TRUE", StringComparison.OrdinalIgnoreCase))
                    encryptSections = true;
                }
                
              }
              
              if (fileName == null)
              {
                Console.WriteLine("ERROR: File name must be provided in INPUTFILE section.");
                return null;
              }
        
              // Insert the file into the AIS image
              if (( entryPointAddr == 0x00000000))
              {
                Console.WriteLine("Entrypoint Address = {0} is not valid.", entryPointAddr);
                return null;
              }
            }
          }
          
          // Validate an input binary file was given in the INI file
          if (fileCount == 0)
          {
            Console.WriteLine("ERROR: You did not supply a binary file section in the INI file!");
            return null;
          }
          if (fileCount > 1)
          {
            Console.WriteLine("WARNING: You supplied too many binary file sections in the INI file.");
            Console.WriteLine("         Only using the first one.");
          }
          
          // Figure out the size of the secure data region (signature + keystruct)
          if (devAISGen.SecureType == AisSecureType.CUSTOM)
          {
            // On custom secure we have rsa keySize bits for signature and modulus of key struct, 
            // plus eight additional bytes from key struct
            secureDataSize = (UInt32) (8 + (devAISGen.rsaObject.KeySize >> 3) + (devAISGen.rsaObject.KeySize >> 3));
          }
          else if (devAISGen.SecureType == AisSecureType.GENERIC)
          {
            // On generic secure we have 32 for key and HashSize bits rounded up to nearst 16 bytes
            // for signature data.
            secureDataSize = (UInt32) (32 + ((((devAISGen.currHashAlgorithm.HashSize >> 3)+15)>>4)<<4));
          }

          // Verify legacy input binary file exists, and get data if it does
          if (File.Exists(fileName))
          {
            Byte[] tempFileData = FileIO.GetFileData(fileName);
            
            fileSize = (UInt32) tempFileData.Length;
            
            totalImgSize = 16 + fileSize + secureDataSize;
                  
            if (totalImgSize > 16*1024)
            {
              Console.WriteLine("WARNING: The input image is too large for the ROM boot loader.");
              Console.WriteLine("Reduce its size by {0} bytes.", (totalImgSize - (16*1024)));
            }
            
            // Figure out how much to pad input file region to acheive complete image ending on 1K boundary
            paddingSize = (((totalImgSize+1023) >> 10) << 10) - totalImgSize;

            // Copy to final data array of fileSize
            fileSize = fileSize + paddingSize;
            fileData = new Byte[fileSize];
            tempFileData.CopyTo(fileData,0);
            
            // Adjust total image size to final amount
            totalImgSize = 16 + fileSize + secureDataSize;
          }
          else
          {
            Console.WriteLine("Error: Binary file was not found!");
            return null;
          }
          
          if ( ((entryPointAddr & 0x00FFFFFF) < loadAddr) ||
               ((entryPointAddr & 0x00FFFFFF) > (loadAddr+fileSize)) )
          {
            Console.WriteLine("ERROR: Entry point falls outside load image region.");
            return null;
          }
        
          // Place header 
          // Config Word - indicates total image size, nor width (if applicable)
          if (devAISGen.busWidth == 16)
          {
            devAISGen.writer.Write   ((UInt32)(((((totalImgSize >> 10)-1) & 0xFFFF) << 8)|(0x1 << 0)|(0x0 << 4)));
            devAISGen.sigWriter.Write((UInt32)(((((totalImgSize >> 10)-1) & 0xFFFF) << 8)|(0x1 << 0)|(0x0 << 4)));
          }
          else
          {
            devAISGen.writer.Write   ((UInt32)(((((totalImgSize >> 10)-1) & 0xFFFF) << 8)|(0x0 << 0)|(0x0 << 4)));
            devAISGen.sigWriter.Write((UInt32)(((((totalImgSize >> 10)-1) & 0xFFFF) << 8)|(0x0 << 0)|(0x0 << 4)));
          }
          
          // Magic Number   - indicates signed or encrypted
          if (encryptSections)
          {
            devAISGen.writer.Write((UInt32)SecureLegacyMagic.ENCMOD_MAGIC);
            devAISGen.sigWriter.Write((UInt32)SecureLegacyMagic.ENCMOD_MAGIC);
          }
          else
          {
            devAISGen.writer.Write((UInt32)SecureLegacyMagic.SIGNMOD_MAGIC);
            devAISGen.sigWriter.Write((UInt32)SecureLegacyMagic.SIGNMOD_MAGIC);
          }
                    
          // Entry Point - where to jump within the image upon load     
          devAISGen.writer.Write( entryPointAddr );
          devAISGen.sigWriter.Write( entryPointAddr );
                    
          // SecureDataSize - size of data following image for key struct & signature
          devAISGen.writer.Write( (UInt32)secureDataSize );
          devAISGen.sigWriter.Write( (UInt32)secureDataSize );
          
          // Now place padded binary contents      
          if (!encryptSections)
          {
            // Non-encrypted section
            devAISGen.writer.Write(fileData);
            devAISGen.sigWriter.Write(fileData);
          }
          else
          {
            // Encrypted section
            // Write unencrypted data to the signature buffer
            devAISGen.sigWriter.Write(fileData);
                      
            // Encrypt data using CBC algorithm
            try
            {
              Byte[] encData = AesManagedUtil.AesCTSEncrypt(fileData,devAISGen.customerEncryptionKey,devAISGen.CEKInitialValue);
              
              // Write encrypted section data out to AIS data stream
              devAISGen.writer.Write(encData);
            }
            catch(Exception e)
            {
              Console.WriteLine("Exception during encryption operation: {0}",e.Message);
            }
          }
                    
          // Now place the key data
          devAISGen.writer.Write(devAISGen.secureKeyData);
          devAISGen.sigWriter.Write(devAISGen.secureKeyData);
                    
          // Finally place the signature which covers the entire image
          InsertAISSecureSignature( devAISGen );
          
          // Flush the data and then return to start
          devAISGen.devAISStream.Flush();
          devAISGen.devAISStream.Seek(0,SeekOrigin.Begin);
          
          devAISGen.sigStream.Close();
        }   
      #endregion

      #region AIS Generation
        else
        {
          // ---------------------------------------------------------
          // ****************** BEGIN AIS GENERATION *****************
          // ---------------------------------------------------------
          Console.WriteLine("Begining the Secure AIS file generation.");
          
          // Diaplay currently selected boot mode
          Console.WriteLine("AIS file being generated for bootmode: {0}.",Enum.GetName(typeof(AisBootModes),devAISGen.bootMode));
          
          // Write the premilinary header and fields (everything before first AIS command)
          devAISGen.InsertAISPreamble();
          Debug.DebugMSG("Preamble Complete");
          
          // Write the Secure Key command and key data
          devAISGen.InsertAISSecureKeyLoad(devAISGen.secureKeyData);
          Debug.DebugMSG("Secure Key Insertion Complete");
          
          // Insert Exit Mode type
          devAISGen.InsertAISSetExitMode(devAISGen.bootLoaderExitType);
          Debug.DebugMSG("Set Exit Mode complete");
          
          // Parse the INI sections in order, inserting needed AIS commands
          foreach(IniSection sec in iniFile.Sections)
          {
            InsertSecureAISCommandViaINI(devAISGen, sec);
          }
          Debug.DebugMSG("INI parsing complete");
          
          // Insert the object file(s) passed in on the top-level (if it exists)
          foreach (String fn in inputFileNames)
          {
            String[] nameAndAddr = fn.Split('@');
            
            Debug.DebugMSG("Inserting file " + nameAndAddr[0]);
            
            if (!File.Exists(nameAndAddr[0]))
            {
              Console.WriteLine("ERROR: {0} does not exist. Aborting...", nameAndAddr[0]);
              return null;
            }
            
            // Handle binary files provided with a load address
            if (nameAndAddr.Length == 2)
            {
              UInt32 loadAddr;
              
              nameAndAddr[1] = nameAndAddr[1].ToLower();
              
              if (nameAndAddr[1].StartsWith("0x"))
              {
                if (!UInt32.TryParse(nameAndAddr[1].Replace("0x", ""), NumberStyles.HexNumber, null, out loadAddr))
                {
                  Console.WriteLine("WARNING: Invalid address format, {0}. Ignoring...", nameAndAddr[1]);
                }
                else
                {
                  devAISGen.InsertAISSecureObjectFile(nameAndAddr[0], loadAddr, false);
                }
              }
              else if (UInt32.TryParse(nameAndAddr[1], out loadAddr))
              {
                devAISGen.InsertAISSecureObjectFile(nameAndAddr[0], loadAddr, false);
              }
              else
              {
                Console.WriteLine("WARNING: Invalid address format, {0}. Ignoring...", nameAndAddr[1]);
              }
            }
            
            // Handle Object files (ones provided without load address)
            else if (nameAndAddr.Length == 1)
            {
              // If we still have not had a valid entry point set, then use entry point from 
              // first encountered non-binary file in the inputFileNames list
              if (devAISGen.entryPoint == 0xFFFFFFFF)
              {
                devAISGen.InsertAISSecureObjectFile(nameAndAddr[0], true, false);
              }
              else
              {
                devAISGen.InsertAISSecureObjectFile(nameAndAddr[0], false, false);
              }
            }
            else
            {
              Console.WriteLine("WARNING: Invalid filename format, {0}. Ignoring...", fn);
            }
          }
          
          Debug.DebugMSG("Main input file insertion complete.");
          
          // Insert closing JumpClose AIS command (issue warning)
          if (devAISGen.entryPoint == 0x00000000)
          {
            Console.WriteLine("WARNING: Entry point set to null pointer!");
          }
          devAISGen.InsertAISJumpClose(devAISGen.entryPoint);
          
          // Insert final Signature
          InsertAISSecureSignature(devAISGen);            
          
          // Flush the data and then return to start
          devAISGen.devAISStream.Flush();
          devAISGen.devAISStream.Seek(0,SeekOrigin.Begin);
        
          Console.WriteLine("AIS file generation was successful.");
          // ---------------------------------------------------------
          // ******************* END AIS GENERATION ******************
          // ---------------------------------------------------------
        }
      #endregion

        
        // Now create return byte array based on AIS Stream data
        EndianBinaryReader tempAIS_br = new EndianBinaryReader(devAISGen.devAISStream,Endian.LittleEndian);

        numWords = ((UInt32)tempAIS_br.BaseStream.Length) >> 2;
        devAISGen.AISData = new Byte[numWords << 2];   //Each word converts to 4 binary bytes

        Debug.DebugMSG("Number of words in the stream is {0}", numWords);

        // Copy the data to the output Byte array
        for (UInt32 i = 0; i < numWords; i++)
        {
          BitConverter.GetBytes(tempAIS_br.ReadUInt32()).CopyTo(devAISGen.AISData, i * 4);
        }

        // Close the binary reader
        tempAIS_br.Close();
      }  
      
      // Dispose of all object files
      foreach (ObjectFile file in devAISGen.objectFiles)
      {
        try
        {
          file.Dispose();
        }
        catch (Exception e)
        {
          Console.WriteLine(e.Message);
        }
      }
      
      // Clean up any embedded file resources that may have been extracted
      EmbeddedFileIO.CleanUpEmbeddedFiles();

      // Return Byte Array
      return devAISGen.AISData;
    }
   
    /// <summary>
    /// SecureGenAIS command.
    /// </summary>
    /// <param name="inputFileNames">File name of input .out file</param>
    /// <param name="bootMode">AISGen Object for the particular device</param>
    /// <returns>Bytes of the binary or AIS boot image</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      String iniData)
    {
      return SecureGenAIS(devAISGen, inputFileNames, new IniFile(iniData));
    }    
   
    /// <summary>
    /// Secondary genAIS thats calls the first
    /// </summary>
    /// <param name="inputFileNames">File name of .out file</param>
    /// <param name="bootmode">String containing desired boot mode</param>
    /// <returns>an Array of bytes to write to create an AIS file</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      String bootmode,
                                      IniFile iniFile)
    {
      devAISGen.bootMode = (AisBootModes)Enum.Parse(typeof(AisBootModes), bootmode, true);
      Console.WriteLine("Chosen bootmode is {0}.", devAISGen.bootMode.ToString());
      return SecureGenAIS(devAISGen, inputFileNames, iniFile);
    }

    /// <summary>
    /// Secondary genAIS thats calls the first
    /// </summary>
    /// <param name="inputFileNames">File name of .out file</param>
    /// <param name="bootmode">String containing desired boot mode</param>
    /// <returns>an Array of bytes to write to create an AIS file</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      String bootmode,
                                      String iniData)
    {
      return SecureGenAIS(devAISGen, inputFileNames, bootmode, new IniFile(iniData));
    }
    
    /// <summary>
    /// Secondary genAIS thats calls the first
    /// </summary>
    /// <param name="inputFileNames">File name of .out file</param>
    /// <param name="bootmode">AISGen.AisBootModes Enum value containing desired boot mode</param>
    /// <returns>an Array of bytes to write to create an AIS file</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      AisBootModes bootmode,
                                      IniFile iniFile)
    {
      devAISGen.bootMode = bootmode;
      Console.WriteLine("Chosen bootmode is {0}.", devAISGen.bootMode.ToString());
      return SecureGenAIS(devAISGen, inputFileNames, iniFile);
    }

    /// <summary>
    /// Secondary genAIS thats calls the first
    /// </summary>
    /// <param name="inputFileNames">File name of .out file</param>
    /// <param name="bootmode">AISGen.AisBootModes Enum value containing desired boot mode</param>
    /// <returns>an Array of bytes to write to create an AIS file</returns>
    public static Byte[] SecureGenAIS(AISGen devAISGen,
                                      List<String> inputFileNames,
                                      AisBootModes bootmode,
                                      String iniData)
    {
      return SecureGenAIS(devAISGen, inputFileNames, bootmode, new IniFile(iniData));
    }
    
    public static retType InsertSecureAISCommandViaINI(AISGen devAISGen, IniSection sec)
    {
      #region Handle Input Binary and Object Files
      if (sec.sectionName.Equals("INPUTFILE", StringComparison.OrdinalIgnoreCase))
      {
        String fileName = null;
        Boolean useEntryPoint = false, encryptSections = false;
        UInt32 loadAddr = 0x00000000;
        UInt32 entryPointAddr = 0x00000000;
        
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          // File name for binary section data
          if (((String)de.Key).Equals("FILENAME", StringComparison.OrdinalIgnoreCase))
          {
             fileName = (String) sec.sectionValues["FILENAME"];
          }
          
          // Binary section's load address in the memory map
          if (((String)de.Key).Equals("LOADADDRESS", StringComparison.OrdinalIgnoreCase))
          {
            loadAddr = (UInt32) sec.sectionValues["LOADADDRESS"];
          }
          
          // Binary section's entry point address in the memory map
          if (((String)de.Key).Equals("ENTRYPOINTADDRESS", StringComparison.OrdinalIgnoreCase))
          {
            entryPointAddr = (UInt32) sec.sectionValues["ENTRYPOINTADDRESS"];
          }
          
          // Option to specify that this entry point should be used for AIS
          if (((String)de.Key).Equals("USEENTRYPOINT", StringComparison.OrdinalIgnoreCase))
          {
            if (((String)sec.sectionValues["USEENTRYPOINT"]).Equals("YES", StringComparison.OrdinalIgnoreCase))
              useEntryPoint = true;
            if (((String)sec.sectionValues["USEENTRYPOINT"]).Equals("TRUE", StringComparison.OrdinalIgnoreCase))
              useEntryPoint = true;
          }
          
          // Binary section's entry point address in the memory map
          if (((String)de.Key).Equals("ENCRYPT", StringComparison.OrdinalIgnoreCase))
          {
            if (((String)sec.sectionValues["ENCRYPT"]).Equals("YES", StringComparison.OrdinalIgnoreCase))
              encryptSections = true;
            if (((String)sec.sectionValues["ENCRYPT"]).Equals("TRUE", StringComparison.OrdinalIgnoreCase))
              encryptSections = true;
          }
          
        }
        
        if (fileName == null)
        {
          Console.WriteLine("ERROR: File name must be provided in INPUTFILE section.");
          return retType.FAIL;
        }
        
        // Insert the file into the AIS image
        if ( loadAddr != 0x00000000)
        {
          // binary image
          if ( entryPointAddr != 0x00000000)
          {
            devAISGen.InsertAISSecureObjectFile(fileName, loadAddr, entryPointAddr, encryptSections);
          }
          else
          {
            devAISGen.InsertAISSecureObjectFile(fileName, loadAddr, encryptSections);
          }
        }
        else
        {
          devAISGen.InsertAISSecureObjectFile(fileName, useEntryPoint, encryptSections);
        }
      }
      #endregion
      
      #region Handle ROM and AIS Extra Functions  
      // Handle ROM functions
      if (devAISGen.ROMFunc != null)
      {
        for (UInt32 j = 0; j < devAISGen.ROMFunc.Length; j++)
        {
          if (sec.sectionName.Equals(devAISGen.ROMFunc[j].iniSectionName, StringComparison.OrdinalIgnoreCase))
          {
            UInt32 funcIndex = j;

            UInt32[] args = new UInt32[ (UInt32)devAISGen.ROMFunc[funcIndex].numParams ];
            
            for (Int32 k = 0; k < devAISGen.ROMFunc[funcIndex].numParams; k++)
            {
              //FIXME
              Debug.DebugMSG("\tParam name: {0}, Param num: {1}, Value: {2}\n",
                devAISGen.ROMFunc[funcIndex].paramNames[k],
                k, 
                sec.sectionValues[devAISGen.ROMFunc[funcIndex].paramNames[k].ToUpper()]);
                
              args[k] = (UInt32) sec.sectionValues[devAISGen.ROMFunc[funcIndex].paramNames[k].ToUpper()];
            }

            devAISGen.InsertAISFunctionExecute((UInt16) funcIndex, (UInt16) devAISGen.ROMFunc[funcIndex].numParams, args);
            
            // Insert signature
            InsertAISSecureSignature(devAISGen);
          }
        }
      }
      
      // Handle AISExtras functions
      if (devAISGen.AISExtraFunc != null)
      {
        for (UInt32 j = 0; j < devAISGen.AISExtraFunc.Length; j++)
        {
          if (sec.sectionName.Equals(devAISGen.AISExtraFunc[j].iniSectionName, StringComparison.OrdinalIgnoreCase))
          {
            UInt32 funcIndex = j;
            
            // Load the AIS extras file if needed
            {
              IniSection tempSec = new IniSection();
              tempSec.sectionName = "INPUTFILE";
              tempSec.sectionValues = new Hashtable();
              tempSec.sectionValues["FILENAME"] = devAISGen.AISExtraFunc[funcIndex].aisExtraFileName;
              
              EmbeddedFileIO.ExtractFile(Assembly.GetExecutingAssembly(), devAISGen.AISExtraFunc[funcIndex].aisExtraFileName, true);
                    
              InsertAISCommandViaINI(devAISGen, tempSec);
              
              // Use symbols to get address for AISExtra functions and parameters
              for (Int32 k = 0; k < devAISGen.AISExtraFunc.Length; k++)
              {
                ObjectFile tempFile = FindFileWithSymbol(devAISGen, devAISGen.AISExtraFunc[funcIndex].funcName);
                if (tempFile == null)
                {
                  // Try looking for underscore version
                  tempFile = FindFileWithSymbol(devAISGen, "_" + devAISGen.AISExtraFunc[funcIndex].funcName);
                }
                
                if (tempFile != null)
                {
                  ObjectSymbol tempSym = tempFile.symFind(devAISGen.AISExtraFunc[funcIndex].funcName);
                  if (tempSym == null)
                  {
                    // Try looking for underscore version
                    tempSym = tempFile.symFind("_"+devAISGen.AISExtraFunc[funcIndex].funcName);
                  }
                  
                  devAISGen.AISExtraFunc[funcIndex].funcAddr = (UInt32) tempSym.value;
                  tempSym = tempFile.symFind(".params");
                  devAISGen.AISExtraFunc[funcIndex].paramAddr = (UInt32) tempSym.value;
                }
                else
                {
                  // The function name was not found - that's a big problem with our 
                  // device specific AISGen class.
                  Console.WriteLine("AIS extra function, {0}, not found in file {1}.", 
                                    devAISGen.AISExtraFunc[funcIndex].funcName, 
                                    devAISGen.AISExtraFunc[funcIndex].aisExtraFileName);
                  return retType.FAIL;
                }
              }
              
            }
            for (Int32 k = 0; k < devAISGen.AISExtraFunc[funcIndex].numParams; k++)
            {
              // Write SET command
              devAISGen.InsertAISSet(
                (UInt32)0x3,    // Write type field (32-bit only)
                (UInt32) (devAISGen.AISExtraFunc[funcIndex].paramAddr + (k * 4)), 
                (UInt32)sec.sectionValues[devAISGen.AISExtraFunc[funcIndex].paramNames[k].ToString()],
                (UInt32)0x0 );  // Write Sleep value (should always be zero)
                
              // Insert signature
              InsertAISSecureSignature(devAISGen);
            }

            // Now that params are set, Jump to function
            devAISGen.InsertAISJump(devAISGen.AISExtraFunc[funcIndex].funcAddr);
            
            // Insert signature
            InsertAISSecureSignature(devAISGen);
          }
        }
      }
      #endregion
      
      #region Handle AIS Command Sections
      
      if (sec.sectionName.Equals("AIS_EnableCRC", StringComparison.OrdinalIgnoreCase))
      {
        devAISGen.InsertAISEnableCRC();
      }
      
      if (sec.sectionName.Equals("AIS_DisableCRC", StringComparison.OrdinalIgnoreCase))
      {
        devAISGen.InsertAISDisableCRC();
      }
      
      if (sec.sectionName.Equals("AIS_RequestCRC", StringComparison.OrdinalIgnoreCase))
      {
        UInt32 crcValue = 0x00000000;
        Int32 seekValue = -12;
        
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("CRCValue", StringComparison.OrdinalIgnoreCase))
          {
            crcValue = (UInt32)sec.sectionValues["CRCVALUE"];
          }
          if (((String)de.Key).Equals("SEEKValue", StringComparison.OrdinalIgnoreCase))
          {
            seekValue = (Int32)sec.sectionValues["SEEKVALUE"];
          }
        }
        if (devAISGen.InsertAISRequestCRC(crcValue, seekValue) != retType.SUCCESS)
        {
          Console.WriteLine("WARNING: Final function register AIS command failed.");
        }
      }
      
      if (sec.sectionName.Equals("AIS_Jump", StringComparison.OrdinalIgnoreCase))
      {
        String symbolName = "";
        UInt32 address = 0x00000000;
      
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("LOCATION", StringComparison.OrdinalIgnoreCase))
          {
            symbolName = (String)sec.sectionValues["LOCATION"];
          }
        }
        // See if string is number (address)
        if (UInt32.TryParse(symbolName, out address))
        {
          if (devAISGen.InsertAISJump(address) != retType.SUCCESS)
          {
            Console.WriteLine("WARNING: AIS Jump to {0} was not inserted.",symbolName);
          }
        }
        else
        {
          if (devAISGen.InsertAISJump(symbolName) != retType.SUCCESS)
          {
            Console.WriteLine("WARNING: AIS Jump to {0} was not inserted.",symbolName);
          }
        }
        
        // Insert signature
        InsertAISSecureSignature(devAISGen);
      }
      
      if (sec.sectionName.Equals("AIS_JumpClose", StringComparison.OrdinalIgnoreCase))
      {
        String symbolName = "";
        UInt32 address = 0x00000000;
      
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("ENTRYPOINT", StringComparison.OrdinalIgnoreCase))
          {
            symbolName = (String)sec.sectionValues["ENTRYPOINT"];
          }
        }
        
        if (symbolName == "")
        {
          devAISGen.InsertAISJumpClose(devAISGen.entryPoint);
        }
        else
        {
          // See if string is number (address)
          if (UInt32.TryParse(symbolName, out address))
          {
            if (devAISGen.InsertAISJumpClose(address) != retType.SUCCESS)
            {
              Console.WriteLine("WARNING: AIS Jump to {0} was not inserted.",symbolName);
            }
          }
          else
          {
            if (devAISGen.InsertAISJumpClose(symbolName) != retType.SUCCESS)
            {
              Console.WriteLine("WARNING: AIS Jump to {0} was not inserted.",symbolName);
            }
          }
        }
        
        // Insert signature
        InsertAISSecureSignature(devAISGen);
      }
      
      if (sec.sectionName.Equals("AIS_Set", StringComparison.OrdinalIgnoreCase))
      {
        UInt32 type     = 0x00000000;
        UInt32 address  = 0x00000000;
        UInt32 data     = 0x00000000;
        UInt32 sleep    = 0x00000000;
      
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (sec.sectionValues["TYPE"].GetType() == typeof(String))
          {          
            if (((String)de.Key).Equals("TYPE", StringComparison.OrdinalIgnoreCase))
            {
              if (! UInt32.TryParse((String)sec.sectionValues["TYPE"], out type))
              {
                try
                {
                  type = (UInt32)Enum.Parse(typeof(AisSetType),(String)sec.sectionValues["TYPE"]);
                }
                catch (ArgumentException e)
                {
                  Console.WriteLine((String)sec.sectionValues["TYPE"] + " is not allowed specifier for SET type.");
                  Console.WriteLine(e.Message);
                  return retType.FAIL;
                }
              }
            }
          }
          else
          {
            type = (UInt32)sec.sectionValues["TYPE"];
          }
          if (((String)de.Key).Equals("ADDRESS", StringComparison.OrdinalIgnoreCase))
          {
            address = (UInt32)sec.sectionValues["ADDRESS"];
          }
          if (((String)de.Key).Equals("DATA", StringComparison.OrdinalIgnoreCase))
          {
            data = (UInt32)sec.sectionValues["DATA"];
          }
          if (((String)de.Key).Equals("SLEEP", StringComparison.OrdinalIgnoreCase))
          {
            sleep = (UInt32)sec.sectionValues["SLEEP"];
          }
          
        }
        devAISGen.InsertAISSet(type, address, data, sleep);
        
        // Insert signature
        InsertAISSecureSignature(devAISGen);
      }
      
      if (sec.sectionName.Equals("AIS_SectionFill", StringComparison.OrdinalIgnoreCase))
      {
        UInt32 address  = 0x00000000;
        UInt32 size     = 0x00000000;
        UInt32 type     = 0x00000000;
        UInt32 pattern  = 0x00000000;
      
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("ADDRESS", StringComparison.OrdinalIgnoreCase))
          {
            address = (UInt32)sec.sectionValues["ADDRESS"];
          }
          if (((String)de.Key).Equals("SIZE", StringComparison.OrdinalIgnoreCase))
          {
            size = (UInt32)sec.sectionValues["SIZE"];
          }
          if (((String)de.Key).Equals("TYPE", StringComparison.OrdinalIgnoreCase))
          {
            type = (UInt32)sec.sectionValues["TYPE"];
          }
          if (((String)de.Key).Equals("PATTERN", StringComparison.OrdinalIgnoreCase))
          {
            pattern = (UInt32)sec.sectionValues["PATTERN"];
          }
        }
        devAISGen.InsertAISSectionFill( address, size, type, pattern);
      }
      
      if (sec.sectionName.Equals("AIS_FastBoot", StringComparison.OrdinalIgnoreCase))
      {
        devAISGen.InsertAISFastBoot();
      }
      
      if (sec.sectionName.Equals("AIS_ReadWait", StringComparison.OrdinalIgnoreCase))
      {
        UInt32 address  = 0x00000000;
        UInt32 mask     = 0xFFFFFFFF;
        UInt32 data     = 0xFFFFFFFF;
      
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("ADDRESS", StringComparison.OrdinalIgnoreCase))
          {
            address = (UInt32)sec.sectionValues["ADDRESS"];
          }
          if (((String)de.Key).Equals("MASK", StringComparison.OrdinalIgnoreCase))
          {
            mask = (UInt32)sec.sectionValues["MASK"];
          }
          if (((String)de.Key).Equals("DATA", StringComparison.OrdinalIgnoreCase))
          {
            data = (UInt32)sec.sectionValues["DATA"];
          }
        }
        devAISGen.InsertAISReadWait(address, mask, data);
        
        // Insert signature
        InsertAISSecureSignature(devAISGen);
      }
      
      if (sec.sectionName.Equals("AIS_SeqReadEnable", StringComparison.OrdinalIgnoreCase))
      {
        devAISGen.InsertAISSeqReadEnable();
      }
      
      if (sec.sectionName.Equals("AIS_FinalFunctionReg", StringComparison.OrdinalIgnoreCase))
      {
        String finalFxnName = "";
        
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("FINALFXNSYMBOLNAME", StringComparison.OrdinalIgnoreCase))
          {
            finalFxnName = (String)sec.sectionValues["FINALFXNSYMBOLNAME"];
          }
        }
        if (devAISGen.InsertAISFinalFxnReg(finalFxnName) != retType.SUCCESS)
        {
          Console.WriteLine("WARNING: Final function register AIS command failed.");
        }
      }
      
      if (sec.sectionName.Equals("AIS_SetDelegateKey", StringComparison.OrdinalIgnoreCase))
      {
        String rsaKeyFileName = null;
        String keyString = null;
        
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          if (((String)de.Key).Equals("RSAKEYFILENAME", StringComparison.OrdinalIgnoreCase))
          {
            rsaKeyFileName = (String)sec.sectionValues["RSAKEYFILENAME"];
          }
          if (((String)de.Key).Equals("ENCRYPTIONKEY", StringComparison.OrdinalIgnoreCase))
          {
            keyString = (String)sec.sectionValues["ENCRYPTIONKEY"];
          }
        }
        
        if ((rsaKeyFileName == null) || (keyString == null))
        {
          Console.WriteLine("ERROR: Key file name and encryption key must be given.");
          return retType.FAIL;
        }
        
        if (devAISGen.InsertAISSetDelegateKey(rsaKeyFileName, keyString) != retType.SUCCESS)
        {
          Console.WriteLine("WARNING: Set Delegate Key AIS command failed.");
        }
      }
      
      if (sec.sectionName.Equals("AIS_RemoveDelegateKey", StringComparison.OrdinalIgnoreCase))
      {
        devAISGen.InsertAISRemDelegateKey();
      }
      
      #endregion  
    
      return retType.SUCCESS;
    }
        
    /// <summary>
    /// Signature insertion creation and insertion routine
    /// </summary>
    public static retType InsertAISSecureSignature( AISGen devAISGen )
    {
      EndianBinaryWriter ebw = new EndianBinaryWriter(
          devAISGen.devAISStream,
          devAISGen.devEndian);
      
      // Make sure all data is present for signature calculation
      devAISGen.sigStream.Flush();
      
      // Reset stream to start
      devAISGen.sigStream.Position = 0;
      
    #if (DEBUG)
      // For debug write the data out to file
      Byte[] sigData = null;
      sigData = ((MemoryStream) devAISGen.sigStream).ToArray();
      FileIO.SetFileData("sig_data.bin",sigData,true);
    #endif

      // Calculate hash of data
      Byte[] hash = devAISGen.currHashAlgorithm.ComputeHash(devAISGen.sigStream);
      Console.WriteLine("\tSignature Hash: {0}", BitConverter.ToString(hash));
      Console.WriteLine("\tSignature Byte Count = {0}", devAISGen.sigStream.Length);
      Byte[] signatureData = null;
      
      // Generate signature via encryption
      if ( devAISGen.SecureType == AisSecureType.GENERIC )
      {
        signatureData = new Byte[32];
               
        // Fill signature data buffer with random bytes
        (new Random()).NextBytes(signatureData);
        
        // Copy calculated SHA hash into signature data buffer
        hash.CopyTo(signatureData,0);

        using (MemoryStream ms = new MemoryStream())
        {
          Aes myAES = new AesManaged();
          myAES.KeySize = 128;
          myAES.Mode = CipherMode.CBC;
          myAES.Padding = PaddingMode.None;
          ICryptoTransform encryptor = myAES.CreateEncryptor(devAISGen.customerEncryptionKey, devAISGen.CEKInitialValue);
          CryptoStream cs = new CryptoStream(ms,encryptor,CryptoStreamMode.Write);
          cs.Write(signatureData,0,signatureData.Length);
          cs.FlushFinalBlock();
          ms.ToArray().CopyTo(signatureData,0);
        }
      }
      else if ( devAISGen.SecureType == AisSecureType.CUSTOM )
      {

        RSAPKCS1SignatureFormatter rsaFormatter = new RSAPKCS1SignatureFormatter(devAISGen.rsaObject);

        // Create a signature for HashValue and return it.
        signatureData = rsaFormatter.CreateSignature(devAISGen.currHashAlgorithm);
        
        // Signature info needs to be revered to work with RSA functionality in ROM
        Array.Reverse(signatureData);
      }
      
      // Write the signature data to the output AIS binary writer
      ebw.Write(signatureData);
        
      // Clear the signature stream now that we have used the data for signature generation
      devAISGen.sigStream.SetLength(0);
      devAISGen.sigStream.Position = 0;
      
      return retType.SUCCESS;
    }

  #endregion

  #region Private Static Methods
    /// <summary>
    /// AIS Section Load command generation
    /// </summary>
    /// <param name="cf">The COFFfile object that the section comes from.</param>
    /// <param name="secHeader">The Hashtable object of the section header to load.</param>
    /// <param name="devAISGen">The specific device AIS generator object.</param>
    /// <returns>retType enumerator indicating success or failure.</returns>
    private static retType AISSecureSectionLoad( AISGen devAISGen, ObjectFile file, ObjectSection section, Boolean encryptSection)
    {
      Byte[] secData = file.secRead(section);

      // Write Section_Load AIS command, load address, and size
      if (encryptSection)
      {
        Byte[] encData = null;

        // Encrypt data using CTS algorithm
        try
        {
          encData = AesManagedUtil.AesCTSEncrypt(secData,devAISGen.customerEncryptionKey,devAISGen.CEKInitialValue);
        }
        catch(Exception e)
        {
          Console.WriteLine("Exception during encryption operation: {0}",e.Message);
          return retType.FAIL;
        }
        
        if (encData != null)
        {
          devAISGen.InsertAISEncSectionLoad((UInt32) section.loadAddr, (UInt32) section.size, secData, encData);
        }
        else
        {
          Console.WriteLine("Section encryption failed.");
          return retType.FAIL;
        }
      }
      else
      {
        devAISGen.InsertAISSectionLoad((UInt32) section.loadAddr, (UInt32) section.size, secData);
      }
      
      // Add this section's memory range, checking for overlap
      AddMemoryRange(devAISGen, (UInt32) section.loadAddr, (UInt32) (section.loadAddr+section.size-1));
      
      return retType.SUCCESS;
    }

    /// <summary>
    /// AIS COFF file Load command generation (loads all sections)
    /// </summary>
    /// <param name="cf">The COFFfile object that the section comes from.</param>
    /// <param name="devAISGen">The specific device AIS generator object.</param>
    /// <returns>retType enumerator indicating success or failure.</returns>
    private static retType AISSecureObjectFileLoad( AISGen devAISGen, ObjectFile file )
    {
      UInt32 loadedSectionCount = 0;
      
      // Check if object file already loaded
      if (FindObjectFile(devAISGen,file.FileName) != null)
      {
        return retType.FAIL;
      }
      
      // Ii this is a new file, let's add it to our list
      devAISGen.objectFiles.Add(file);
    
      // Make sure we have an endianness match be
      // FIXME - Is this a good idea, what about binary files?
      if (!devAISGen.devEndian.ToString().Equals(file.Endianness))
      {
        Console.WriteLine("Endianness mismatch. Device is {0} endian, Object file is {1} endian",
            devAISGen.devEndian.ToString(),
            file.Endianness);
        return retType.FAIL;
      }
     
      // Make sure the .TIBoot section is first (if it exists)
      ObjectSection firstSection = file.LoadableSections[0];
      for (int i = 1; i < file.LoadableSectionCount; i++)
      {
        if ((file.LoadableSections[i].name).Equals(".TIBoot"))
        {
          file.LoadableSections[0] = file.LoadableSections[i];
          file.LoadableSections[i] = firstSection;
          break;
        }
      }      
      
      // Do all SECTION_LOAD commands
      for (Int32 i = 0; i < file.LoadableSectionCount; i++)
      {
        Boolean encryptSection = false;
        
        // Determine section encryption status
        if (devAISGen.sectionsToEncrypt != null)
        {
          if ( (devAISGen.sectionsToEncrypt.Length == 1) && devAISGen.sectionsToEncrypt[0].Equals("ALL"))
          {
            encryptSection = true;
            Console.WriteLine("Encrypting section {0}, since ALL was specified for encryptSections in ini file.",file.LoadableSections[i].name);
          }
          else
          {
            if ( Array.IndexOf(devAISGen.sectionsToEncrypt,file.LoadableSections[i].name) >= 0 )
            {
              encryptSection = true;
              Console.WriteLine("Encrypting section {0}, since it was explicitly specified in encryptSections in ini file.",file.LoadableSections[i].name);
            }
          }
        }
      
        // Perform secure section load
        if (AISSecureSectionLoad(devAISGen, file, file.LoadableSections[i], encryptSection) != retType.SUCCESS)
        {
          return retType.FAIL;
        }
        
        // Check for need to do TIBoot initialization
        if ( (loadedSectionCount == 0) && ((file.LoadableSections[i].name).Equals(".TIBoot")) )
        {
          devAISGen.InsertAISJump("_TIBootSetup");
          InsertAISSecureSignature(devAISGen);
        }
        
        loadedSectionCount++;
      } 
      // End of SECTION_LOAD commands
      
      // Now that we are done with file contents, we can close it
      file.Close();
      
      return retType.SUCCESS;
    }
  
    private static retType SecurityIniSectionParse(AISGen devAISGen, IniFile iniFile)
    {
      String currHashAlgorithmString = "SHA1";  // Default hash algorithm
      
      // Get data from the GENERAL INI Section
      IniSection sec = iniFile.GetSectionByName("Security");
    
    #region INI Section parsing
      if (sec != null)
      {
        foreach (DictionaryEntry de in sec.sectionValues)
        {
          // Security Type
          if (((String)de.Key).Equals("SECURITYTYPE", StringComparison.OrdinalIgnoreCase))
          {
            devAISGen.SecureType = (AisSecureType) Enum.Parse(typeof(AisSecureType), (String)sec.sectionValues["SECURITYTYPE"], true);
          }
          
          // Boot exit type
          if (((String)de.Key).Equals("BOOTEXITTYPE", StringComparison.OrdinalIgnoreCase))
          {
            devAISGen.bootLoaderExitType = (BootLoaderExitType) Enum.Parse(typeof(BootLoaderExitType), (String)sec.sectionValues["BOOTEXITTYPE"], true);
          }
          
          // Encrypted section settings
          if (((String)de.Key).Equals("ENCRYPTSECTIONS", StringComparison.OrdinalIgnoreCase))
          {
            Char[] separators = new Char[]{','};
            String encryptSections = (String)sec.sectionValues["ENCRYPTSECTIONS"];
            devAISGen.sectionsToEncrypt = encryptSections.Split(separators,StringSplitOptions.RemoveEmptyEntries);
            for( int k = 0; k<devAISGen.sectionsToEncrypt.Length; k++)
            {
              devAISGen.sectionsToEncrypt[k] = devAISGen.sectionsToEncrypt[k].Trim();
            }
          }
          
          // AES Encryption Key (CEK)
          if (((String)de.Key).Equals("ENCRYPTIONKEY", StringComparison.OrdinalIgnoreCase))
          {
            devAISGen.customerEncryptionKey = new Byte[16];
            devAISGen.rootCustomerEncryptionKey = null;
            devAISGen.CEKInitialValue = new Byte[16];
            devAISGen.rootCEKInitialValue = null;
            
            String keyString = (String)sec.sectionValues["ENCRYPTIONKEY"];
            if (keyString.Length != 32)
            {
              Console.WriteLine("AES Encryption Key is wrong length!");
              return retType.FAIL;
            }
            for (int j=0; j<keyString.Length; j+=2)
            {
              devAISGen.customerEncryptionKey[(j>>1)] = Convert.ToByte(keyString.Substring(j,2),16);
            }
            
            // Save base CEK key for restoring after delegate key removal
            devAISGen.rootCustomerEncryptionKey = devAISGen.customerEncryptionKey;
            
            // Generate IV as encrypted version of AES Key
            using (MemoryStream ms = new MemoryStream(devAISGen.CEKInitialValue))
            {
              Aes myAES = new AesManaged();
              myAES.KeySize = 128;
              myAES.Mode = CipherMode.ECB;
              myAES.Padding = PaddingMode.None;
              ICryptoTransform encryptor = myAES.CreateEncryptor(devAISGen.customerEncryptionKey, new Byte[16]);
              CryptoStream cs = new CryptoStream(ms,encryptor,CryptoStreamMode.Write);
              cs.Write(devAISGen.customerEncryptionKey,0,devAISGen.customerEncryptionKey.Length);
            }  
            devAISGen.rootCEKInitialValue = devAISGen.CEKInitialValue;
          }
          
          // Key Encryption Key (not normally known, here for debug/testing purposes)
          if (((String)de.Key).Equals("KEYENCRYPTIONKEY", StringComparison.OrdinalIgnoreCase))
          {
            devAISGen.keyEncryptionKey = new Byte[16];
            String keyString = (String)sec.sectionValues["KEYENCRYPTIONKEY"];
            if (keyString.Length != 32)
            {
              Console.WriteLine("Key Encryption Key is wrong length!");
              return retType.FAIL;
            }
            for (int j=0; j<keyString.Length; j+=2)
            {
              devAISGen.keyEncryptionKey[(j>>1)] = Convert.ToByte(keyString.Substring(j,2),16);
            }
          }
          
          // Generic Secure Option to force JTAG off as part of the key loading
          if (((String)de.Key).Equals("GENERICJTAGFORCEOFF", StringComparison.OrdinalIgnoreCase))
          {
            if (((String)sec.sectionValues["GENERICJTAGFORCEOFF"]).Equals("TRUE", StringComparison.OrdinalIgnoreCase))
              devAISGen.genericJTAGForceOff = true;
            else
              devAISGen.genericJTAGForceOff = false;
          }
          
          // Generic Secure Option to select the hash algorithm for signatures
          if (((String)de.Key).Equals("GENERICSHASELECTION", StringComparison.OrdinalIgnoreCase))
          {
            currHashAlgorithmString = (String)sec.sectionValues["GENERICSHASELECTION"];
            devAISGen.currHashAlgorithmValue = (SHA_Algorithm) Enum.Parse(typeof(SHA_Algorithm), currHashAlgorithmString, true);
          }
          
          // Generic Key Header file
          if (((String)de.Key).Equals("GENKEYHEADERFILENAME", StringComparison.OrdinalIgnoreCase))
          {
            String genKeyHeaderFileName = (String)sec.sectionValues["GENKEYHEADERFILENAME"];
            devAISGen.genericKeyHeaderData = new Byte[32];
            // Open file, read contents, copy to our AIS array
            using (FileStream tempFS = new FileStream(genKeyHeaderFileName, FileMode.Open, FileAccess.Read))
            {
              tempFS.Read(devAISGen.genericKeyHeaderData,0,32);
            }
          }
          
          // Custom Secure RSA Key File
          if (((String)de.Key).Equals("RSAKEYFILENAME", StringComparison.OrdinalIgnoreCase))
          {
            String rsaKeyFileName = (String)sec.sectionValues["RSAKEYFILENAME"];
            devAISGen.rsaObject = RSAKey.LoadFromFile(rsaKeyFileName);
            devAISGen.rootRsaObject = null;
            
            if (devAISGen.rsaObject == null)
            {
              Console.WriteLine("RSA key loading failed!");
              return retType.FAIL;
            }
            
            // Update the hash algo string if RSA key size is 2048 bits
            if (devAISGen.rsaObject.KeySize == 2048)
            {
              currHashAlgorithmString = "SHA256";
              devAISGen.currHashAlgorithmValue = SHA_Algorithm.SHA256;
            }
            
            // Save root public key reference for use after delegate key removal
            devAISGen.rootRsaObject = devAISGen.rsaObject;
          }
        }
      }
      else 
      {
        sec = iniFile.GetSectionByName("SecureLegacy");
        if (sec != null)
        {
          devAISGen.bootMode = AisBootModes.LEGACY;
        }
        else
        {
          Console.WriteLine("INI file is missing a SECURITY or SECURELEGACY section.");
          return retType.FAIL;
        }
      }
    #endregion
      
    #region Security INI Input Validation
      // 2) Make sure a secure type has been specified
      if (devAISGen.SecureType == AisSecureType.NONE)
      {
        Console.WriteLine("ERROR: The device's security type was not specified!");
        return retType.FAIL;
      }
      else
      {
        Console.WriteLine("Creating boot image for a {0} secure device.",devAISGen.SecureType.ToString().ToLower());
        // 3) Make sure we have a CEK and IV
        if (devAISGen.customerEncryptionKey == null)
        {
          Console.WriteLine("ERROR: No encryption key was specified!");
          return retType.FAIL;
        }
        
        // 4) If custom secure, make sure we have an rsaObject
        if ((devAISGen.SecureType == AisSecureType.CUSTOM) && (devAISGen.rsaObject == null))
        {
          Console.WriteLine("ERROR: No RSA key file was specified!");
          return retType.FAIL;
        }
        
        // 5) Make sure RSA key size is supported
        if ((devAISGen.SecureType == AisSecureType.CUSTOM) && (devAISGen.rsaObject != null))
        {
          if ( (devAISGen.rsaObject.KeySize != 1024) && (devAISGen.rsaObject.KeySize != 2048) )
          {
            Console.WriteLine("ERROR: No RSA key size is invalid!");
            return retType.FAIL;
          }
          else
          {
            Console.WriteLine("INFO: RSA key is {0} bits.",devAISGen.rsaObject.KeySize);
          }
        }
        
        // 6) Specify Boot Exit type (if not legacy boot)
        if (devAISGen.bootMode != AisBootModes.LEGACY)
        {
          if (devAISGen.bootLoaderExitType == BootLoaderExitType.NONE)
          {
            Console.WriteLine("ERROR: No boot loader exit type was specified!");
            return retType.FAIL;
          }
          else
          {
            Console.WriteLine("INFO: Boot exit type has been selected as {0}.",devAISGen.bootLoaderExitType);
          }
        }

        // 7) If generic secure, make sure we have the CEK header info (or KEK though that's not typical use-case)
        if ( (devAISGen.SecureType == AisSecureType.GENERIC) &&
             (devAISGen.genericKeyHeaderData == null) &&
             (devAISGen.keyEncryptionKey == null) )
        {
          Console.WriteLine("WARNING: Encrypted Key Header data is absent - generating plaintext version. ");
          Console.WriteLine("         The Customer Encryption Key will be transferred in plaintext!  ");
        }
        
        // 8) Give warning if generic device and no sections are specified for encryption
        if (devAISGen.bootMode != AisBootModes.LEGACY)
        {
          if ((devAISGen.SecureType == AisSecureType.GENERIC) && (devAISGen.sectionsToEncrypt == null))
          {
            Console.WriteLine("WARNING: Generic Secure device was specified, but no input sections were indicated for encryption.");
            Console.WriteLine("         Only boot image signing will take place.");
          }
        }
      }

      // 9) Make sure valid hash algorithm was selected      
      try
      {
        devAISGen.currHashAlgorithm = HashAlgorithm.Create(currHashAlgorithmString);
        Console.WriteLine("INFO: Current SHA algorithm is {0}.",devAISGen.currHashAlgorithmValue);
      }
      catch (Exception e)
      {
        Console.WriteLine("Invalid Hash Algorithm Selected. Exception message: {0}.",e.Message);
        return retType.FAIL;
      }
    #endregion
    
    #region Secure data/structure creation
      if ( devAISGen.SecureType == AisSecureType.GENERIC )
      {
        // Create our own key header
        devAISGen.secureKeyData = new Byte[32];
        Byte[] tempHeaderData   = new Byte[32];
        
        // Init with Random Data
        (new Random()).NextBytes(tempHeaderData);
                  
        // Copy in the magic word for the generic key header structure
        BitConverter.GetBytes((UInt32)SecureLoadMagic.GENKEY_MAGIC).CopyTo(tempHeaderData, 0);
        
        // Insert JTAGForceOff word
        BitConverter.GetBytes( (UInt32) (devAISGen.genericJTAGForceOff ? 0x00000001 : 0x00000000) ).CopyTo(tempHeaderData, 4);
        
        // Insert Hash algorithm selection word
        BitConverter.GetBytes( (UInt32)devAISGen.currHashAlgorithmValue ).CopyTo(tempHeaderData, 8);
        
        // Insert key Data (at offset 16)
        devAISGen.customerEncryptionKey.CopyTo(tempHeaderData, 16);
      
        // If KEK is provided, use it to generate the header for the boot image
        if (devAISGen.keyEncryptionKey != null)
        {
          Byte[] iv = new Byte[16];
          using (MemoryStream ms = new MemoryStream(iv))
          {
            Aes myAES = new AesManaged();
            myAES.KeySize = 128;
            myAES.Mode = CipherMode.ECB;
            myAES.Padding = PaddingMode.None;
            ICryptoTransform encryptor = myAES.CreateEncryptor(devAISGen.keyEncryptionKey, new Byte[16]);
            CryptoStream cs = new CryptoStream(ms,encryptor,CryptoStreamMode.Write);
            cs.Write(devAISGen.keyEncryptionKey,0,devAISGen.keyEncryptionKey.Length);
          }
          
          Byte[] encSecureKeyData = new Byte[32];
          using (MemoryStream ms = new MemoryStream(encSecureKeyData))
          {
            Aes myAES = new AesManaged();
            myAES.KeySize = 128;
            myAES.Mode = CipherMode.CBC;
            myAES.Padding = PaddingMode.None;
            ICryptoTransform encryptor = myAES.CreateEncryptor(devAISGen.keyEncryptionKey, iv);
            CryptoStream cs = new CryptoStream(ms,encryptor,CryptoStreamMode.Write);
            cs.Write(tempHeaderData,0,32);
          }
        //#if (DEBUG)
          // For debug write the data out to file
          FileIO.SetFileData("gen_keyhdr_unencrypted.bin",tempHeaderData,true);
          FileIO.SetFileData("gen_keyhdr_encrypted.bin",encSecureKeyData,true);
        //#endif
        
          Array.Copy(encSecureKeyData,0,devAISGen.secureKeyData,0,32);
        }
      
        // We have a file being provided - use first 32 bytes of it
        else if ((devAISGen.genericKeyHeaderData != null) && (devAISGen.genericKeyHeaderData.Length >= 32))
        {
          Array.Copy(devAISGen.genericKeyHeaderData,0,devAISGen.secureKeyData,0,32);
        }
        
        // There is no KEK to create an encrypted header and no file is provided with key data
        // so let's just use our own unencrypted data
        else
        {
          Array.Copy(tempHeaderData,0,devAISGen.secureKeyData,0,32);          
        }
      }
      else if ( devAISGen.SecureType == AisSecureType.CUSTOM )
      {
        // Create RPK Verify Struct
        devAISGen.secureKeyData = RSAKey.CreateCustomSecureKeyVerifyStruct(devAISGen.rsaObject);
      #if (DEBUG)    
        // For debug write the data out to file
        FileIO.SetFileData("rpk_struct.bin",devAISGen.secureKeyData,true);
       
        // Calculate the SHA hash of the Root Public Key
        Byte[] digest = devAISGen.currHashAlgorithm.ComputeHash(devAISGen.secureKeyData);
        
        // Write the full hash of the RPK struct
        // For debug write the data out to file
        FileIO.SetFileData("rpk_hash_full.bin",digest,true);
      
        for (int i=16;i<digest.Length;i++)
        {
          digest[i-16] ^= digest[i];
        }
        
        // Write the expected MPK (hash of RPK structure truncated to 128 bits) in binary format to file mpk.bin
        Byte[] mpk = new Byte[16];
        Array.Copy(digest,0,mpk,0,16);
        // For debug write the data out to file
        FileIO.SetFileData("mpk.bin",mpk,true);
      #endif
      }
    
    #endregion
    
      return retType.SUCCESS;
    }
    
  #endregion
  }
} //end of AISGenLib namespace

