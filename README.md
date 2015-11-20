# mmountd
###Minimalistic (auto) mount daemon

This is an automounter with a minimal approach.

It takes only a few arguments:

`mmountd timeout mount-options device mountpoint [helper]`

and performs automounting just on this device/mountpoint.
The helper will be called after each mount/unmount operation:

`helper mount|unmount`


If there are more devices start more instances of mmountd.
This is usually controlled by small scripts, for an example
please take a look at [mmountd-OpenWRT](https://github.com/stagprom/mmountd-OpenWRT)
