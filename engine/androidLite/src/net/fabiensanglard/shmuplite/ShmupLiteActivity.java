package net.fabiensanglard.shmuplite;

import java.io.IOException;

import android.app.NativeActivity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;


public class ShmupLiteActivity extends NativeActivity {

	//Seems this is not needed when working 
   
    static {
		//System.loadLibrary("openal");
		//Shmup .so will automatically be loaded but we need to manually load the openAL dependency
        //System.loadLibrary("shmup");
    }

  
 
	@Override
	protected void onCreate(Bundle savedInstanceState) {

		System.out.println("Pre native code call.");   

		super.onCreate(savedInstanceState);  

		System.out.println("Post native code call."); 
	}


}