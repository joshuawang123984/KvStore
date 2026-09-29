# Distributed Key Value Store

A persistent key value store built from scratch in C++ involving **networking, concurrency, storage systems, and distributed systems**.

The project started as an in memory key value store and is being developed into a distributed system with persistent storage, replication, and fault tolerance.

## Features

### Currently Implemented

* **Key value storage**

  * `SET`, `GET`, and `DELETE` operations
  * In memory storage using `std::unordered_map`

* **TCP server**

  * Client communication over TCP sockets
  * Custom text based command protocol
  * Newline based message framing to handle TCP's byte-stream behavior

* **Concurrent clients**

  * Multiple clients can connect simultaneously
  * Mutex-protected shared key-value store

* **Persistent storage**

  * Append only operation log
  * Data survives server restarts
  * Startup recovery by replaying the operation log

* **Log compaction**

  * Rewrites the log using the store's current state when file grows past a limit
  * Prevents the append only log from growing indefinitely

* **Stress testing**

  * Concurrent clients performing thousands of operations
  * Validates thread safety and server behavior under concurrent load

* **Replication**

  * Replicate writes from a primary node to replica nodes
  * Run multiple independent server processes

* **Failure handling**

  * Detect replica failures and continue Serving from the primary
  * reconnect failed replicas, resync replica state

### Planned

* **Sharding**

  * Partition keys across multiple nodes
  * Route requests to the appropriate node

## Persistence

Writes are stored in an append only log file:

```text
SET user Joshua
SET language C++
DELETE language
```

When the server starts, it replays the log file to reconstruct the kv store.

For example:

```text
SET name Joshua
DELETE name
SET name Josh
```

results in:

```text
name → Josh
```

When the log reaches a configured operation threshold, **log compaction** rewrites the log to contain only the current state.

## Example

Start the server:

```bash
bash test_cluster.sh <# of processes>
```

Connect using a TCP client:

```bash
nc localhost <port_number>
```

Then:

```text
SET name Joshua
name set with the value of: Joshua **(this is program output, not user input)**

GET name
Joshua **(this is program output, not user input)**

DELETE name
Variable successfully deleted. **(this is program output, not user input)**

GET name
Variable not found. **(this is program output, not user input)**
```

## Building

Requirements:

* C++17
* `g++`
* POSIX sockets
* Make

Build the server:

```bash
make
```

Run it:

```bash
bash test_cluster.sh <# of processes>
```

Clean build artifacts:

```bash
make clean
```

## Goals

This project is primarily a systems programming and distributed systems exercise. The goal is to understand how the components behind a distributed database work by implementing them directly rather than relying on database or networking frameworks.

The main areas explored are:

* TCP networking
* Client/server architecture
* Concurrency and synchronization
* Persistent storage
* Append only logging
* Crash recovery
* Log compaction
* Replication
* Fault tolerance