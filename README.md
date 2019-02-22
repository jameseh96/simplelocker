# Simplelocker

A small tool to run a command on certain events (before suspend, after some period of idling etc.). Originally designed to launch a screen locker such as i3lock, but anything can be run.

## Getting Started

Simplelocker can be launched as:

`simplelocker -i <x> -c <cmd>`

To run <cmd> after the session has been idle for <x> seconds.  

For a more complicated example,

`simplelocker -i 60 -s -l -c i3lock`

Will launch i3lock:
* `-i 60` after 60 seconds idling
* `-s` immediately before the system suspends/sleeps
* `-l` in response to a dbus method call (see [Controlling a running instance](#Controlling-a-running-instance))


#### Controlling a running instance

If simplelocker has been launched with `-l` (say, in an i3 config file), it can be controlled with `simplelocker -r`.

```
# daemon
simplelocker -l -s -i 60 -c i3lock

...

# remote control
simplelocker -r Lock 		# launches i3lock

simplelocker -r Disable		# disables lock-on-idle BUT will still
							# lock on sleep, `-r Lock` etc.

simplelocker -r Enable		# (re)enables lock-on-idle. Still
							# requires the daemon was launched with -i

```

Options:
```
Allowed options:
  -s [ --sleep ]               Run on sleep
  -S [ --shutdown ]            Run on shutdown
  -i [ --idle ] arg            Run on idle
  -d [ --dbus ] arg            Run on dbus method call
  -b [ --dbus-disable ] arg    Disable run-on-idle on this dbus method call
  -e [ --dbus-enable ] arg     Enable run-on-idle on this dbus method call
  -c [ --cmd ] arg             Command to run
  -h [ --help ]                produce help message

DBus options:
  -l [ --listen ]              Register a dbus service to receive method calls 
                               for control
  -D [ --dbus-name ] arg       Dbus service name to bind
  -P [ --dbus-path ] arg       Dbus object path to register
  -I [ --dbus-interface ] arg  Dbus interface to register

Remote call options:
  -r [ --remote ] arg          Call dbus method (i.e., `-r Lock` to lock a 
                               running instance)
```


## Prerequisites

TBC

## Installing

Simplelocker isn't packaged for any distros (yet), but it is straightforward to build.

```
git clone --recurse-submodules https://github.com/jameseh96/simplelocker.git simplelocker
cd ./simplelocker
mkdir ./build
cd !$
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install
```

## Built With

* [Boost](https://github.com/boostorg/boost)
* [boost-cmake](https://github.com/Orphis/boost-cmake)
* [forked sdbus-cpp](https://github.com/jameseh96/sdbus-cpp)
    * forked from [sdbus-cpp](https://github.com/Kistler-Group/sdbus-cpp)
