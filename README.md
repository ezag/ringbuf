libringbuf
==========

Description
-----------

Small and simple C library implementing byte [circular (aka ring) buffer][1].

[1]: http://en.wikipedia.org/wiki/Circular_buffer


Requirements
------------

* CMake 2.8.8
* GCC 4.7.1
* GLib 2.32.4 (optional, for unit tests)

Different versions haven't been tested but might work as well.


Installation
------------

    cmake -DCMAKE_INSTALL_PREFIX=/usr/local .
    make
    make install


Documentation
-------------

See `ringbuf.h` for reference, function names are self-describing (I hope so).


Contacts
--------

See [project page](https://github.com/ezag/ringbuf).
