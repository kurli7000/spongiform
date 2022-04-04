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
ant release install
if [ $? -ne 0 ] ; then
  exit 1
fi

cp ./bin/DEMOActivity-release.apk ./spongiform.apk


# start 
adb shell am start -a android.intent.action.MAIN -n com.kewlers.spongiform/.DEMOActivity
