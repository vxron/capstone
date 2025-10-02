/*
==============================================================================
	File: RingBuffer.h
	Desc: Non-Blocking single-producer, single-consumer (SPSC) ring buffer
	for passing data between two threads.
	This header is used by:
  - Acquisition (producer): reads short "Chunks" and stores them in this SPSC
  ring buffer.
  - Decoder (consumer): gets handed chunks from the buffer.

==============================================================================
*/

#pragma once
#include <cstddef> // std::size_t
#include <optional>
#include <mutex>
#include <condition_variable>
#include <vector>

template<typename T>
class RingBuffer_C {
public:
	// CONSTRUCTORS/DESTRUCTORS
	explicit RingBuffer_C(std::size_t capacity); // explicit because we don't want implicit conversions from size_t to RingBuffer_C
	// This queue should not be copyable: delete copy constructor/assignment operator
	RingBuffer_C(const RingBuffer_C&) = delete;
	RingBuffer_C& operator=(const RingBuffer_C&) = delete;
	// This queue can be moveable (RAII): default move constructor/assignment operator
	RingBuffer_C(RingBuffer_C&&) = default;
	RingBuffer_C& operator=(RingBuffer_C&&) = default;
	// Default destructor
	~RingBuffer_C() = default;
	
	// DATA OPERATION METHODS
	bool push_drop_oldest(T chunk); // pass-by-value so we can copy it into the buffer with std::move(chunk), non-const to do this move
	std::optional<T> pop(); // waits until a chunk is available or the queue is closed; returns no value when closed and drained
	bool try_pop(T& chunkOut); // non-blocking, pass by ref so we can write oldest chunk to chunkout if available

	// STATE METHODS
	void close(); // close the queue; after this, push() will fail and pop() will return no value when drained

	// GETTERS
	std::size_t get_capacity() const; // capacity of the ring buffer (max number of chunks it can hold)
	std::size_t get_approx_size() const; // approx current number of chunks in the ring buffer - not exact because we lock momentarily to return count_

private:
	const std::size_t cap_; // capacity of the ring buffer (max number of chunks it can hold)
	std::size_t count_ = 0; // current number of chunks in the ring buffer
	std::size_t head_ = 0;  // index of the next chunk to pop (oldest chunk)
	std::size_t tail_ = 0;  // index of the next chunk to push
	
	std::vector<T> buffer_; // the ring buffer storage

	bool closed_ = false;
	mutable std::mutex mtx_; // mutex for synchronizing access to the ring buffer; mutable so read-only functions can still lock it
	std::condition_variable not_empty_cv_; // condition variable to notify consumer when new data is available (non empty queue)

	std::condition_variable not_full_cv_; // TODO: decide if needed (if pop(), try_pop() notifies producers ab anything)

}; // RingBuffer_C

#include "RingBuffer.tpp"