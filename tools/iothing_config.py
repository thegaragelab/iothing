#!/usr/bin/env python
#----------------------------------------------------------------------------
# 20-Mar-2016 ShaneG
#
# Initial version. Requires the 'zeroconf' and 'requests' modules.
#
# Use 'pip install zeroconf requests'
#----------------------------------------------------------------------------
from six.moves import input
from zeroconf import ServiceBrowser, Zeroconf
from socket import inet_ntoa
from threading import Lock
from time import sleep
from uuid import uuid4
import requests
import json

def getDeviceId(address):
  """ Get the unique node ID, assigning one if needed
  """
  r = requests.get("http://%s/config" % address)
  if r.status_code <> 200:
    return None
  # Get the configuration
  try:
    config = json.loads(r.text)
    nodeid = config.get("node", "")
    if nodeid == "":
      # Need to create a new node ID
      nodeid = str(uuid4())
      r = requests.post("http://%s/config" % address, json={ "node": nodeid })
      if r.status_code <> 200:
        return None
      config = json.loads(r.text)
      if not config.get("status", False):
        return None
      if config.get("node", "") <> nodeid:
        return None
    return nodeid
  except:
    return None

class IoThingListener(object):
  """ Listen for IoThing devices on the network
  """

  def __init__(self):
    self._lock = Lock()
    self._pending = list()

  def remove_service(self, zeroconf, type, name):
    print("Service %s removed" % (name,))

  def add_service(self, zeroconf, type, name):
    info = zeroconf.get_service_info(type, name)
    if info is None:
      self._lock.acquire()
      self._pending.append((type, name))
      self._lock.release()
    else:
      address = inet_ntoa(info.address)
      nodeid = getDeviceId(address) or "unknown"
      print "Found IoThing %s at %s" % (nodeid, address)

  def update(self):
    self._lock.acquire()
    unresolved = list()
    for type, name in self._pending:
      info = zeroconf.get_service_info(type, name)
      if info is None:
        unresolved.append((type, name))
      else:
        address = inet_ntoa(info.address)
        nodeid = getDeviceId(address) or "unknown"
        print "Found IoThing %s at %s" % (nodeid, address)
    self._pending = unresolved
    self._lock.release()

if __name__ == "__main__":
  # Set up the listener
  zeroconf = Zeroconf()
  listener = IoThingListener()
  browser = ServiceBrowser(zeroconf, "_iothing._tcp.local.", listener)
  try:
    while True:
      listener.update()
      sleep(0.5)
  finally:
    zeroconf.close()
