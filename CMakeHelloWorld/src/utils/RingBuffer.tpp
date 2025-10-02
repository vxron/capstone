// for .tpp file
#include <cassert>
#include <utility> // std::move
#include "RingBuffer.h"

/* Default Constructor
* Initialize the queue with fixed capacity and pre-construct storage
*/
template<typename T>
RingBuffer_C<T>::RingBuffer_C(std::size_t capacity) : cap_(capacity) {
	// Debug assertion
	assert(capacity > 0 && "[ERROR] RingBuffer_C must have a capacity > 0");
	buffer_.resize(cap_); // pre-allocate storage for the ring buffer so that we don't have to re-allocate on every push/pop
}

/*
* push_drop_oldest:
* Goal: 
*/
template<typename T>
bool RingBuffer_C<T>::push_drop_oldest(T chunk) {
	std::lock_guard<std::mutex> lock(mtx_); // lock during push

	if (closed_) {
		return false; // cannot push to a closed queue
	}

	buffer_[tail_] = std::move(chunk); // always right at current tail
	tail_++; // increment tail to next pos
	if (tail_ == cap_) {
		tail_ = 0; // wrap around
	}

	if (count_ != cap_) {
		count_++;
	}
	else if (count_ == cap_) {
		// Buffer is full, drop the oldest chunk (head)
		head_++; // next pop should be from "next oldest" since we're dropping the oldest (to become newest)
		if (head_ == cap_) { 
			head_ = 0; // wrap around
		}
	}

	// Notify waiting consumer that new data is available (just pushed)
	not_empty_cv_.notify_one();

}

/*
* NON BLOCKING POP:
* used by places that must not block (e.g. UI thread) gives immediate "is anything ready?" check and returns immediately if not
*/
template<typename T>
bool RingBuffer_C<T>::try_pop(T& chunkOut) {
	std::lock_guard<std::mutex> lock(mtx_); // lock during pop

	if (count_ == 0) {
		return false; // queue empty, nothing to pop
	}

	chunkOut = std::move(buffer_[head_]); // move oldest chunk to output param
	head_++; // increment head to next pos
	if (head_ == cap_) {
		head_ = 0; // wrap around
	}

	count_--;

	return true;

}

/*
* POP:
* Behavior: blocking
* used by consumer thread whose SOLE PURPOSE is to wait for data (e.g., decoder thread) --> can sleep when the buffer is empty and wakes on signals (new pushes) or close() (for shutdown)
* - if the queue is empty and not closed, pop() will block (wait) until either a producer pushes something (count_ > 0) or someone calls close()
* - if the queue is empty and closed, pop() should return no value (std::nullopt) --> terminal
* - if the queue is not empty, pop() should return the oldest chunk immediately (try_pop() behavior)
*/
template<typename T>
std::optional<T> RingBuffer_C<T>::pop() {
	std::unique_lock<std::mutex> lock(mtx_); // lock during pop, unique_lock needed for wait

	// chat wants you to use predicate form instead of while loop, idk?
	while (count_ == 0 && !closed_) {
		// wait until not empty or closed
		not_empty_cv_.wait(lock); // releases lock while waiting, re-acquires lock when notified
	}

	if (count_ == 0 && closed_) {
		return std::nullopt; // queue empty and closed, nothing to pop
	}

	auto chunkOut = std::optional<T>{ std::move(buffer_[head_]) }; // move oldest chunk to output param as an xvalue
	head_++; // increment head to next pos
	if (head_ == cap_) {
		head_ = 0; // wrap around
	}

	count_--;

	return chunkOut;
}