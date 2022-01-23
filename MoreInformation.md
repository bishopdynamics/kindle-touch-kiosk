# Kindle Touch Chromium Kiosk

Converting a Kindle Touch into a wall-mounted touchscreen kiosk for web-based interfaces like Home Assistant.

## Acknowledgements
This repository is a repackaging of [other people's work](https://wiki.mobileread.com/wiki/K5_Index). 
Credit to NiLuJe hwhw, schuhumi, and all the other fine people on MobileRead forums who made all this work, and shared it with us.

I chose to include all required binaries instead of linking to download sources (links can be found further down in "Other Hacks"), 
mostly out of fear that the original sources would eventually be gone. I think the work that has been done to make this possible is valuable, 
and worth preserving long-term. In that same vein, I have also included more documentation than strictly necessary.

## Introduction

The Kindle Touch, Model D01200 was the first touchscreen kindle produced by Amazon. It comes in two flavors: with or without 3G.
It has a Freescale i.MX508 SoC, with a single ARM Cortex-A8 (armv7l) core clocked at 800MHz, 250MB of RAM, 4GB of flash storage, 600x800 16-level grayscale e-ink display, and a serial port.
The 3G model has an HSDPA modem with fallback to EDGE/GPRS, with coverage via Amazon Whispernet (via AT&T in US). 

This particular model has a connector populated for the serial port, which is handy for hacking it. 

Amazon has long discontinued support and software updates for this model, which is actually great news because it means that
your device wont randomly update itself and break homebrew software. 

The downside to end of updates, is that the device comes with Linux kernel 2.6.31, so that presents some limitations.
For this project I will be using a vnc client on the kindle to connect to a headless docker container running chromium.
My original plan was to [install alpine linux in a chroot](https://github.com/schuhumi/alpine_kindle) and run chromium natively on the kindle, but that turns out to be
too demanding on the hardware, and we also do not have recent enough TLS support available to support HTTPS pages. 

Instead we will leverage [this fantastic native vnc client](https://github.com/hwhw/kindlevncviewer) on the kindle, and offload the
effort of running a modern browser to a docker container.

You will be able to ssh to it over wifi in order to configure it, but
it will auto-start straight into the url you give it, and sit there indefinitely. 
I use mine for HomeAssistant.

## Requirements
* Kindle Touch (aka KT, K5) model D01200
* a computer running a unix-like OS
* a willingness to accept that you might end up ruining your device
* a willingness to accept that if you do ruin your device, you will have no one to blame but yourself

## Serial port
You will not actually need to use the serial port for this project, unless you decide to remove the battery, or you need to downgrade firmware, or something goes wrong.

To pull off the back cover, pry along the bottom edge and peel it up. 
Peel it back about halfway up, and then you can slide the cover down and it will lift off.
At the very bottom of the board, near the right side, you will find a black 3-pin connector, with a cutout in the frame to access it.

On some models, especially the ones prior to the D01200, the serial port does not have a connector populated. 
You can solder tiny wires to it, or if you have the tools and expertise to do surface mount soldering you can put a connector on it.
[I found a connector on DigiKey](https://www.digikey.com/en/products/detail/molex/0781710003/2754172), but I haven't actually tried it myself.


The mate to that connector is a Molex Pico-EZMate 3-circuit, part of series 3690.
[Here it is on DigiKey.](https://www.digikey.com/en/products/detail/0369200306/WM26608-ND/10232997) 

The pins, from left to right (with the device upright) are: `Ground, RX, TX`.
This is a 1.8v TTL serial port; not all USB serial adapters can work with 1.8v, so you might need a level shifter. 


The serial port config is:
```yaml
baud: 115200
data bits: 8
parity: none
stop bits: 1
```

## Default root password
You will not actually need this, the jailbreak clears the password for you, and then you can change it afterward with: `passwd`

There are a couple websites that will take your device serial number and spit back the stock root password for that device. 
This is all they are actually doing:

```python3
# python3
import hashlib
print("fiona%s"%hashlib.md5("YOURSERIALNOSPACES\n".encode('utf-8')).hexdigest()[13:16])
```
Replace `YOURSERIALNOSPACES` with your serial (no spaces).

With this password, you can connect to the serial port of a stock device and do a jailbreak that way. In most cases this is
not neccessary, but its still handy info. Here is an article about how to do a serial jailbreak, [aimed at windows users](https://www.mobileread.com/forums/showthread.php?t=267541), and another [for linux users](https://www.mobileread.com/forums/showthread.php?p=3137590&postcount=1)

## Other Hacks

There are a TON of other neat things you can run on your kindle, especially if you actually want to use it as a reader.
The website `mobileread.com` seems to be where most kindle development is happening.

Here are some useful resources for K5 (aka Kindle 5, Kindle Touch, KT):
* [The K5 Index Page](https://wiki.mobileread.com/wiki/K5_Index) is the place to start, it links to pretty much everything you could want to do with your kindle.
* [The Kindle Touch Hacking Page](https://wiki.mobileread.com/wiki/Kindle_Touch_Hacking) has a lot of info specific to this device
* [Backup your kindle The Manual Way](https://wiki.mobileread.com/wiki/Kindle_Touch_Hacking#B:_The_manual_way) is my preffered method to backup the kindle, and a pretty neat use of netcat
* [Snapshots of NiLuJe's hacks](https://www.mobileread.com/forums/showthread.php?t=225030) is where the latest downloads of most useful hacks are. When you are reading through forum posts and someone refers to "the snapshots thread", this is what they are referring to.
* [Booting U-Boot and Linux over USB](https://www.mobileread.com/forums/showthread.php?t=341361) is another way to get the device to do your bidding
* [Building a custom initramfs for fun and profit](https://www.mobileread.com/forums/showthread.php?t=343310) may be helpful as well
* [Snapshots of Katadelos's Hacks](https://www.mobileread.com/forums/showthread.php?t=342535&highlight=kernel+versions) is where you can find some kernel hacking going on, still active in 2021!
* [Official Kindle Software Updates Page](https://www.amazon.com/gp/help/customer/display.html?nodeId=GKMQC26VQQMM8XSW) is where you can find official update files, if you want to manually update to a specific version
* [This pretty cool project](https://barwap.com/projects/okmonitor/) uses a native video player and netcat to stream video to the kindle 
* [cross-compile python3 modules on desktop for kindle](https://www.mobileread.com/forums/showthread.php?t=343714), thats helpful
## Kiosk Setup Steps

### Out of Box Setup
1. turn it on
2. DO NOT CONNECT TO WIFI
   1. it will auto update to the latest software version, and then it gets a lot harder
   2. we will disable auto updates once we get a few things installed
3. go to settings and find the firmware version
   1. it MUST be between 5.0.0 and 5.4.4.2
      1. fun fact: 5.3.7.3 was the last version that supported the kindle touch/KT/K5, so you are very unlikely to have a firmware version that wont work!
   2. it is possible to downgrade some specific versions back to a jailbreakable version
   3. with serial port access, it is theoretically possible to downgrade to and from any version
   4. none of that is covered here, checked out the K5 Index Page linked above
4. while there, turn on airplane mode out of an abundance of caution


### Jailbreak

1. once you are good with firmware, in this repo find `PutOnKindle/` 
2. plug in your kindle via usb, and it should mount internal storage
3. put the contents of `PutOnKindle/` onto the kindle internal storage
4. eject and unplug it
5. do the "update dance":
   1. tap the menu icon
   2. tap "Settings"
   3. tap the menu icon again
   4. tap "Update Your Kindle"
6. it wont actually reboot and update, instead after a few seconds it will say `****  JAILBREAK  ****` at the bottom of the screen
7. go back into settings and restart the device anyway (or some things wont work right)

### Install KUAL (Kindle Unified Application Launcher)

KUAL is a nice launcher build as an interactive "book", so its easy to access. 
Configuration lives in the `extensions` folder on internal storage. 
We already dropped in a bunch of extensions, but we still need to install KUAL itself.

Plug in your kindle via usb and take a look at internal storage.
Notice that one of the files you put there named `Update_jb_$(cd mnt && cd us && sh jb.sh).bin` is gone now. 
It was processed when you ran "update your kindle", (and you can see how the jailbreak attack vector must work, which is neat), and then the update file got cleaned up. That's how the update mechanism works, and we will be using that to install two more packages.

Important note: never leave a `.bin` file just chilling in the root of internal kindle storage, it will break device boot up and you will have to recover via serial.
If you put a package there to install, dont forget to actually install it (which will then clean it up, and usually reboot).
Later on, we will switch to using a different package installation method, which avoids the risk of this issue.

KUAL comes as two separate packages: the app itself, and developer keys needed to enable running the app.

1. inside `ManualPackages`, find `Update_mkk-20141129-k5-ALL_install.bin` and put that on your kindle internal storage
2. eject, unplug, and do the update dance
3. repeat for `Update_KUALBooklet_v2.7_install.bin`
4. You will see KUAL "book" on your home screen

### Install MOAR PACKAGES!

TODO: python is only needed for screensavers hack

KUAL has an add-on called "Helper+" which includes an option to "Install MR Packages". 
This is the superior method to install packages:
* displays package icon and better progress visuals
* can install many packages at once
* there is no risk of accidentally leaving a `.bin` file around and bricking your device
* similar to the manual method, it cleans up package files after installing them

When you copied everything from `PutOnKindle/`, that included a folder `mrpackages`. 
All you need to do is put packages (`.bin` files) in that folder on the device, and then use the "Install MR Packages" button in KUAL, and it will install them.


I included four packages, in that folder now:
* Screen Savers hack - gives you more control over what the screensaver does
* python2.7
* python3
* USB Networking Hack

Some of these packages have corresponding KUAL extensions, they are already in the `extensions` folder.

To install those packages:
1. open KUAL app
2. tap "Helper+"
3. tap "Install MR Packages"
4. watch the progress, it will take several minutes for the python stuff
5. now that folder `mrpackages/` is empty

### Disable Updates

I know, you want to connect to WiFi. Resist a moment longer.

We need to disable auto updates, or your kindle will wait til you aren't looking and quietly update itself. This will break things.

There are two known methods to disable auto updates, and I recommend you do both to be sure, as they do not conflict.

In KUAL, under "Helper+", tap "PREVENT OTA Updates". 
This method works by creating a folder `/mnt/ub/update.bin.tmp.partial/`, which confuses the auto-updater script because it wants to store an update file at that location while downloading, but we put a folder there with the same name and it cannot handle that, so it fails.

The second method is also in KUAL, under "Back Door Lock". Just tap "Lock the Back Door". 
This method works by intercepting the process that installs the update after its downloaded, killing it. 
It also leaves the downloaded update file behind so that it wont be downloaded over and over.
An update should never get downloaded in the first place, because of the first method; this second method is just another layer of defense.

You can now connect to wifi without fear. 

There is a third method discovered recently, which involves removing/renaming the actual binaries that handle OTA updates. [You can read about it here](https://www.mobileread.com/forums/showthread.php?t=327879), but I dont recommend it.

### USB Networking

There is a section in KUAL for controlling USB Networking.

Pro-tip: when usb network is being enabled, it does some weird stuff on the USB bus that can crash the whole usb bus of your computer. 
Always unplug USB cable before enabling it.

When enabled, the kindle no longer shows up as USB storage device. Instead, it shows up as an RNDIS/Ethernet Gadget. 
For the K5, you want to set your machine's IP to `192.168.15.201`, and the kindle will be `192.168.15.244`. 

Since we ran the jailbreak, the root password is now blank, so you can ssh as root. 
You MUST change the password, or you will not be able to connect via WiFi. 
Also if you change to OpenSSH (option in KUAL, its faster than Dropbear), you will need a password set even for USB Networking.

The internal storage that gets mounted normally via USB connection is located at `/mnt/us`. 
If you want to use ssh keys for auth, `authorized_Keys` lives at `/mnt/us/usbnet/etc/authorized_keys`

My preference is to change password and install ssh key via USB Networking, then set it to only do ssh over wifi so that I can have USB Storage.
Also, since enabling usb networking causes aformentioned weirdness on the USB bus, it is not practical to have it enabled at boot for a device which is permanently connected to a host.

### Prevent Screensaver

TODO

### Alpine Linux

You need to be connected to wifi for this.

Welcome to the fun part! While using KUAL earlier, you probably noticed a menu item for "Alping Linux".
Under that, hit the to button labeled "Deploy newest release of Alpine Linux for...". 
It will tell you that you need KTerm, let it download and install it for you.
After that, it will open KTerm to run the Alpine install script.

It will take about 15 minutes; it prints progress the whole way through.

After Alpine Linux is installed (and most of your kindle storage is now used up), we need to make some tweaks to Alpine scripts to launch chromium instead of a full desktop.

A word of warning: at the time of this writing, the Alpine desktop seems to be sized wrong for the Kindle Touch, and there is no real way to get out of it without connecting via ssh (or serial) and rebooting. 

### Chromium

TODO

## Backup: The manual way
The examples below assume that you are able to connect to your Kindle via network (i.e., you must have installed the jailbreak, and enabled usb networking already), 
and that are using a Linux or MacOS computer (the "host") which has the appropriate commands installed, and is available via network at 192.168.15.201. Adjust according to your setup.

on the Host: `nc -l 31337|dd of=mmcblk0p1.bin`
on the Kindle: `dd if=/dev/mmcblk0p1|nc 192.168.15.201 31337`

Note: depending on your version of netcat, this may not work. 
If you have an old version of netcat, try `nc -l -p 31337|dd of=mmcblk0p1.bin` instead.


Repeat this procedure for mmcblk0p2 through mmcblk0p4 accordingly. 
The files that you get are binary copies of the individual partitions of the device. 
A short explanation of each partition can be found below.

Partitions

* Partition 1 (mmcblk0p1 / ext3, 350 MB): The root file system. This contains the operating system and the files of the framework. If this partition is damaged, your device will not work properly.
* Partition 2 (ext3, 64 MB): This is the emergency recovery system (diagnostics system). You normally won't even get to seeing or modifying this partition. Under all normal circumstances, keep your fingers away from this.
* Partition 3 (ext3, 32 MB (KT)/ 64MB (PW)): This partition (mounted to /var/local/ ) contains local settings. Most probably, the contents can be deleted [1]. You will lose your settings, but the device will still work.
* Partition 4 (FAT32, 3.3 GB (KT and some PW2) / 1.4 GB(PW)): This is where your documents go. In normal operations, it is mounted as /mnt/us, and this is the partition you get to modify when you mount the Kindle via USB. You can delete the contents if you can afford to lose your documents; the device will still work. [Do not delete /diagnostic_logs/device_info.xml from a USB Drive exported from the diagnostic recovery menu screen, or it will be difficult to reboot to the main partition.]

A backup of partitions 3 and 4 is thus not necessarily needed, if you can afford to lose personal settings. In contrast, backups of partitions 1 and 2 are highly recommended.
On Linux, you can mount partition images using mount -o loop,ro <image> <mountpoint>. Since partition 4 is actually a disk image which contains a single partition itself, the mount command for that partition is mount -o loop,ro,offset=8192 mmcblk0p4.bin <mountpoint>. From there, you can access the original files, to restore them on the device if some modification went wrong.

### Removing the Battery

You should definitely NOT just mount the kindle on the wall somewhere, permanently charging. 
The battery chemistry will break down over time (1-2 years) and eventually swell, burst, erupt flames, etc. 
This is true of anything with a lithium-ion battery; they are not meant to be charging 24/7.

The kindle battery has a little management board, usually just taped to the top of the cell.
When the linux kernel boots it checks the battery status, and if the battery level is too low, too high, too hot, or missing, or if the battery manager does not respond, it will
shut down. This is a great safety feature, but it gets in our way if we want to use it without a battery. 

If you remove the battery and just provide 4.2v (simulating a fully charged li-ion battery), the Kindle will not boot. 
This is because the kernel checks with the battery, looking for a valid ID and voltage.
If you connect a serial cable you can see this happen, and interrupt the boot process to skip this check. Right now, this is actually how I
"solved" the issue, because I very rarely need to reboot my wall-mounted Kindles. 

However, there exist two actual solutions to running without a battery.

#### Emulate the battery manager
You can fool the kernel by emulating the battery monitor with a PIC microcontroller [like this guy (Niel) did](http://bloodsweatandsolder.blogspot.com/2016/04/booting-kindle-dx-graphite-without.html).
If you look inside [PIC_KindleBattery.X/](PIC_KindleBattery.X) I have modified version of Niel's work, that I cleaned up and commented.
One important thing to note, compared to Niel's work, is that the battery does NOT actually use the SMBus protocol; it uses normal I2C.
I couple addresses line up with the SMBus standard, but if you look at the driver [here](https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/drivers/power/yoshi_battery.c),
you can see that the majority of the addresses do not line up with SMBus spec.

I have one Kindle that is running using this solution, and it works flawlessly. One downside is that you need to power the PIC from 3.3v or I2C will not 
work correctly, so you need to provide the kindle with two different voltages. I put a pair of cheap adjustable DC-DC regulators on the back of my Kindle, so that I can just
feed it 5V and let the regulators produce the two needed voltages. You can feed the kindle 5V instead of 4.2V, and it will operate just fine,
however if you monitor dmesg you will see that the yoshi driver warns about battery being over-voltage, and so I chose to operate at 4.2V to eliminate that warning.

If you look in [Arduino_KindleBattery/](Arduino_KindleBattery) you will see that I attempted to port this emulation to Arduino, hoping to make it more accessible to the average hacker.
Unfortunately, there appears to be a major issue with the Wire library that prevents it from working. From my understanding, even though the protocol is definitely not
SMBus, when the cpu starts the I2C conversation, it issues a duplicate start symbol; this is normal for SMBus but not I2C. The library used in the PIC implementation 
tolerates this duplicate start symbol and behaves as we would expect, but the Wire library for arduino does not appear to tolerate this deviation. 
I welcome any insight and solutions that others can provide.

#### Recompile the kernel
Another, arguably "neater" solution is to modify the kernel. 
There are some forum posts out there that claim that the battery check can be disabled by tweaking an init script.
This might have worked on super early generation Kindles (non-touch), but it definitely will not work for Kindle Touch, nor the K4 before it.

The battery driver is included directly in the kernel, and the battery safety checks are done very early in kernel execution, long before userland is involved.

* [This is how to compile the kernel](https://www.mobileread.com/forums/showthread.php?t=91862)
* [This is the file to edit](https://github.com/fread-ink/kernel-k4-usb-otg/blob/master/drivers/power/yoshi_battery.c)

I have bypassing battery check this way, and it works, but I had to go through a lot of effort to tweak the kernel sources to build correctly on a modern compiler.
I did not document this work well enough to publish, and I also am not confident that my changes did not have additional undesired effect. I will continue
working on this, and may publish modified kernel sources at a later date if I can make it reliable.

