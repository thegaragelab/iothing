package org.sensaura.iothing.model;

/**
 */
public class IoThing {
    public final String id;
    public final String content;
    public final String details;

    public IoThing(String id, String content, String details) {
        this.id = id;
        this.content = content;
        this.details = details;
    }

    @Override
    public String toString() {
        return content;
    }
}
