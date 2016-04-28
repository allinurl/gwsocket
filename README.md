GWSocket
========

## What is it? ##
GWSocket is a standalone, simple, yet powerful rfc6455 complaint WebSocket
Server, written in C.

## Why? ##
I needed a simple, fast, no-dependencies, rfc6455 complaint WebSocket Server
written in C that I could use as a library for the upcoming version (v1.0) of
[**GoAccess**](https://github.com/allinurl/goaccess) by simply piping data in
and out.

## Features ##

* Message Fragmentation (rfc [5.4](https://tools.ietf.org/html/rfc6455#page-33))
* UTF-8 Handling
* Framing (Text & Binary messages)
* Non-blocking network I/O
* Ability to pipe data in and out in two different modes (stdin/stdout & strict mode)
* It passes the Autobahn Testsuite :)
* and of course, [Valgrind](http://valgrind.org/) tested.

## Data Modes ##

There are two ways to communicate to the server, the first and default is by
piping data from your application stdout into a GWSocket named pipe. The second
mode is using a strict mode pipe. See Usage below for details.

## Usage ##

By default, running GWSocket without any command line options, will run in
standard mode, and listening on port `7890` and all IPv4 addresses `0.0.0.0`.
For instance,

    # gwsocket

See command line options below for more options.

### How to Send Data? ###

#### Standard Mode ####

It's fairly easy to send data using the standard mode to all the connected
clients:

    # tail -f /var/log/nginx/access.log > /tmp/wspipein.fifo

OR

    # top -b > /tmp/wspipein.fifo

OR

    # while true; do echo $RANDOM > /tmp/wspipein.fifo; sleep 1; done

You can send as many bytes `PIPE_BUF` can hold. If a message is greater than
`PIPE_BUF`, it would send the rest on a second message or third, and so on. See
**strict mode** for more control over messages.

#### Strict Mode ####

The strict mode in GWSocket allows you to send data to specific connected
clients as well as to keep track of who opened/closed a WebSocket connection.
It also gives you the ability to pack and send as much data as you would like
to a client or broadcast the message to all of them.

##### Data Format #####

GWSocket implements its own "mini-procotol", that way it knows how much data is
coming through the pipe, to which client is going and the type of message we
are sending (binary/text). Here's how the format is structured.

```
0                   1                   2                   3
+---------------------------------------------------------------+
|                  Client Socket Id (listener)                  |
+---------------------------------------------------------------+
|              Message Type (binary: 0x2 / text: 0x1)           |
+---------------------------------------------------------------+
|                       Payload length                          |
+---------------------------------------------------------------+
|                        Payload Data                           |
+---------------------------------------------------------------+
```

The first `12 bytes` (uint32_t) are packed in network byte order and contain
the "meta-data" of the message we are sending. The rest of it is the atual
message.

## Roadmap ##

* Add SSL support.
* Replace `select(2)` for `epoll(2)` and `kqueue(2)`.
* Message logging
* Command line options.
