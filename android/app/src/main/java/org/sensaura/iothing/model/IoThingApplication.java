package org.sensaura.iothing.model;

//--- Imports
import android.app.Application;
import android.net.nsd.*;
import android.content.Context;
import android.util.Log;

import java.util.*;

/** Custom application class
 *
 * Use this to provide the model.
 */
public class IoThingApplication extends Application {
  private static final String IOTHING_SERVICE = "_iothing._tcp";

  /* Provide a static method of acquiring the application instance
   */
  private static IoThingApplication m_instance;
  public static final IoThingApplication getInstance() {
    return m_instance;
    }

  //-------------------------------------------------------------------------
  // Service listener helper class
  //-------------------------------------------------------------------------

  private class ServiceListener implements NsdManager.DiscoveryListener {
    static private final String TAG = "ServiceListener";

    //--- Instance variables
    private boolean                    m_discovering; //!< True if we are in discovery mode
    private NsdManager.ResolveListener m_resolver;    //!< Handler service resolution

    /** Constructor
     *
     */
    public ServiceListener() {
      m_discovering = false;
      m_resolver = new NsdManager.ResolveListener() {
        @Override
        public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
          // Called when the resolve fails.  Use the error code to debug.
          Log.e(TAG, "Resolve failed" + errorCode);
          }

        @Override
        public void onServiceResolved(NsdServiceInfo serviceInfo) {
          Log.e(TAG, "Resolve Succeeded. " + serviceInfo);
          }
        };
      }

    /** Invoked when discovery is started
     *
     * @param serviceType
     */
    @Override
    public void onDiscoveryStarted(String serviceType) {
      Log.d(TAG, "Starting service discovery.");
      m_discovering = true;
      }

    /** Invoked when discovery is stopped
     *
     * @param serviceType
     */
    @Override
    public void onDiscoveryStopped(String serviceType) {
      Log.d(TAG, "Stopping service discovery");
      m_discovering = false;
      }

    /** Invoked when a service has been found
     *
     * @param serviceInfo
     */
    @Override
    public void onServiceFound(NsdServiceInfo serviceInfo) {
      Log.d(TAG, "Service discovered");
      NsdManager nsdManager = (NsdManager)IoThingApplication.getInstance().getSystemService(Context.NSD_SERVICE);
      nsdManager.resolveService(serviceInfo, m_resolver);
      }

    /** Invoked when a service is no longer available
     *
     * @param serviceInfo
     */
    @Override
    public void onServiceLost(NsdServiceInfo serviceInfo) {
      // TODO: Remove service
      }

    @Override
    public void onStartDiscoveryFailed(String serviceType, int errorCode) {
      Log.e(TAG, "Failed to start discovery");
      m_discovering = false;
      }


    @Override
    public void onStopDiscoveryFailed(String serviceType, int errorCode) {
      Log.e(TAG, "Failed to stop discovery");
      m_discovering = false;
      }

    //-----------------------------------------------------------------------
    // Custom methods
    //-----------------------------------------------------------------------

    /** Determine if we are in discovery mode
     *
     * @return true if discovery is active.
     */
    public boolean isDiscovering() {
      return m_discovering;
      }
  }

  //--- Instance variables
  private ServiceListener m_serviceListener;

  //-------------------------------------------------------------------------
  // Public access to devices for UI binding
  //-------------------------------------------------------------------------

  /** The collection of available IoThing instances
   */
  public static final DynamicCollection<IoThing> Things = new DynamicCollection<>();

  /** The collection of available IoThingDevice instances
   */
  public static final DynamicCollection<IoThingDevice> Devices = new DynamicCollection<>();

  //-------------------------------------------------------------------------
  // Application life cycle management
  //-------------------------------------------------------------------------

  /** Application creation
   *
   * This simply stores a reference to the singleton.
   */
  public void onCreate() {
    super.onCreate();
    this.m_instance = this;
    }

  //-------------------------------------------------------------------------
  // Custom operations
  //-------------------------------------------------------------------------

  /** Enable (or disable) discovery mode for things
   *
   * @param enabled
   *
   * @return true on success, false on failure
   */
  public boolean discoverThings(boolean enabled) {
    // Make sure we have a service listener available
    synchronized (this) {
      if (m_serviceListener == null)
        m_serviceListener = new ServiceListener();
      }
    // Get the discovery manager
    NsdManager nsdManager = (NsdManager)getSystemService(Context.NSD_SERVICE);
    if (nsdManager == null)
      return false;
    // Change the discovery state
    if (enabled && !m_serviceListener.isDiscovering()) {
      // Start the discovery process
      nsdManager.discoverServices(IOTHING_SERVICE, NsdManager.PROTOCOL_DNS_SD, m_serviceListener);
      }
    else if (m_serviceListener.isDiscovering() && !enabled) {
      // Stop the discovery process
      nsdManager.stopServiceDiscovery(m_serviceListener);
      }
    return true;
    }

  }
