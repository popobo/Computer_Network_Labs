#include "wrapping_integers.hh"
#include <algorithm>
#include <cassert>

using namespace std;

static constexpr uint64_t TWO_POW_32 = static_cast<uint64_t>( 1 ) << 32;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32( static_cast<uint32_t>( n + static_cast<uint64_t>( zero_point.raw_value_ ) % ( TWO_POW_32 ) ) );
}

static uint64_t distance( uint64_t a, uint64_t b )
{
  return ( a > b ) ? ( a - b ) : ( b - a );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  const uint64_t times = checkpoint / TWO_POW_32;

  const uint64_t first_raw = zero_point.raw_value_ <= raw_value_
                               ? raw_value_ - zero_point.raw_value_
                               : raw_value_ + UINT32_MAX - zero_point.raw_value_ + 1;
  const uint64_t r = ( times + 1 ) * TWO_POW_32 + first_raw;
  const uint64_t m = times * TWO_POW_32 + first_raw;
  const uint64_t l = ( times - 1 ) * TWO_POW_32 + first_raw;

  const uint64_t rd = distance( r, checkpoint );
  const uint64_t md = distance( m, checkpoint );
  const uint64_t ld = distance( l, checkpoint );

  // Get the minimum distance and its corresponding value
  const uint64_t min_distance = std::min( { rd, md, ld } );

  if ( min_distance == rd ) {
    return r;
  }
  if ( min_distance == md ) {
    return m;
  }
  return l;
}
