#############################################################
# Device and Flash types for Makefile use                   #
#############################################################
#

# Generic string for device family
DEVSTRING=DM36x

#Architecture types
ARCHTYPES:=ARM

# Particular device types of the family

#DEVICETYPES:=DM36x_297_DDR243 DM36x_270_DDR216 DM36x_216_DDR173 DM36x_297_DDR270 DM36x_297_DDR277 DM36x_297_DDR173_OSC24 DM36x_432_DDR340 DM36x
DEVICETYPES:=DM36x_ARM216_DDR173_OSC19P2 DM36x_ARM445_DDR351_OSC24 DM36x_ARM270_DDR216_OSC27 DM36x_ARM297_DDR243_OSC24 DM36x_ARM297_DDR270_OSC24 DM36x_ARM297_DDR277_OSC27 DM36x_ARM432_DDR340_OSC24 DM36x_ARM216_DDR173_OSC24 DM36x
# Supported flash memory types for booting
#FLASHTYPES:=NAND 
FLASHTYPES:=NAND SDMMC
