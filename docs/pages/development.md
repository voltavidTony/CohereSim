# Developer Manual

[TOC]

This page is still under construction, and more content will appear in later updates.

## Debugging Features

Debugging features are parts of the code which can be enabled via pre-defines. In other words, parts of the code that will be excluded from compilation if their required define is not present is a debugging feature.

To make use of debugging features, define them in the `CPPFLAGS` variable when executing the make command. The general format is `make [target] CPPFLAGS=-D<pre-define>`. When using multiple debugging features, use multiple `-D` options, separated by a space and contained in quotes.

__Note:__ It is not recommended to make use of debugging features when operating in [batch metrics mode](docs/pages/cache_sim.md), as this mode of operation is intended as a "production" mode for processing large trace files. Further, the debugging features may not make use of synchronization mechanisms, leading to undefined behavior or unintelligible output.

### Write Timestamp Checking

This feature is enabled with the `WRITE_TIMESTAMP` predefine.

The phrase 'timestamp' refers to the number of the current memory access. It is a 0-based index, meaning that a timestamp of 10, for instance, represents the 11th memory access.

When enabled, each cache line, in addition to its state and tag, also records the last timestamp it received a write, either through a write access, bus update message, or the first time the memory block is allocated in any cache. Whenever a read or write access is completed, CohereSim checks that all valid cache lines belonging to the recently accessed address have the same timestamp. If not, the IDs of any out of date cache lines are printed, along with the details of the access, namely the address, whether it is a read or write, and the current timestamp.

This feature is added as a verification measure when writing code for CohereSim, since the software does not process the actual data that is stored in a cache. The codebase can be analyzed for errors when using a debugger and setting a breakpoint in the `MemoryBus::verifyTimestamp` method to catch the exact moment when a cache line becomes out of date.
