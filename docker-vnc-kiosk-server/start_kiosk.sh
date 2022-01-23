#!/bin/bash
# chromium kiosk via vnc
set -x

# handle defaults
URL="${URL:-http://wikipedia.com}"
WINDOW_WIDTH="${WINDOW_WIDTH:-800}"
WINDOW_HEIGHT="${WINDOW_HEIGHT:-600}"
SCALE="${SCALE:-1.0}"
VNC_PASSWORD="${VNC_PASSWORD:-badpass}"
EXTRA_CHROMIUM_ARGS="${EXTRA_CHROMIUM_ARGS:-}"
EXTRA_VNC_ARGS="${EXTRA_VNC_ARGS:-}"
EXTRA_X_ARGS="${EXTRA_X_ARGS:-}"

# TODO add extra args for xvfb and x11vnc (nocursor)

## Hardcoded Vars
VIRTUAL_VRAM="192000"  # need more vram for higher resolutions
REFRESH_RATE="60"  # refreshrate hardcoded to 60 for virtual screen
DESKTOP_USER="chromium"  # avoid running chromium as root. if you do run as root, you need to use flag --no-sandbox
USER_DATA_DIR="/userdata"
WINDOW_POSITION="0,0"  # always want window to start at 0,0
DISK_CACHE_DIR="/dev/null"  # prevent chromium from caching anything
VIRTUAL_DISPLAY=":99"  # Xvfb will create a virtual display with this identifier and that will be used for everything else

## Generated Vars
WINDOW_SIZE_VNC="${WINDOW_WIDTH}x${WINDOW_HEIGHT}"  # x11vnc wants separated by x
WINDOW_SIZE_CHROMIUM="${WINDOW_WIDTH},${WINDOW_HEIGHT}"  # chromium wants separated by ,

#   TODO if we do this we should also do the xdotool thing like firefox_wrapper does, to cleanly stop chromium
# handler for killing things off when container is stopped
_kill_procs() {
  echo "signal caught, cleaning up processes"
  kill -TERM "$PID_CHROMIUM"
  wait "$PID_CHROMIUM"
  kill -TERM "$PID_UNCLUTTER"
  wait "$PID_UNCLUTTER"
  kill -TERM "$PID_VNC"
  wait "$PID_VNC"
  kill -TERM "$PID_WM"
  wait "$PID_WM"
#  kill -TERM "$PID_XVFB"
  kill -TERM "$PID_X"
  echo "done with cleanup"
  exit 0
}

# trap signals and call our handler
trap _kill_procs SIGTERM SIGINT

############## Actual startup begins here ##################

# X and Chromium both want dbus
echo ""
echo "starting dbus"
eval "$(dbus-launch --sh-syntax)"

echo "giving dbus a moment to settle"
sleep 2

# write our own xorg conf
NEW_MODENAME="${WINDOW_SIZE_VNC}"
echo "rendering /xorg-dummy.conf for resolution: $NEW_MODENAME"
NEW_MODELINE=$(cvt "$WINDOW_WIDTH" "$WINDOW_HEIGHT" "$REFRESH_RATE" | tail -n1 | awk '{print $3 " " $4 " " $5 " " $6 " " $7 " " $8 " " $9 " " $10 " " $11 " " $12 " " $13}' )

cat << EOF_XORG > /xorg-dummy.conf
Section "Device"
  Identifier "dummy_videocard"
  Driver "dummy"
  Option "ConstantDPI" "true"
  VideoRam $VIRTUAL_VRAM
EndSection

Section "Monitor"
  Identifier "dummy_monitor"
  HorizSync   5.0 - 1000.0
  VertRefresh 5.0 - 200.0
  Modeline "$NEW_MODENAME" $NEW_MODELINE
EndSection

Section "Screen"
  Identifier "dummy_screen"
  Device "dummy_videocard"
  Monitor "dummy_monitor"
  DefaultDepth 24
  SubSection "Display"
    Viewport 0 0
    Depth 24
    Modes "$NEW_MODENAME"
    Virtual $WINDOW_WIDTH $WINDOW_HEIGHT
  EndSubSection
EndSection

EOF_XORG


# now lets build our virtual framebuffer
echo "starting X"
DISPLAY=$VIRTUAL_DISPLAY
X $DISPLAY $EXTRA_X_ARGS -config /xorg-dummy.conf & PID_X=$!
export DISPLAY

echo "giving X a moment to settle"
sleep 2

# TODO this could go in the xorg.conf
echo ""
echo "making sure X doesnt try to sleep or blank the screen"
xset s off
xset s noblank

echo ""
echo "starting vnc server with size: $WINDOW_SIZE_VNC"
/usr/bin/x11vnc -safer -passwd "$VNC_PASSWORD" -forever -quiet -scale 1 -display "$DISPLAY" -notruecolor -shared -geometry "$WINDOW_SIZE_VNC" $EXTRA_VNC_ARGS & PID_VNC=$!

echo ""
echo "starting matchbox window manager"
/usr/bin/matchbox-window-manager -use_titlebar no -use_cursor no & PID_WM=$!

echo ""
echo "letting everything settle for a couple seconds"
sleep 4

echo ""
echo "starting chromium with url: $URL"

# make sure desktop user can write to user data
chown "$DESKTOP_USER" "$USER_DATA_DIR"

# this is the stupid lockfile chromium leaves behind because it doesnt get shut down properly
rm ${USER_DATA_DIR}/SingletonLock

CHROMIUM_CMD="export DISPLAY=$DISPLAY; chromium \
  --no-gpu \
  --display=$DISPLAY \
	--force-device-scale-factor=$SCALE \
	--window-size=$WINDOW_SIZE_CHROMIUM \
	--window-position=$WINDOW_POSITION \
	--pull-to-refresh=1 \
	--disable-smooth-scrolling \
	--disable-login-animations \
	--disable-modal-animations \
	--noerrdialogs \
	--no-first-run \
	--disable-infobars \
	--fast \
	--fast-start \
	--disable-pinch \
	--overscroll-history-navigation=0 \
	--disable-translate \
	--disable-overlay-scrollbar \
	--disable-features=OverlayScrollbar \
	--disable-features=TranslateUI \
	--disk-cache-dir=$DISK_CACHE_DIR \
	--password-store=basic \
	--touch-events=enabled \
	--ignore-certificate-errors \
	--user-data-dir=$USER_DATA_DIR \
	--kiosk $EXTRA_CHROMIUM_ARGS \
	--app=$URL"

echo ""
echo "running chromium command: $CHROMIUM_CMD"
echo ""
su -c "$CHROMIUM_CMD" "$DESKTOP_USER" & PID_CHROMIUM=$!

echo ""
echo "######## Chromium has been started  #########"
echo ""

wait $PID_CHROMIUM
wait $PID_X
