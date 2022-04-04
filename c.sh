#
# if device not showing, test these:
# adb devices list
# adb kill-server && adb start-server
#
clear
ndk-build
if [ $? -ne 0 ] ; then
  exit 1
fi
android update project -t android-14 -p .
if [ $? -ne 0 ] ; then
  exit 1
fi
ant debug install
if [ $? -ne 0 ] ; then
  exit 1
fi

# push all shaders to sdcard
echo "Pushing shader sources to /sdcard/tmp"
find ./assets/shaders -type f -exec adb push '{}' /sdcard/tmp \;

# clear log
#adb logcat -c

# start 
adb shell am start -a android.intent.action.MAIN -n com.kewlers.spongiform/.DEMOActivity
#adb logcat -s "KEWLERS"
#adb logcat

# asset sync
while true; do 
find ./assets/shaders -type f -ctime -3s -exec adb push '{}' /sdcard/tmp \;
sleep 2
done
