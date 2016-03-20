#!/usr/bin/env python
#----------------------------------------------------------------------------
# 20-Mar-2016 ShaneG
#
# Initial version. Requires the 'zeroconf' package from
#
#   https://pypi.python.org/pypi/zeroconf
#
# Use 'pip install zeroconf'
#----------------------------------------------------------------------------
from six.moves import input
from zeroconf import ServiceBrowser, Zeroconf
from socket import inet_ntoa
from threading import Lock
from time import sleep

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
      print "Found IoThing at %s" % (inet_ntoa(info.address),)

  def update(self):
    self._lock.acquire()
    unresolved = list()
    for type, name in self._pending:
      info = zeroconf.get_service_info(type, name)
      if info is None:
        unresolved.append((type, name))
      else:
        print "Found IoThing at %s" % (inet_ntoa(info.address),)
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
