using System;
using System.Collections.Generic;
using System.Text;

namespace OMAP_L138_AIS_GUI
{
    class Consts
    {
        public const int majorRev = 1;
        public const int minorRev = 7;
        public const String twitchRev = "c";

        public enum Tab
        {
            num_tabs = 9,
            general = 0,
            flash = 1,
            peripheral = 2,
            pll0 = 3,
            sdram = 4,
            pll1 = 5,
            ddr2 = 6,
            lpsc = 7,
            pinmux = 8
        }

        // note: these values must match the index of their respective
        //       options in the (first) "device type" combo box
        public enum RomID
        {
            d800k002 = 0,
            d800k004 = 1,
            d800k006 = 2
        }

        // note: these values must match the index of their respective
        //       options in the (second) "device type" combo box
        public enum BootType
        {
            arm = 0,
            dsp = 1
        }

        public enum BinAddr :uint
        {
            boilerplate = 0x80000000,   // default is a reserved word, so...
            invalid = 0xFFFFFFFF
        }

        // note: these values must match the index of their respective
        //       options in the "clock source" combo box
        public enum ClockSrc
        {
            crystal = 0,
            oscillator = 1
        }

        public enum ClockIn
        {
            min = 1,
            max = 50
        }

        public enum ClockMult
        {
            min = 1,
            max = 64
        }
        
        public enum ClockDiv
        {
            min = 1,
            max = 32
        }
        public enum ClockCPU
        {
            min = 1,
            max = 1000
        }
        
        public enum ClockEMAC
        {
            min = 1,
            max = 50
        }
        
        public enum ClockSDRAM
        {
            min = 1,
            max = 250
        }
        
        public enum ClockDDR2
        {
            min = 1,
            max = 1000
        }

        public enum ClockI2Cmod
        {
            min = 7
        }

        public enum ClockAUX
        {
            max = 50
        }

        public enum ClockSYS2
        {
            max = 500
        }

        public enum ClockSYS4
        {
            max = 250
        }

        public enum FlashWidth
        {
            eightbit = 0,
            sixteenbit = 1
        }

        public enum LpscDomain
        {
            min0 = 0,
            max0 = 15,
            min1 = 0,
            max1 = 31,
            enable = 3,
            disable = 2,
            syncRst = 1
        }

        public enum PinmuxReg
        {
            min = 0,
            max = 19
        }
    }
}
