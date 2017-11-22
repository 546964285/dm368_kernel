/****************************************************************
 *  TI OMAP-L138 Boot and Flash Utilities                       *
 *  (C) 2009-2010 Texas Instruments, Inc.                       *
 *                                                              *
 ****************************************************************/
 
This directory contains the source and build files for various utilities 
associated with booting and flashing the OMAP-L138 SoC device. These utilities 
include:
  AISUtils                - Contains tools for generating AIS boot images, which are intended for
                            the primary ROM boot loader of the device.
  GenECC                  - Used to generate NAND ECC parity data for the EMIF hardware
                            offline on a PC.
  SFH                     - Serial Flasher Host, command-line tool to connect to platform in 
                            UART boot mode for writing various flash devices.
  SFT                     - Serial Flasher Target, runs on the target platform, used by SFH to
                            do the actual flashing of on-board flash devices.
  SLH                     - Serial Loader Host, command-line tool to boot an AIS image to a 
                            platform in UART boot mode.
  UBL                     - Secondary boot loader code/images.


For more info on each program please look for readme files in the various subdirectories.
