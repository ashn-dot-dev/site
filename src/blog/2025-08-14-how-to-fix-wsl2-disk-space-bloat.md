How to Fix WSL2 Disk Space Bloat
================================

This morning when I hopped online for work I was greeted with a murder mystery.
For some unknown reason, my WSL2 Ubuntu installation had eaten over a third of
my 1 TB disk space even though the actual content within distro was less than
10 GB:

![WSL2 taking up 373 GB of disk space](/blog/2025-08-14-how-to-fix-wsl2-disk-space-bloat/ubuntu-373gb.png)

```ps1
PS C:\Users\me> wsl --list --verbose
  NAME              STATE           VERSION
* Ubuntu            Stopped         2
  docker-desktop    Stopped         2
PS C:\Users\me> wsl --system -d Ubuntu df -h /mnt/wslg/distro
Filesystem      Size  Used Avail Use% Mounted on
/dev/sdc       1007G  9.8G  946G   2% /mnt/wslg/distro
```

Turns out this happens because the virtual hard disk file used for this WSL2
Ubuntu installation had grown to 373 GB over time, but was not set to
automatically shrink back down in order to reclaim unused space.

```
PS C:\Users\me> Get-ChildItem "$env:LOCALAPPDATA\Packages" -Recurse -Name "ext4.vhdx" | ForEach-Object { "$env:LOCALAPPDATA\Packages\$_" }
C:\Users\me\AppData\Local\Packages\CanonicalGroupLimited.Ubuntu_79rhkp1fndgsc\LocalState\ext4.vhdx
PS C:\Users\me> (Get-Item (Get-ChildItem "$env:LOCALAPPDATA\Packages" -Recurse -Name "ext4.vhdx" | ForEach-Object { "$env:LOCALAPPDATA\Packages\$_" })).Length / 1GB
372.947265625
```

The `wsl2.exe` program has an an option `--set-sparse` which according to the
program help text should allow the disk space to be automatically reclaimed
when set to `true`:

```ps1
PS C:\Users\me> wsl --shutdown
PS C:\Users\me> wsl --manage Ubuntu --set-sparse true
```

However, running that command by itself did not immediately fix the issue on my
machine. My guess was that some other background task or system reboot would
normally kick off the auto compacting process, but I had work to do and was
getting kind of impatient. So the actual way I ended up solving this was to
un-set-sparse the distribution and use `diskpart` to manually compact the disk
from within a terminal running as administrator:

```ps1
PS C:\Users\me> wsl --shutdown
PS C:\Users\me> wsl --manage Ubuntu --set-sparse false
PS C:\Users\me> diskpart

Microsoft DiskPart version 10.0.26100.1150

Copyright (C) Microsoft Corporation.
On computer: MYLAP25

DISKPART> select vdisk file=C:\Users\me\AppData\Local\Packages\CanonicalGroupLimited.Ubuntu_79rhkp1fndgsc\LocalState\ext4.vhdx

DiskPart successfully selected the virtual disk file.

DISKPART> compact vdisk

  100 percent completed

DiskPart successfully compacted the virtual disk file.

DISKPART> exit

Leaving DiskPart...
PS C:\Users\me> (Get-Item (Get-ChildItem "$env:LOCALAPPDATA\Packages" -Recurse -Name "ext4.vhdx" | ForEach-Object { "$env:LOCALAPPDATA\Packages\$_" })).Length / 1GB
12.93359375
```

And just like that, **poof**, disk space cleared.

![WSL2 taking up 13 GB of disk space](/blog/2025-08-14-how-to-fix-wsl2-disk-space-bloat/ubuntu-13gb.png)

Case closed!
