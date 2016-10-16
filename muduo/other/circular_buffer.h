/******************************************************
# EMAIL   : alexstocks@foxmail.com
# MOD   : 2016-10-15 14:49
# FILE  : circularbuffer.h
******************************************************/

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <iostream>

template <typename T>
class circular_buffer
{
public:
  explicit circular_buffer(size_t capacity);
  circular_buffer(const circular_buffer<T> &rhs);
  circular_buffer(circular_buffer<T>&& rhs);
  ~circular_buffer() { if (buffer_) delete[] buffer_; }

  circular_buffer<T>& operator=(circular_buffer<T> rhs);

  size_t size() const { return (write_pos_ + capacity_ - read_pos_) % capacity_; }
  size_t capacity() const { return capacity_; }

  bool empty() const { return read_pos_ == write_pos_; }
  bool full() const { return (write_pos_ + 1) % capacity_ == read_pos_; }

  bool push_back(T item);
  void pop_front();
  const T& front();

  friend void swap(circular_buffer<T> &lhs, circular_buffer<T> &rhs)
  {
    std::swap(lhs.buffer_, rhs.buffer_);
    std::swap(lhs.capacity_, rhs.capacity_);
    std::swap(lhs.read_pos_, rhs.read_pos_);
    std::swap(lhs.write_pos_, rhs.write_pos_);
  }

private:
  T* buffer_;
  size_t capacity_;
  size_t read_pos_;
  size_t write_pos_;

  circular_buffer();
};

template<typename T>
circular_buffer<T>::circular_buffer()
  : buffer_(nullptr)
  , capacity_(0)
  , read_pos_(0)
  , write_pos_(0)
{
}

template<typename T>
circular_buffer<T>::circular_buffer(size_t capacity)
  : circular_buffer()
{
  if (capacity < 1) throw std::length_error("Invalid capacity");

  buffer_ = new T[capacity];
  capacity_ = capacity;
}

template<typename T>
circular_buffer<T>::circular_buffer(const circular_buffer<T> &rhs)
  : buffer_(new T[rhs.capacity_])
  , capacity_(rhs.capacity_)
  , read_pos_(rhs.read_pos_)
  , write_pos_(rhs.write_pos_)
{
  std::copy(rhs.buffer_, rhs.buffer_ + capacity_, buffer_);
}

template<typename T>
circular_buffer<T>::circular_buffer(circular_buffer<T>&& rhs)
  : circular_buffer()
{
  swap(*this, rhs);
}

template<typename T>
circular_buffer<T>&
circular_buffer<T>::operator=(circular_buffer<T> rhs)
{
  swap(*this, rhs);
  return *this;
}

template<typename T>
bool
circular_buffer<T>::push_back(T item)
{
  if (full()) {
    return false;
  }

  buffer_[write_pos_] = item;
  write_pos_ = (write_pos_+1) % capacity_;
  return true;
}

template<typename T>
const T&
circular_buffer<T>::front()
{
  static T t;
  if (empty()) {
    return t;
  }

  return buffer_[read_pos_];
}

template<typename T>
void
circular_buffer<T>::pop_front()
{
  if (empty()) {
    return;
  }

  read_pos_ = (read_pos_+1) % capacity_;
}

#endif // CIRCULAR_BUFFER_H
