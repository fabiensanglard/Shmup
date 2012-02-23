package net.fabiensanglard.shmuplite;

import java.io.IOException;


import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;


public class ShmupLiteActivity extends NativeActivity {
 
	private static Context context;
	private static Handler handler;
	
	public static void goToWebsite(final String url){
		handler.postAtFrontOfQueue(new Runnable(){

			public void run() {
				Intent intent = new Intent(Intent.ACTION_VIEW);
				intent.setData(Uri.parse(url));
				intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
				ShmupLiteActivity.context.startActivity(intent);
				
			}});
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {

		System.out.println("Pre native code call.");
		context = getApplicationContext();
		handler = new Handler();
		super.onCreate(savedInstanceState);  

		System.out.println("Post native code call.");
	}

}