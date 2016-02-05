package org.sensaura.iothing.model;

/** Represents a single IoThing
 *
 * This is the base class for all IoThings. It supports both configured and
 * unconfigured devices (unconfigured devices are not connected to a WiFi
 * network yet).
 */
public class IoThing implements DynamicObject {
  //--- Instance variables
  public String  m_id;         //!< The unique ID of the thing.
  public String  m_name;       //!< The name of the thing
  public boolean m_configured; //!< True if this device is configured.

  public IoThing(boolean configured, String id, String name) {
    m_configured = configured;
    m_id = id;
    m_name = name;
    }

  @Override
  public String toString() {
    return m_name;
    }

  /** Get the unique ID of the thing
   *
   * @return the string representing the things unique ID.
   */
  @Override
  public String getID() {
    return m_id;
    }

  /** Determine if the device is configured
   *
   * @return true if the device has been configured.
   */
  public boolean isConfigured() { return m_configured; }

  /** Get the name of the thing
   *
   * @return the string representing the displayed name of the thing.
   */
  public String getName() { return m_name; }

  }
