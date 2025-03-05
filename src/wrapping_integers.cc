#include "wrapping_integers.hh"
#include "debug.hh"
#include <algorithm>
#include <climits>
#include <cstdint>

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  // debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  uint64_t k = n % ( 1UL << 32 );
  k = k + uint64_t( zero_point.raw_value_ );
  k = k % ( 1UL << 32 );
  return Wrap32( uint32_t( k ) );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  // debug( "unimplemented unwrap( {}, {} ,{}) called", zero_point.raw_value_, checkpoint ,raw_value_);
  uint32_t n = raw_value_ - zero_point.raw_value_;
  uint64_t s = uint64_t( n );
  size_t i = 63;
  while ( s < checkpoint && s < ( UINT64_MAX - UINT32_MAX - 1 ) ) {
    if ( i == 31 ) {
      break;
    }
    if ( ( 1UL << i ) <= ( checkpoint - s ) )
      s += ( 1UL << i );
    else
      --i;
  }

  i = 63;
  while ( s > checkpoint && s >= ( 1UL << 32 ) ) {
    if ( i == 31 ) {
      break;
    }
    if ( ( 1UL << i ) <= ( UINT64_MAX - checkpoint ) && s >= checkpoint + ( 1UL << i ) )
      s -= ( 1UL << i );
    else
      --i;
  }

  if ( s == checkpoint )
    return s;
  if ( s > checkpoint ) {
    if ( s >= ( 1UL << 32 ) )
      return ( s - checkpoint ) < checkpoint - ( s - ( 1UL << 32 ) ) ? s : ( s - ( 1UL << 32 ) );
    else
      return s;
  } else {
    if ( s <= ( UINT64_MAX - UINT32_MAX - 1 ) )
      return ( checkpoint - s ) < ( s + ( 1UL << 32 ) - checkpoint ) ? s : ( s + ( 1UL << 32 ) );
    else
      return s;
  }
}
