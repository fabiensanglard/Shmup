package net.fabiensanglard.shmup;

import java.io.IOException;

import android.app.NativeActivity;
import android.content.Context;
import android.content.Intent;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;


public class Launcher extends NativeActivity {

	public void goToWebsite(final String url){
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.setData(Uri.parse(url));
		intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		getApplicationContext().startActivity(intent);
	}
}

