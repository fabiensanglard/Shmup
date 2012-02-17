package net.fabiensanglard.shmuplite;

import java.io.IOException;

import android.app.NativeActivity;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;


public class ShmupLiteActivity extends NativeActivity {
 
	@Override
	protected void onCreate(Bundle savedInstanceState) {

		System.out.println("Pre native code call.");   

		super.onCreate(savedInstanceState);  

		System.out.println("Post native code call."); 
	}
	
	public void goToWebsite(String url){
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse(url));
		startActivity(intent);
	}

}