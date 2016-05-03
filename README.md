gwsocket [![Build Status](https://travis-ci.org/allinurl/gwsocket.svg?branch=master)](http://travis-ci.org/allinurl/gwsocket) [![gwsocket](http://gwsocket.io/badge?v0.1)](http://gwsocket.io)
========

## What is it? ##
**gwsocket** is a free, standalone, WebSocket Server. It sits between your
application and the client's browser, giving the ability for bidirectional
communication between these two with ease and flexibility. More info at:
[http://gwsocket.io](http://gwsocket.io/?src=gh).

![gwsocket terminal](http://gwsocket.io/images/gwsocket-terminal-620.gif?2016050202)

## Features ##
* Message Fragmentation per section [5.4](https://tools.ietf.org/html/rfc6455#page-33)
* UTF-8 Handling
* Framing (Text & Binary messages)
* Multiplexed non-blocking network I/O
* Ability to pipe data in/out in two different modes (stdin/stdout & strict mode)
* It passes the [Autobahn Testsuite](http://gwsocket.io/autobahn/) :)
* and of course, [Valgrind](http://valgrind.org/) tested.
* missing something?, please feel free to post it on Github.

## Why gwsocket? ##
I needed a simple, fast, no-dependencies, RFC6455 compliant WebSocket Server
written in C that I could use as a library for the upcoming version (v1.0) of
[**GoAccess**](https://goaccess.io/) by simply piping data in and out —
WebSockets made easy!

## How it Works? ##
Very simple, just redirect/send the output **(stdout)** from your application to a
file and let gwsocket do the rest. e.g., tailing your server's logs into the
browser:

    tail -f /var/log/nginx/access.log > /tmp/wspipein.fifo

BTW, you can also get the client's data into **(stdin)** your application.

Tip: You can even send your favorite NCurses program's output. See screencast
above.

## More Examples? ##
Looking for more examples and details on how it works? Head to the [man page](http://gwsocket.io/?src=gh)
for more details.

## Installation ##
Installing gwsocket is pretty easy. Just download, extract and compile it with:

```
$ wget http://tar.gwsocket.io/gwsocket-0.1.tar.gz
$ tar -xzvf gwsocket-0.1.tar.gz
$ cd gwsocket-0.1/
$ ./configure
$ make
# make install
```
No dependencies needed. How nice isn't it :), well almost, you need `gcc`, `make`, etc.

## Build from GitHub ##
```
$ git clone https://github.com/allinurl/gwsocket.git
$ cd gwsocket
$ autoreconf -fiv
$ ./configure
$ make
# make install
```

## Data Modes ##
In order to establish a channel between your application and the client's
browser, gwsocket provides two methods that allow the user to send data in and
out. The first one is through the use of the standard input (stdin), and the
standard output (stdout). The second method is through a fixed-size header
followed by the payload. See options below for more details.

### STDIN/STDOUT ###
The standard input/output is the simplest way of sending/receiving data to/from
a client. However, it's limited to broadcasting messages to all clients. To
send messages to or receive from a specific client, use the strict mode in the
next section. See language specific examples [here](http://gwsocket.io/).

### Strict Mode ###
gwsocket implements its own tiny protocol for sending/receiving data. In
contrast to the **stdin/stdout** mode, the strict mode allows you to
send/receive data to/from specific connected clients as well as to keep track
of who opened/closed a WebSocket connection. It also gives you the ability to
pack and send as much data as you would like on a single message. See language
specific examples [here](http://gwsocket.io/).

## Command Line / Config Options ##
The following options can be supplied to the command line.


| Command Line Option          | Description                                                         |
| ---------------------------- | --------------------------------------------------------------------|
| `-p --port`                  | Specifies the port to bind.                                         |
| `-h --help`                  | Command line help.                                                  |
| `-V --version`               | Display version information and exit.                               |
| `--access-log=<path/file>`   | Specifies the path/file for the access log.                         |
| `--bind=<addr>`              | Specifies the address to bind.                                      |
| `--echo-mode`                | Set the server to echo all received messages.                       |
| `--max-frame-size=<bytes>`   | Maximum size of a websocket frame.                                  |
| `--origin=<origin>`          | Ensure clients send the specified origin header upon handshake.     |
| `--pipein=<path/file>`       | Creates a named pipe (FIFO) that reads from on the given path/file. |
| `--pipeout=<path/file>`      | Creates a named pipe (FIFO) that writes to the given path/file.     |
| `--strict`                   | Parse messages using strict mode. See man page for more details.    |

## Roadmap ##
* Support for `epoll` and `kqueue`.
* Support for SSL
* Add more command line options
* Add configuration file
* Please feel free to open an issue to discuss a new feature.

## License ##
MIT Licensed

## Contributing ##

Any help on gwsocket is welcome. The most helpful way is to try it out and give
feedback. Feel free to use the Github issue tracker and pull requests to
discuss and submit code changes.

Enjoy!
