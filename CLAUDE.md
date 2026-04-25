# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

```bash
mkdir -p build && cd build
cmake ..
make
```

Run a single module's build:
```bash
cd build && make ratingCalculator_service
cd build && make ratingCalculator_testClient
```

## Tests

Uses Boost.Test framework. Run via CTest from the build directory:

```bash
cd build && ctest              # all tests
cd build && ctest --verbose    # with output
```

Tests exist only for `core`, `webapi`, and `tempstore` modules. The `service` and `testClient` modules have no unit tests.

## Running

```bash
# Service (default port 88888, 5s update period, 5 threads)
./build/service/ratingCalculator_service -p 88888 -t 5 --threads 5

# Test client (connects to localhost:88888, 100 users, 1s message interval)
./build/testClient/ratingCalculator_testClient -a localhost -p 88888 -u 100 -t 1
```

## Architecture

The project is a WebSocket backend that calculates trader ratings based on won deals during a rolling weekly period (Monday 00:00:00 to next Monday 00:00:00).

**Module dependency chain:**

```
core (types/interfaces)
  └── webapi (WebSocket transport + serialization)
        └── tempstore (in-memory storage + rating engine)
              └── service (WebSocket server executable)
testClient (standalone test traffic generator)
```

**Key layers:**

- **`core/`** — Data types (`UserIdentifier`, `DealInformation`, `UserRelativeRating`), message model (`Model.hpp`), storage interfaces (`IUserDataStore`, `IUserDealDataStore`), and `MultiKeyVolatileContainer` (sorted multi-key in-memory store).

- **`webapi/`** — WebSocket transport (`Simple-WebSocket-Server`, header-only, vendored). `WsProtocol` adds ACK/resend logic on top of raw WebSocket with a dedicated resend thread. Serialization uses Boost.PropertyTree for JSON (note: all values are quoted strings — non-standard JSON).

- **`tempstore/`** — `UserRatingProvider` is the rating calculation engine: stores deals, sorts by amount, computes relative positions (user rank + neighbors above/below). `DataStoreFactory` creates in-memory `UserDataStore` and `UserDealDataStore`. All data is volatile (no persistence); data clears at week boundaries.

- **`service/`** — `ApplicationService` wires up the WebSocket server at endpoint `/rating`, initializes `WsProtocol`, and runs a thread pool. `UserRatingWatcher` tracks connected users and broadcasts periodic `UserRelativeRating` updates on a configurable interval.

**Message flow for a deal:**
1. Client sends `user_deal_won` wrapped in `WsData` over WebSocket
2. `WsProtocol` receives, parses, routes to application layer
3. `UserRatingProvider` records deal in `UserDealDataStore`
4. `WsProtocol` sends `WsAck` back
5. `UserRatingWatcher` periodically pushes updated `UserRelativeRating` to all connected clients

## Dependencies

- CMake >= 3.7
- GCC with C++11 support
- Boost >= 1.64.0 (`system`, `program_options`, `serialization`, `unit_test_framework`)
- OpenSSL >= 1.0.2
- pthreads
