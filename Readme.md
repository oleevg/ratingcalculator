# Project Title

Network service that provides traders rating calculation according to their successful deals during the specified period.

### Description

WebSocket protocol is used to communicate with the service. [Simple-WebSocket-Server](https://github.com/eidheim/Simple-WebSocket-Server) header-only implementation is used in project to deal with WebSocket protocol.

The service provides 'http://hostname:port/rating' WebSocket endpoint to receive clients connections.

The service supports the following client messages transmitted in JSON format:

* user_registered(id, name)
* user_renamed(id, name)
* user_deal_won(id, time, mount)
* user_connected(id)
* user_disconnected(id)


The service uses its own simple protocol (WsProtocol) upon WebSocket messages. So any supported message must be wrapped in WsProtocol's message (WsMessage).

The service's response message contains specific user relative rating. The rating is calculated within the limited period of time starting from the current Monday 00:00:00 and ending on the next Monday 00:00:00. The service sends the response message with rating information on every user's connection and also periodically in the specified timeout for all connected users.

The project contains simple test client that emulates users activities and sends the corresponding messages to the server. See section 'Usage' for additional information.

### Prerequisites

* CMake build system (>= 3.7) is used to compile the project. Compiling tested with gcc6.3.0.
* Compiler support for C++11 standard is required.
* boost (>= 1.64.0) is required to build the project.
* openssl (>=1.0.2) is required to build the project.
* [zlog library](https://github.com/HardySimpson/zlog) might be used to collect the services' messages and redirect it to an ordinary file, syslog service etc.

### Getting Started

* Download or clone the project's repository.
* Install all required tools and libraries listed in 'Prerequisites' section.
* Create build directory inside project folder and compile the sources. Building in source tree directory is forbidden.

```
    $ mkdir .build && cd .build
    $ cmake ..
    $ make
```
* Getting help:

```
    $ cd .build/service
    $ ./ratingCalculator_service -h
    Usage: ./ratingCalculator_service [options]...
    Options:
      -p [ --port ] arg (=88888) The port number to listen.
      -t [ --period ] arg (=5)   Rating update period in seconds.
      --threads arg (=5)         The service's thread pool size.
      -h [ --help ]              As it says.
```

### Usage

* Start the service with the required arguments:

```
    $ cd .build/service
    $ ./ratingCalculator_service
```

* Testing the service with the provided test client application.

  Start the test client application with the required arguments:
```
   $ cd .build/testClient
   $ ./ratingCalculator_testClient -h
   Usage: ./ratingCalculator_testClient [options]...
   Options:
     -a [ --address ] arg (=localhost) The server address to connect to.
     -p [ --port ] arg (=88888)        The port number to connect to.
     -t [ --timeout ] arg (=1)         Timeout in seconds to send generated test
                                       requests.
     -u [ --users ] arg (=100)         Maximum number of test users.
     -h [ --help ]                     As it says.
```

By default the server and the client applications write log messages on their corresponding terminals.

### Running the tests
  CMake build framework might be used to run available unit tests. Just run 'ctest' inside build directory after building the sources.

```
    $ ctest
    $ ctest --verbose
```
  *More tests to be done*

### Known issues

Boost.PropertyTree is used to work with JSON format. Its way to represent objects in JSON format is not conformed to the JSON standard. As a consequence all the types are quoted the same way as usual strings.

##$ Authors
Oleg Fedorov, fedorov.ftf@gmail.com


##$ License

To be done...


