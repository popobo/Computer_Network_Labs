#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  const uint64_t capacity_left = capacity_ - bytes_.size();
  const uint64_t push_len = data.size() > capacity_left ? capacity_left : data.size();

  if ( 0 == push_len ) {
    return;
  }

  bytes_.insert( bytes_.end(), data.begin(), data.begin() + static_cast<int64_t>( push_len ) );
  bytes_pushed_ += push_len;
}

void Writer::close()
{
  is_closed_ = true;
}

void Writer::set_error()
{
  is_error_ = true;
}

bool Writer::is_closed() const
{
  return is_closed_;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - bytes_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return bytes_pushed_;
}

string Reader::peek() const
{
  return string { bytes_.begin(), bytes_.end() };
}

bool Reader::is_finished() const
{
  return is_closed_ && bytes_.empty();
}

bool Reader::has_error() const
{
  return is_error_;
}

void Reader::pop( uint64_t len )
{
  const auto pop_len = static_cast<uint64_t>( len > bytes_.size() ? bytes_.size() : len );

  if ( 0 == pop_len ) {
    return;
  }

  bytes_.erase( bytes_.begin(), bytes_.begin() + static_cast<int64_t>( pop_len ) );
  bytes_poped_ += pop_len;
}

uint64_t Reader::bytes_buffered() const
{
  return bytes_.size();
}

uint64_t Reader::bytes_popped() const
{
  return bytes_poped_;
}
