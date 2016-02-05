package org.sensaura.iothing.model;

/** Represents a unconfigured IoThing
 */
public class IoThingDevice extends IoThing {
  //--- Instance variables
  private String m_id;

  /** Constructor
   *
   * @param id
   */
  public IoThingDevice(String id, String content, String details) {
    super(id, content, details);
    }

  @Override
  public String getID() {
    return m_id;
    }

  }
