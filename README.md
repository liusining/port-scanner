# Port Scanner

Scan TCP open ports on a server.

## Getting Started

```
cc -o scan tcp_scan.c
scan IPV4_ADDRESS
```

**Usage**

```
$ ./scan -h
Usage: [-SC] <ip address>
-S default, scan with SYN packets
-C scan with connect(), i.e. the whole 3-way handshake procedure
```

**NOTE**

1. Scanning with SYN packets (the default mode) only works on linux, since [*BSD systems are not allowing applications to read packets through raw sockets](https://github.com/isdrupter/busybotnet/blob/master/miscutils/synscan.c#L36) for [security issues](http://squidarth.com/networking/systems/rc/2018/05/28/using-raw-sockets.html#raw-sockets-and-security).
2. When you want to scan ports with SYN packets (on linux, of course), sudo priviledge may be required. Or, you can use the [linux capbilities](http://squidarth.com/networking/systems/rc/2018/05/28/using-raw-sockets.html#the-solution-linux-capabilities) with `sudo setcap cap_net_admin,cap_net_raw=eip scan`, and then run it as normal.

## References

- [A brief programming tutorial in C for raw sockets](http://www.cs.binghamton.edu/~steflik/cs455/rawip.txt)
- [SYN flooder](https://jakash3.wordpress.com/2011/01/21/syn-flooder/)
- [Using Linux Raw Sockets](http://squidarth.com/networking/systems/rc/2018/05/28/using-raw-sockets.html#the-solution-linux-capabilities)
- [Linux IPv4 raw sockets](http://man7.org/linux/man-pages/man7/raw.7.html)
