package org.sensaura.iothing.model;

//--- Imports
import java.util.*;

/** Provides a way to manage a dynamic collection of objects
 */
public class DynamicCollection<T extends DynamicObject> extends Observable {
  // The list of items (publicly available for binding)
  public final List<T> Items = new ArrayList<>();

  // Map of item IDs to instances
  private final Map<String, T> m_itemMap = new HashMap<>();

  /** Add a new item to the collection
   *
   * This adds (or replaces) an item in the list and notifies any observers
   * that might be attached.
   *
   * @param item the item to add or update
   */
  public void add(T item) {
    if (item == null)
      return;
    synchronized (this) {
      // Does the item already exist?
      if (m_itemMap.containsKey(item.getID())) {
        T existing = m_itemMap.get(item.getID());
        m_itemMap.remove(item.getID());
        Items.remove(existing);
        }
      // Add the item
      m_itemMap.put(item.getID(), item);
      Items.add(item);
      setChanged();
      }
    notifyObservers(this);
    }

  /** Remove an item from the collection
   *
   * Removes an existing item from the list and notifies any observers that
   * might be listening.
   *
   * @param id the ID of the item to remove
   */
  public void remove(String id) {
    if (id==null)
      return;
    synchronized (this) {
      // Does the item exist?
      if (!m_itemMap.containsKey(id))
        return;
      T existing = m_itemMap.get(id);
      m_itemMap.remove(id);
      Items.remove(existing);
      setChanged();
      }
    notifyObservers(this);
    }

  /** Remove an item from the collection
   *
   * Removes an existing item from the list and notifies any observers that
   * might be listening.
   *
   * @param item the item to remove
   */
  public void remove(T item) {
    if (item == null)
      return;
    remove(item.getID());
    }

  /** Fetch an item by it's ID
   *
   * @param id the unique ID of the item wanted
   *
   * @return the item or null if it is not present.
   */
  public T get(String id) {
    if (id==null)
      return null;
    synchronized (this) {
      // Does the item exist?
      if (!m_itemMap.containsKey(id))
        return null;
      return m_itemMap.get(id);
      }
    }

  }
