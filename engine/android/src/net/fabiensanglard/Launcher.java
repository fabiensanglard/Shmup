package net.fabiensanglard;

import android.app.NativeActivity;
import android.os.Bundle;

public class Launcher extends NativeActivity {
/*
    static {
		//System.loadLibrary("openal");
        System.loadLibrary("shmup");
    }
	*/
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
		System.out.println("Pre native code call.");
		
		super.onCreate(savedInstanceState);  
		
		System.out.println("Post native code call.");
	}

	
}
