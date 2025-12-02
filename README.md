# regfixup
This tool will help you fix broken registry hives.

Examples:
* BAD\_SYSTEM\_CONFIG\_INFO blue screen
* ERROR\_BADDB when using DISM/after a bad Windows Update

## Downloads
Releases page has the latest Windows build. For other platforms see "Build instructions"

## How to use
```
regfixup 1.1 (https://github.com/helloryuko/regfixup)
fix broken registry hives with desynchronized sequence numbers or wrong checksum

usage: regfixup [hive file location]

optional parameters:
  -h  show this help message
  -p  prefer primary sequence number
  -s  prefer secondary sequence number
  -c  only recalculate checksum

return codes:
  0   fixup successful or not required
  1   can't open file
  2   file isn't a regfile
  3   invalid parameters
```

## I just want to fix my BSOD, how?
### Find the broken registry hive
1. Boot into a Windows installation medium
2. Choose language and click "Repair options"
3. Click "Command Prompt"
4. Enter `dism /cleanup-image /restorehealth /image:C:`, where C: is the disk with your broken Windows installation
  * If you have an error about scratch partition, create a folder on some disk with `mkdir C:\somefolder` and add a `/scratchdir:C:\somefolder` parameter to DISM
5. The check should fail and it will output "The DISM log file can be found at ..."
6. Enter `notepad <path to log file>` without the brackets
7. Search for "ERROR\_BADDB" in the file
8. You will find a string that looks something like this: `Failed to load offline SOMETHING hive`. The word instead SOMETHIHNG is your hive.
### Fix the registry hive
**!!! WARNING !!!**
**Do a backup of your C:\Windows\System32\config folder before proceeding!**
**Don't blame me if something broke and you don't have a backup :)**
1. Somehow obtain a copy of regfixup. You can drop it into your pendrive besides your Windows installer
2. Enter `X:\regfixup.exe C:\Windows\System32\config\YOUR_HIVE_NAME`, where YOUR_HIVE_NAME is your hive name (duh)
3. Choose either primary or secondary sequence
4. Reboot, you can do that with `wpeutil reboot` command

## Build instructions
### Windows
0. Download Visual Studio
1. Open "Developer Command Prompt"
2. `cl regfixup.c`
### Linux/macOS
0. Install either clang or gcc
1. Open terminal
2. `clang regfixup.c -o regfixup` or `gcc regfixup.c -o regfixup`
