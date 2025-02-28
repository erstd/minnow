#include "wrapping_integers.hh"
#include "debug.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  debug( "unimplemented wrap( {}, {} ) called", n, zero_point.raw_value_ );
  return Wrap32 {  uint32_t((n + uint64_t(zero_point.raw_value_))%(uint64_t(UINT32_MAX) + 1)) };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  debug( "unimplemented unwrap( {}, {} ) called", zero_point.raw_value_, checkpoint );
  uint64_t n = uint64_t(raw_value_ - zero_point.raw_value_ + (UINT32_MAX + 1));
  while(n<checkpoint){
    n+= (UINT32_MAX + 1);
  }
  if(n>=(UINT32_MAX + 1))
  return (n - checkpoint) < checkpoint-(n - (UINT32_MAX + 1)) ? n:(n - (UINT32_MAX + 1));
  else return n;
}
