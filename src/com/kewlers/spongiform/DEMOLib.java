/*
  Wrapper for native library
*/

package com.kewlers.spongiform;

import android.content.res.AssetManager;

public class DEMOLib {

     static {
         System.loadLibrary("bass");
         System.loadLibrary("demo");
     }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void resizegl(int width, int height);
     public static native boolean initgl(AssetManager assetManager);      
     public static native boolean step();
     public static native void stop();
     public static native void pause(boolean pause);
     public static native void touchevent();
     public static native String getmessage();
}
