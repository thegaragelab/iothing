package org.sensaura.iothing.model;

/** Represents a unconfigured IoThing
 */
public class IoThingDevice implements DynamicObject {
  //--- Instance variables
  private String m_id;

  /** Constructor
   *
   * @param id
   */
  public IoThingDevice(String id) {
    m_id = id;
    }

  @Override
  public String getID() {
    return m_id;
    }

  }
