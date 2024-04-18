# Microsoft Developer Studio Project File - Name="IRControl.uvproj" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=IRControl.uvproj - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "IRControl.uvproj.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "IRControl.uvproj.mak" CFG="IRControl.uvproj - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "IRControl.uvproj - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "IRControl.uvproj - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "IRControl.uvproj - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "USE_HAL_DRIVER" /D "STM32F070x6"  /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\../Inc" /I "..\../Drivers/STM32F0xx_HAL_Driver/Inc" /I "..\../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" /I "..\../Drivers/CMSIS/Device/ST/STM32F0xx/Include" /I "..\../Drivers/CMSIS/Include" /I "..\..\Modbus\Modbus_PORTABLE" /I "..\..\Modbus\Modbus_CORE" /I "..\..\HARDWARE\RingBuff" /I "..\..\HARDWARE\APP_Jump_Bootloader" /I "..\..\HARDWARE\STMFLASH" /I "..\..\HARDWARE\APP" /I "..\..\RECORD" /I "..\..\ProtoOS" /I "..\..\HARDWARE\24CXX" /I "..\..\HARDWARE\IIC"  /D "USE_HAL_DRIVER" /D "STM32F070x6"  /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 user32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "IRControl.uvproj - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "USE_HAL_DRIVER" /D "STM32F070x6"  /YX /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\../Inc" /I "..\../Drivers/STM32F0xx_HAL_Driver/Inc" /I "..\../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" /I "..\../Drivers/CMSIS/Device/ST/STM32F0xx/Include" /I "..\../Drivers/CMSIS/Include" /I "..\..\Modbus\Modbus_PORTABLE" /I "..\..\Modbus\Modbus_CORE" /I "..\..\HARDWARE\RingBuff" /I "..\..\HARDWARE\APP_Jump_Bootloader" /I "..\..\HARDWARE\STMFLASH" /I "..\..\HARDWARE\APP" /I "..\..\RECORD" /I "..\..\ProtoOS" /I "..\..\HARDWARE\24CXX" /I "..\..\HARDWARE\IIC"  /D "USE_HAL_DRIVER" /D "STM32F070x6"  /YX /FD /GZ  /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 user32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 user32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "IRControl.uvproj - Win32 Release"
# Name "IRControl.uvproj - Win32 Debug"
# Begin Group "Application/MDK-ARM"

# PROP Default_Filter ""
# Begin Source File

SOURCE=".startup_stm32f070x6.s"
# End Source File
# End Group
# Begin Group "Application/User"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\../Src/main.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/gpio.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/iwdg.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/sys.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/stm32f0xx_hal_timebase_tim.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/usart.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/stm32f0xx_it.c"
# End Source File
# Begin Source File

SOURCE="..\../Src/stm32f0xx_hal_msp.c"
# End Source File
# End Group
# Begin Group "Drivers/STM32F0xx_HAL_Driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_iwdg.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart.c"
# End Source File
# Begin Source File

SOURCE="..\../Drivers/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart_ex.c"
# End Source File
# End Group
# Begin Group "Drivers/CMSIS"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\../Src/system_stm32f0xx.c"
# End Source File
# End Group
# Begin Group "Modbus_PORTABLE"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\Modbus\Modbus_PORTABLE\MemoryBuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_PORTABLE\Physical.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_PORTABLE\PhysicalRx.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_PORTABLE\PhysicalTx.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_PORTABLE\Protocol.c"
# End Source File
# End Group
# Begin Group "Modbus_CORE"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\Modbus\Modbus_CORE\mbSlaverCore.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_CORE\mbSlaverMap.c"
# End Source File
# Begin Source File

SOURCE="..\..\Modbus\Modbus_CORE\mbSlaverStr.c"
# End Source File
# End Group
# Begin Group "HARDWARE"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\HARDWARE\RingBuff\RingBuff.c"
# End Source File
# Begin Source File

SOURCE="..\..\HARDWARE\APP_Jump_Bootloader\Jump_Bootloader.c"
# End Source File
# Begin Source File

SOURCE="..\..\HARDWARE\STMFLASH\stmflash.c"
# End Source File
# Begin Source File

SOURCE="..\..\HARDWARE\APP\app.c"
# End Source File
# Begin Source File

SOURCE="..\..\HARDWARE\24CXX\24cxx.c"
# End Source File
# Begin Source File

SOURCE="..\..\HARDWARE\IIC\myiic.c"
# End Source File
# End Group
# Begin Group "RECORD"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\RECORD\RECORD.h"
# End Source File
# End Group
# Begin Group "RTOS"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\..\ProtoOS\lc.h"
# End Source File
# Begin Source File

SOURCE="..\..\ProtoOS\lc-addrlabels.h"
# End Source File
# Begin Source File

SOURCE="..\..\ProtoOS\lc-switch.h"
# End Source File
# Begin Source File

SOURCE="..\..\ProtoOS\pt.h"
# End Source File
# Begin Source File

SOURCE="..\..\ProtoOS\pt-sem.h"
# End Source File
# Begin Source File

SOURCE="..\..\ProtoOS\pt-timer.h"
# End Source File
# End Group
# Begin Group "::CMSIS"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
