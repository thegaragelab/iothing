package org.sensaura.iothing.model;

import java.util.*;

import android.content.*;
import android.net.*;
import android.util.*;

/** Monitor the WiFi connection state.
 */
public class WiFiConnection extends Observable {
  private static final String TAG = "IoThingWiFiConnection";

  // WiFi Connection state and information
  private boolean m_connected;
  private String  m_ssid;

  WiFiConnection() {
    // Get our current connection state
    m_connected = false;
    m_ssid = null;
    }

  /** Update the network state
   *
   */
  void updateState() {
    Log.d(TAG, "State may have changed, checking values.");
    ConnectivityManager cm = (ConnectivityManager)IoThingApplication.getInstance().getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo info = cm.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
    boolean connected = (info != null) && info.isConnected();
    if (connected != m_connected) {
      Log.d(TAG, String.format("Network connection state changed %s -> %s", m_connected, connected));
      // State has changed
      m_connected = connected;
      m_ssid = (connected) ? "WIFI" : null;
      setChanged();
      notifyObservers(this);
      }
    }

  /** Get the current connection state
   *
   * @return true if WiFi is currently connected
   */
  public boolean isConnected() {
    return m_connected;
    }

  /** Get the connected SSID
   *
   * @return the SSID of the currently connected network or null if no connection
   *         is active.
   */
  public String SSID() {
    return m_ssid;
    }

  }
