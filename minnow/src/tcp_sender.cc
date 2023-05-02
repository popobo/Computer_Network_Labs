#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <cassert>
#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  uint64_t res = 0;
  for(const auto& msg: se_msgs_)
  {
    res += msg.sequence_length();
  }
  return res;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here. 
  return {};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  auto op_msg = se_msgs_.empty() ? optional<TCPSenderMessage>{} : optional<TCPSenderMessage>{ se_msgs_.front() };
  return op_msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  auto window_size = cur_rec_msg_.window_size == 0 ? 1 : cur_rec_msg_.window_size;

  auto num_of_max = window_size / TCPConfig::MAX_PAYLOAD_SIZE;
  auto win_remain = window_size % TCPConfig::MAX_PAYLOAD_SIZE;

  auto peek_str = outbound_stream.peek();
  size_t peek_str_size = peek_str.size();

  // bytes poped this time
  size_t i = 0;
  for ( ; i < num_of_max + 1; ++i ) {
    auto buf_size = TCPConfig::MAX_PAYLOAD_SIZE;
    if (i == num_of_max)
    {
      buf_size = win_remain;
    }

    if (num_seq_has_sent_ > outbound_stream.bytes_popped())
    {
      break;
    }

    auto SYN = outbound_stream.bytes_popped() == 0 && i == 0;
    buf_size = buf_size - SYN;
    auto FIN = outbound_stream.is_finished() && buf_size > outbound_stream.bytes_buffered();

    auto buf = peek_str.substr( i * TCPConfig::MAX_PAYLOAD_SIZE, buf_size );
    auto seqno = Wrap32::wrap( outbound_stream.bytes_popped(), isn_ );
    outbound_stream.pop(buf_size);
    
    se_msgs_.push_back( { seqno, SYN, { buf }, FIN } );
    
    num_seq_has_sent_ = outbound_stream.bytes_popped() + buf_size + SYN + FIN;

    peek_str_size -= buf_size;
    
    if (peek_str_size <= buf_size) {
      break;
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage empty_msg;
  empty_msg.seqno = se_msgs_.empty() ? Wrap32{0} : se_msgs_.back().seqno + se_msgs_.back().sequence_length();
  empty_msg.payload = Buffer{""};
  return empty_msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  cur_rec_msg_ = msg;
  auto ab_ackno = msg.ackno->unwrap(isn_, current_ackno_);
  for (auto it = se_msgs_.begin(); it != se_msgs_.end();)
  {
    auto ab_seqno = it->seqno.unwrap(isn_, current_ackno_);
    auto ab_end_seqno = ab_seqno + it->sequence_length() - 1;
    if (ab_end_seqno < ab_ackno)
    {
      it = se_msgs_.erase(it);
      continue;
    }
    ++it;
  }
  current_ackno_ = ab_ackno;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  // Your code here.
  (void)ms_since_last_tick;
}
