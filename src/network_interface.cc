#include <iostream>

#include "arp_message.hh"
#include "debug.hh"
#include "ethernet_frame.hh"
#include "exception.hh"
#include "helpers.hh"
#include "network_interface.hh"

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( string_view name,
                                    shared_ptr<OutputPort> port,
                                    const EthernetAddress& ethernet_address,
                                    const Address& ip_address )
  : name_( name )
  , port_( notnull( "OutputPort", move( port ) ) )
  , ethernet_address_( ethernet_address )
  , ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but
//! may also be another host if directly connected to the same network as the destination) Note: the Address type
//! can be converted to a uint32_t (raw 32-bit IP address) by using the Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  uint32_t next_hop_ip = next_hop.ipv4_numeric();
  if ( cache_.find( next_hop_ip ) != cache_.end() && cache_[next_hop_ip].known == true ) { // 如果destination已知
    EthernetFrame the {};
    the.header.src = ethernet_address_;
    Serializer srl {};
    dgram.serialize( srl );
    the.payload = move( srl.finish() );
    the.header.dst = cache_[next_hop_ip].add;
    the.header.type = EthernetHeader::TYPE_IPv4;
    transmit( the );
    debug( "发送一个IPV4链路帧" );
  } else {
    if ( cache_.find( next_hop_ip ) == cache_.end() ) {
      cache_[next_hop_ip] = cached_the { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, 0, false };
      debug( "cache_ 添加一个未知的缓存" );
    } else if ( cache_.find( next_hop_ip ) != cache_.end() && cache_[next_hop_ip].known == false
                && cache_[next_hop_ip].time < 5000 ) {
      datagrams_to_send.push_back( dgram_to_send { dgram, next_hop_ip } );
      debug( "datagrams_to_send 入队一个元素" );
      return;
    }
    EthernetFrame the {};
    the.header.src = ethernet_address_;
    cache_[next_hop_ip].time = 0;
    the.header.type = EthernetHeader::TYPE_ARP;
    the.header.dst = ETHERNET_BROADCAST;
    ARPMessage rmsg;
    rmsg.opcode = ARPMessage::OPCODE_REQUEST;
    rmsg.sender_ethernet_address = ethernet_address_;
    rmsg.sender_ip_address = ip_address_.ipv4_numeric();
    rmsg.target_ip_address = next_hop_ip;
    rmsg.target_ethernet_address = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    Serializer srl {};
    rmsg.serialize( srl );
    the.payload = move( srl.finish() );
    debug( "发送一个ARP请求" );
    transmit( the );
    datagrams_to_send.push_back( dgram_to_send { dgram, next_hop_ip } );
    debug( "datagrams_to_send 入队一个元素" );
  }
}

//! \param[in] frame the incoming Ethernet frame
void NetworkInterface::recv_frame( EthernetFrame frame )
{
  if ( frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST )
    return;
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram gram;
    Parser par( frame.payload );
    gram.parse( par );
    datagrams_received_.push( move( gram ) );
    debug( "接受一个ipv4" );
  } else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    debug( "接受一个ARP" );
    ARPMessage msg;
    Parser par( frame.payload );
    msg.parse( par );
    cout << msg.sender_ip_address << '\n';
    if ( cache_.find( msg.sender_ip_address ) == cache_.end() ) {
      cache_.emplace( msg.sender_ip_address, cached_the { msg.sender_ethernet_address, 0, true } );
      debug( "缓存一个新的映射" );
    } else {
      cache_[msg.sender_ip_address].time = 0;
      cache_[msg.sender_ip_address].add = msg.sender_ethernet_address;
      cache_[msg.sender_ip_address].known = true;
      debug( "一个已缓存的映射更新/响应" );
    }
    if ( msg.opcode == ARPMessage::OPCODE_REQUEST ) {
      if ( ip_address_.ipv4_numeric() == msg.target_ip_address ) {
        ARPMessage rmsg;
        rmsg.opcode = ARPMessage::OPCODE_REPLY;
        rmsg.sender_ethernet_address = ethernet_address_;
        rmsg.sender_ip_address = ip_address_.ipv4_numeric();
        rmsg.target_ip_address = msg.sender_ip_address;
        rmsg.target_ethernet_address = msg.sender_ethernet_address;
        EthernetFrame the {};
        the.header.src = ethernet_address_;
        the.header.type = EthernetHeader::TYPE_ARP;
        the.header.dst = rmsg.target_ethernet_address;
        Serializer srl2 {};
        rmsg.serialize( srl2 );
        the.payload = move( srl2.finish() );
        transmit( the );
        debug( "回复一个ARP" );
      }
    }
    debug( "扫描消息队列" );
    for ( auto it = datagrams_to_send.begin(); it != datagrams_to_send.end(); ) {
      if ( cache_[it->next_hop].known ) {
        debug( "datagrams_to_send 队列发送一个报文" );
        send_datagram( it->dgram, ip_address_.from_ipv4_numeric( it->next_hop ) );
        it = datagrams_to_send.erase( it );
      } else
        ++it;
    }
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for ( auto it = cache_.begin(); it != cache_.end(); ) {
    it->second.time += ms_since_last_tick;
    if ( it->second.known ) {
      if ( it->second.time >= 30000 ) {
        it = cache_.erase( it );
        continue;
      }
    }
    if ( !it->second.known ) {
      if ( it->second.time >= 5000 ) {
        for ( auto sl = datagrams_to_send.begin(); sl != datagrams_to_send.end(); ) {
          if ( sl->next_hop == it->first ) {
            sl = datagrams_to_send.erase( sl );
          }
        }
        it = cache_.erase( it );
        continue;
      }
    }
    ++it;
  }
}
