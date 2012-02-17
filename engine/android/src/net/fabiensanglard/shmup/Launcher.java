package net.fabiensanglard.shmup;

import java.io.IOException;

import android.app.NativeActivity;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;


public class Launcher extends NativeActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		System.out.println("Pre native code call.");

		super.onCreate(savedInstanceState);  

		System.out.println("Post native code call.");
	}


}

