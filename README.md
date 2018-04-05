# ring-buffer
This package provides ring buffer classes for use with C++17.

C++17 features are used if available, and backward compatibility is implemented up to C++14.

## Description

The purpose of a ring buffer is to provide a unidirectional FIFO communication from a thread to another.
The implementation of this data structure can be lock-free, making it suitable for real-time programming.

I implement two variants of the ring buffer.

- **Ring_Buffer** is a *bounded*, *lock-free* ring buffer. The capacity is fixed and determined at instantiation.
It can only store messages up to the capacity of the buffer.
- **Soft_Ring_Buffer** is an *unbounded*, *mostly lock-free* ring buffer. The storage expands as write operations require it,
by a factor of 1.5. The access is protected by a shared-exclusive lock, and only blocks while the buffer expands.
This object is adequate for soft real-time, when communicating all the messages is more important than missing a few deadlines.

## Programming interface

`Ring_Buffer(size_t capacity);`

Instantiate the ring buffer with the given capacity.

`size_t capacity() const;`

Return the capacity.

### Writer interface

`size_t size_free() const;`

Return the number of bytes available to write to.

`template <class T> bool put(const T &x);`

If the buffer has enough room (or the buffer is *unbounded*), store the `sizeof(T)` bytes of `x`, then return `true`. Otherwise, return `false`.

`template <class T> bool put(const T *x, size_t n);`

Similar to `put` above, except it stores an array of `n` consecutive `T` elements.

### Reader interface

`size_t size_used() const;`

Return the number of bytes available to read.

`bool discard(size_t len);`

If the ringbuffer contains at least `len` bytes, extract them and ignore them, then return `true`. Otherwise, return `false`.

`template <class T> bool get(T &x);`

If the buffer contains enough data, extract `sizeof(T)` bytes and assign this data to `x`, then return `true`. Otherwise, return `false`.

`template <class T> bool get(T *x, size_t n);`

Similar to `get` above, except it extracts an array of `n` consecutive `T` elements.

`template <class T> bool peek(T &x);`

If the buffer contains enough data, extract `sizeof(T)` bytes without removing them from storage, assign this data to `x`, then return `true`. Otherwise, return `false`.

`template <class T> bool peek(T *x, size_t n);`

Similar to `peek` above, except it extracts an array of `n` consecutive `T` elements.
