@echo off

if not "%3"=="" goto OK

echo.
echo usage:
echo     "build <output_file_name> <source_files_path> <source_file_name1> [ <source_file_name2> ... ]"
echo.
echo Example 1:
echo     build MyProgram .. main.c
echo.
echo     Will create a file "MyProgram.elf" in the current directory from file
echo     "main.c" located in the parent directory.
echo.
echo Example 2:
echo     build MyProject . MyProgram.c MyRoutines.c
echo.
echo     Will create a file "MyProject.elf" in the current directory from files
echo     "MyProgram.c" and "MyRoutines.c" located in the current directory.
echo.
echo Notes:
echo     1) Files "sam3a8c_flash.ld" and "startup_sam3xa.c" must be in the same directory as the source files.
echo     2) "." (without quotes) denotes the current directory and ".." (without quotes) denotes the parent directory.
echo.
goto END

:OK

set CC_PATH="C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin"
set INCLUDE_PATH1="C:\Program Files (x86)\Atmel\Studio\7.0\Packs\arm\CMSIS\5.4.0\CMSIS\Core\Include"
set INCLUDE_PATH2="C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\SAM3A_DFP\1.0.50\include"

set OUTPUT_NAME=%1
set SOURCE_PATH=%2
set LINK_LIST=startup_sam3xa.c.o

%CC_PATH%\arm-none-eabi-gcc.exe -x c -mthumb -D__SAM3A8C__ -DDEBUG -I%INCLUDE_PATH1% -I%INCLUDE_PATH2% -Og -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99 -march=ARMv4T -MD -MP -MF "startup_sam3xa.d" -MT"startup_sam3xa.d" -MT"startup_sam3xa.c.o" -o "startup_sam3xa.c.o" "%SOURCE_PATH%/startup_sam3xa.c"

:COMPILE

%CC_PATH%\arm-none-eabi-gcc.exe -x c -mthumb -D__SAM3A8C__ -DDEBUG -I%INCLUDE_PATH1% -I%INCLUDE_PATH2% -Og -ffunction-sections -mlong-calls -g3 -Wall -c -std=gnu99 -march=ARMv4T -MD -MP -MF "%3.d" -MT"%3.d" -MT"%3.o" -o "%3.o" "%SOURCE_PATH%/%3"

set LINK_LIST=%LINK_LIST% %3.o

echo %LINK_LIST%

shift

if not "%3"=="" goto COMPILE

%CC_PATH%\arm-none-eabi-gcc.exe -o %OUTPUT_NAME%.elf %LINK_LIST% -mthumb -Wl,-Map="%OUTPUT_NAME%.map" -Wl,--start-group -Wl,--end-group -L"%SOURCE_PATH%" -Wl,--gc-sections -Tsam3a8c_flash.ld

%CC_PATH%\arm-none-eabi-objcopy.exe -O binary "%OUTPUT_NAME%.elf" "%OUTPUT_NAME%.bin"

%CC_PATH%\arm-none-eabi-objcopy.exe -O ihex -R .eeprom -R .fuse -R .lock -R .signature "%OUTPUT_NAME%.elf" "%OUTPUT_NAME%.hex"

%CC_PATH%\arm-none-eabi-objcopy.exe -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 --no-change-warnings -O binary "%OUTPUT_NAME%.elf" "%OUTPUT_NAME%.eep"

%CC_PATH%\arm-none-eabi-objdump.exe -h -S "%OUTPUT_NAME%.elf" > "%OUTPUT_NAME%.lss"

%CC_PATH%\arm-none-eabi-objcopy.exe -O srec -R .eeprom -R .fuse -R .lock -R .signature "%OUTPUT_NAME%.elf" "%OUTPUT_NAME%.srec"

%CC_PATH%\arm-none-eabi-objdump.exe -d "%OUTPUT_NAME%.elf" > "%OUTPUT_NAME%.S"

%CC_PATH%\arm-none-eabi-size.exe "%OUTPUT_NAME%.elf"

:END

