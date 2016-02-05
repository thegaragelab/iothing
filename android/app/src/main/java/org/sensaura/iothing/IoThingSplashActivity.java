package org.sensaura.iothing;

import android.content.*;
import android.net.wifi.*;
import android.support.v7.app.AppCompatActivity;
import android.os.*;
import android.net.*;
import android.util.*;

import org.sensaura.iothing.model.*;

import java.util.*;

public class IoThingSplashActivity extends AppCompatActivity implements Observer{
  private static final String TAG = "IoThingSplashActivity";

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_io_thing_splash);
    // See if WiFi is available
    Log.d(TAG, "Checking current WiFi state.");
    if (IoThingApplication.Network.isConnected())
      launchMainActivity(1000);
    }

  @Override
  protected void onResume() {
    super.onResume();
    Log.d(TAG, "OnResume()");
    // TODO: Monitor network connection changes
    IoThingApplication.Network.addObserver(this);
    }

  @Override
  protected void onPause() {
    super.onPause();
    Log.d(TAG, "OnPause()");
    // TODO: Stop monitoring network connection changes
    IoThingApplication.Network.deleteObserver(this);
    }

  @Override
  public void update(Observable observable, Object data) {
    Log.d(TAG, "Network connection state changed.");
    if (IoThingApplication.Network.isConnected())
      launchMainActivity(0);
    }

  /** Launch the main activity (after a delay)
   *
   * @param delay the number of milliseconds to delay before launching.
   */
  private void launchMainActivity(long delay) {
    Log.d(TAG, "WiFi available, launching main activity.");
    final Handler handler = new Handler();
    final Context ctx = this;
    handler.postDelayed(new Runnable() {
      @Override
      public void run() {
        Intent intent = new Intent(ctx, IoThingListActivity.class);
        startActivity(intent);
       }
      },
      delay);
    }

  }
