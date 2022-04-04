/*
  activity class, handles events and shit(?) 
*/

package com.kewlers.spongiform;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.os.PowerManager;
import android.content.Context;
import java.io.File;
import java.nio.ByteBuffer;
import android.content.res.AssetManager;
import android.view.View;
import android.app.Dialog;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;


public class DEMOActivity extends Activity {

    DEMOView mView;
    private PowerManager.WakeLock wl;
    static AssetManager assetManager;

    @Override protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        wl = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK, "DoNjfdhotDimScreen");

        assetManager = getAssets();
        mView = new DEMOView(getApplication(), this, assetManager);
		setContentView(mView);	

		int SDK_INT = android.os.Build.VERSION.SDK_INT;
		/*
        if (SDK_INT >= 11 && SDK_INT < 14) {
			mView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
		} else if (SDK_INT >= 14) {
			mView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_HIDE_NAVIGATION);
		}*/		
        if (SDK_INT >= 11) {
            mView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE);
        }
    }
    
    @Override protected void onPause() {
        super.onPause();
        mView.onPause();
        wl.release();
		mView.pause(true);
    }

    @Override protected void onResume() {
        super.onResume();
        mView.onResume();
        wl.acquire();
		mView.pause(false);
    }
    
    @Override protected void onDestroy() {
		if (wl.isHeld()) wl.release();
		mView.stop();
        super.onDestroy();
    }

	void movetoback() {
		moveTaskToBack(true);
	}
    
	@Override protected Dialog onCreateDialog(int id) {
    	switch(id) {
			case 0: {               
				return new AlertDialog.Builder(this)
		    		.setTitle("ANAL2.0b")
					.setMessage(mView.getmessage())
        			.setPositiveButton("OK", new DialogInterface.OnClickListener() {                   
            			@Override
            			public void onClick(DialogInterface arg0, int arg1) {
            				DEMOActivity.this.finish();
            		}
        		}).create();
    		}
		}
    	return null;
	}
	
    
}
