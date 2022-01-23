# Kindle Touch (K5, D01200) Chromium Touch Kiosk

The goal of this project is to use a Kindle Touch as an interactive kiosk for Home Assistant Lovelace pages. To achieve this,
we jailbreak the Kindle and put a VNC client on it. We run a VNC server in a docker container, with Chromium browser full-screen,
pointed at whatever URL is desired. It is not perfect, you will see some artifacts typical of e-ink screens, and the framerate is
not stellar, but it is totally usable as interactive kiosk.

This is the short version [Here is the long version](MoreInformation.md), which has a LOT more information

## Acknowledgements
This repository is a repackaging of [other people's work](https://wiki.mobileread.com/wiki/K5_Index). 
Credit to NiLuJe, hwhw, schuhumi, and all the other fine people on MobileRead forums who made all this work, and shared it with us.

## Setup Steps

1. We presume you are using the Kindle Touch, model D01200
2. connect to wifi
3. check firmware version, if it is lower than 5.3.7.3, go ahead and update
   1. that is the last version that supports this hardware, and it can be jailbroken easily
4. plug in usb, mount internal kindle storage
5. in this repo, find `PutOnKindle.zip` and extract its contents into the root of kindle storage
   1. you do NOT want a folder named `PutOnKindle`, take everything inside that folder and put at root of kindle storage
   2. why is this in a zip file? Need to preserve extended attributes which git does not track
6. eject and unplug kindle
7. on kindle, go to settings and update
   1. this will just print a jailbreak message at the bottom 
8. restart the kindle
9. plug in usb, mount internal kindle storage
10. inside `ManualPackages/`, grab `Update_KUALBooklet_v2.7_install.bin`, and put it on the root of kindle internal storage
11. eject and unplug kindle
12. go update software again, this will actually install the package you just put on internal storage (and then delete the package when done)
13. you now have a new "book" on the kindle home screen named KUAL
14. open KUAL, tap Helper+, then Install MR Packages
15. watch it install two packages (jailbreak hotfix, and usbnetwork)
16. you now have usb networking!
17. make sure your kindle is NOT conneted via usb
18. open KUAL, tap USBNetwork
    1. Tap Enable Verbose Mode
       1. this will cause it to print status messages at bottom of the screen, super helpful for debugging
    2. Tap Toggle USBNetwork
    3. when usb networking is toggled, it does some funky stuff on the usb bus that almost always crashes whatever device it is plugged into. This is why you should not toggle usb network while usb connected
19. connect usb to computer (ideally a mac)
20. configure the new RNDIS/Ethernet Gadget device that should be listed in network settings on your desktop
    1. set ip to `192.168.15.201`
21. ssh to your kindle: `ssh root@192.168.15.244`
    1. the jailbreak will have cleared the root password, so you can login with blank password
       1. blank password only works via usbnet, using the default DropBear server. you MUST set root password for ssh to work via wifi
    2. change root password
       1. switch to read/write mode: `mntroot rw`
       2. change password: `passwd`
       3. switch back to readonly mode: `mntroot ro`
       4. readonly mode only affects the system partition, not userdata (`/mnt/us/`)
    3. on the kindle, the internal storage is at `/mnt/us` and you whould put your ssh key in `/mnt/us/usbnet/etc/authorized_keys`
22. once you have changed password and installed your key(s), exit the ssh session and unplug usb
23. open KUAL and toggle usb network
24. Now lets enable ssh via wifi only, at boot
    1. first, toggle usbnet (you need it off to change these settings)
    2. tap Enable SSH at boot
    3. tap Restrict SSH to WiFi, stay in USBMS
       1. now when you connect usb it will always do the storage thing, but ssh works via wifi
    4. tap SSHD: Use OpenSSH
       1. OpenSSH is a bit faster than the default DropBear server on the kindle
25. reboot the kindle and confirm that you can ssh to it via wifi, and that usb works for storage

## VNC Client Auto Start

1. ssh to the kindle (via wifi)
2. `cd /mnt/us/`
3. `./setup_kiosk_autostart.sh`
4. `nano vnc_config.txt`
   1. change the ip, port, and password
5. `reboot; exit`
   1. it helps to include `; exit` because sometimes you dont get kicked out of the ssh session properly and you just end up with a frozen terminal
6. at next boot, it will start the vnc client
7. logs are at `/mnt/us/kiosk.log`

From now on, when the kinde boots up it will launch vnc client instead of the normal GUI. 
If the vnc client crashes, or fails to connect, it will restart with a 10 second delay.
During the delay it will display battery information. 
You can manually force it to quit (and be restarted) by pressing the home button. 

to undo this, there is another script named `undo_kiosk_autostart.sh`, after which you should reboot and it will launch the normal kindle GUI

## VNC Server Docker Container

Inside `docker-vnc-kiosk-server` are the pieces for a docker container running chromium in kiosk mode, pointed at your url, and a vnc server.
Edit `docker-compose.yml` and make sure to adjust the password and url.

This container will persist the chromium profile, so you can save cookies and credentials.
