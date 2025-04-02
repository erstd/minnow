#include "router.hh"
#include "debug.hh"

#include <iostream>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";
  rout_tb.push_back( rout { route_prefix, prefix_length, next_hop, interface_num } );
}

uint32_t pow( uint32_t a, int b )
{
  if ( b == 0 )
    return 1;
  uint32_t tmp = a;
  for ( int i = 1; i <= b - 1; ++i ) {
    tmp *= a;
  }
  return tmp;
}

// Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
void Router::route()
{
  for ( auto& itter : interfaces_ ) {
    while ( !itter->datagrams_received().empty() ) {
      auto& it = itter->datagrams_received().front();
      Match match;
      for ( auto& itt : rout_tb ) {
        if ( itt.prefix_length >= match.len ) {
          if ( itt.prefix_length == 0 ) {
            match.is_match = true;
            match.len = 0;
            match.dst = move( itt );
          } else if ( itt.prefix_length == 32 ) {
            if ( it.header.dst == itt.route_prefix ) {
              match.len = itt.prefix_length;
              match.dst = move( itt );
              match.is_match = true;
            }
          } else {
            if ( it.header.dst / pow( 2, 32 - itt.prefix_length )
                 == itt.route_prefix / pow( 2, 32 - itt.prefix_length ) ) {
              match.len = itt.prefix_length;
              match.dst = move( itt );
              match.is_match = true;
            }
          }
        }
      }
      if ( it.header.ttl <= 1 || match.is_match == false ) {
        itter->datagrams_received().pop();
        continue;
      }
      --it.header.ttl;
      if ( match.dst.next_hop.has_value() )
        interfaces_[match.dst.interface_num]->send_datagram( it, match.dst.next_hop.value() );
      else {
        interfaces_[match.dst.interface_num]->send_datagram( it, Address::from_ipv4_numeric( it.header.dst ) );
      }
      itter->datagrams_received().pop();
    }
  }
}
