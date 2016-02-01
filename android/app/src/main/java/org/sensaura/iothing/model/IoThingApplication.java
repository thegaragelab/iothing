package org.sensaura.iothing.model;

import android.app.Application;

import java.util.*;

/** Custom application class
 *
 * Use this to provide the model.
 */
public class IoThingApplication extends Application {
    private static IoThingApplication m_instance;

    public static final IoThingApplication getInstance() {
        return m_instance;
    }

    public void onCreate(){
        super.onCreate();
        this.m_instance = this;
    }

    /**
     * An array of sample (dummy) items.
     */
    public static final List<IoThing> Things = new ArrayList<>();
    public static final Map<String, IoThing> ThingMap = new HashMap<>();
    public static final List<IoThingDevice> Devices = new ArrayList<>();
    public static final Map<String, IoThingDevice> DeviceMap = new HashMap<>();
  }
