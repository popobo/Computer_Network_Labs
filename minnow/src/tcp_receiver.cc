#include "tcp_receiver.hh"
#include <algorithm>

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  if ( message.SYN ) {
    is_isn_set_ = true;
    ISN_ = message.seqno;
  }

  auto ab_fi = message.seqno.unwrap( ISN_, inbound_stream.bytes_pushed() ); // fi is a absolute seqno

  if ( !is_isn_set_ || (!message.SYN && ab_fi == 0)) {
    return;
  }

  if ( message.FIN ) {
    is_fin_set_ = true;
    FIN_ackno = ab_fi + message.sequence_length() - 1;
  }

  reassembler.insert( ab_fi > 0 ? ab_fi - 1 : 0, message.payload.release(), is_fin_set_, inbound_stream );
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  //  assert(inbound_stream.available_capacity() <= UINT16_MAX);

  auto ackno = std::optional<Wrap32> {};
  if ( is_isn_set_ ) {
    if ( is_fin_set_ && inbound_stream.bytes_pushed() == FIN_ackno - 1 ) {
      ackno = std::optional<Wrap32> { Wrap32::wrap( FIN_ackno + 1, ISN_ ) };
    } else {
      ackno = std::optional<Wrap32> { Wrap32::wrap( inbound_stream.bytes_pushed() + 1, ISN_ ) };
    }
  }

  return {
    ackno,
    static_cast<uint16_t>( min( inbound_stream.available_capacity(), static_cast<uint64_t>( UINT16_MAX ) ) ) };
}
