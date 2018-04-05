# ring-buffer
This package provides ring buffer classes for use with C++17.

Backward compatibility with the C++14 standard is provided if Boost libraries are available.

## Description

The purpose of a ring buffer is to provide a unidirectional FIFO communication from a thread to another.
The implementation of this data structure can be lock-free, making it suitable for real-time programming.

I implement two variants of the ring buffer.

- **Ring_Buffer** is a *bounded*, *lock-free* ring buffer. The capacity is fixed and determined at instantiation.
It can only store messages up to the capacity of the buffer.
- **Soft_Ring_Buffer** is an *unbounded*, *mostly lock-free* ring buffer. The storage expands as write operations require it,
by a factor of 1.5. The access is protected by a shared-exclusive lock, and only blocks while the buffer expands.
This object is adequate for soft real-time, when communicating all the messages is more important than missing a few deadlines.
