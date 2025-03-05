#include "tcp_receiver.hh"
#include "debug.hh"
#include <cstdint>
#include <iostream>
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  // Your code here.
  if ( message.RST ) {
    reader().set_error();
  }
  if ( message.SYN ) {
    syn = 1;
    zero.set_value( message.seqno.get_value() );
    reassembler_.insert(
      message.seqno.unwrap( zero, reassembler_.writer().bytes_pushed() ), message.payload, message.FIN );
    if ( message.FIN )
      fin = 1;
    return;
  }
  if ( syn ) {
    if ( message.FIN )
      fin = 1;
    reassembler_.insert(
      message.seqno.unwrap( zero, reassembler_.writer().bytes_pushed() ) - 1, message.payload, message.FIN );
    cout << message.seqno.unwrap( zero, reassembler_.writer().bytes_pushed() ) << " "
         << reassembler_.writer().bytes_pushed() << endl;
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  // Your code here.
  TCPReceiverMessage msg {};
  if ( reader().has_error() )
    msg.RST = true;
  if ( syn ) {
    msg.ackno = zero.wrap( reassembler_.writer().bytes_pushed(), zero ) + syn;
    if ( reassembler_.count_bytes_pending() == 0 )
      msg.ackno = zero.wrap( reassembler_.writer().bytes_pushed(), zero ) + syn + fin;
  }
  if ( reassembler_.writer().available_capacity() <= UINT16_MAX )
    msg.window_size = reassembler_.writer().available_capacity();
  else
    msg.window_size = UINT16_MAX;
  return msg;
}
