/* --------------------------------------------------------------------------
    FILE        : SecureHexAIS_version.cs
    PURPOSE     : TI Booting and Flashing Utilities
    AUTHOR      : Daniel Allred
    DESC        : Common Version file for various SecureHexAIS tools
 ----------------------------------------------------------------------------- */

using System;
using System.Text;
using System.IO;
using System.Globalization;
using System.Reflection;
using System.Collections;
using System.Collections.Generic;

using TI.AISLib;
using TI.UtilLib;
using TI.UtilLib.IO;
using TI.UtilLib.Ini;
using TI.UtilLib.HexConv;

[assembly: AssemblyTitle("SecureHexAIS")]
[assembly: AssemblyVersion("1.25.*")]

namespace TIBootAndFlash
{
  partial class Program
  {
    static System.Version GetVersion()
    {
      // Assumes that in AssemblyInfo.cs, the version is specified as 1.0.* or the like,
      // with only 2 numbers specified;  the next two are generated from the date.
      return (System.Reflection.Assembly.GetExecutingAssembly().GetName().Version);
    }
    
    static Int32 GetBuildYear()
    {
      System.Version v = GetVersion();
      return (new DateTime( v.Build * TimeSpan.TicksPerDay + v.Revision * TimeSpan.TicksPerSecond * 2 ).AddYears(1999).Year);   
    }
  }
}
