package net.fabiensanglard;

import java.io.IOException;

import android.app.NativeActivity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;


public class Launcher extends NativeActivity {
	
/*
    static {
		//System.loadLibrary("openal");
        System.loadLibrary("shmup");
    }
	*/
	
	void displayFiles (AssetManager mgr, String path) {
		
		Log.v("Assets:'"+path+"'","");
	    try {
	        String list[] = mgr.list(path);
	        if (list != null)
	            for (int i=0; i<list.length; ++i)
	                {
	                    Log.v("Assets:'"+path+"'", list[i]);
	                    //displayFiles(mgr, path + list[i]);
	                }
	    } catch (IOException e) {
	        Log.v("List error:", "can't list" + path);
	    }

	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
		displayFiles(getAssets(),"assets/data");
		displayFiles(getAssets(),"assets/data/menu");
		displayFiles(getAssets(),"assets/data/");
		displayFiles(getAssets(),"assets/data/menu/");
		displayFiles(getAssets(),"/");
		displayFiles(getAssets(),"/assets");
		displayFiles(getAssets(),"");
		displayFiles(getAssets(),"data");
		displayFiles(getAssets(),"data/");
		displayFiles(getAssets(),"assets");
		displayFiles(getAssets(),"scenes");
		displayFiles(getAssets(),"cameraPath");
		
		System.out.println("Pre native code call.");
		
		super.onCreate(savedInstanceState);  
		
		System.out.println("Post native code call.");
	}

	
}
