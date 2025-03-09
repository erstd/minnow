#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"

using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  debug( "unimplemented sequence_numbers_in_flight() called" );
  return {};
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  debug( "unimplemented consecutive_retransmissions() called" );
  return {};
}

void TCPSender::push( const TransmitFunction& transmit )
{
  debug( "unimplemented push() called" );
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug( "unimplemented make_empty_message() called" );
  return {};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  debug( "unimplemented receive() called" );
  uint32_t t = msg.ackno.value().get_value();
  if(buffer.find(t) != buffer.end()){

  }else {
    TCPSenderMessage sm;
    sm.seqno.set_value(t);
    if(t == isn_.get_value()){
      sm.SYN = true;
    };
    read(input_.reader(),msg.window_size,sm.payload);
    if(t + msg.window_size() >=isn_.get_value() + input_.reader().bytes_buffered())
      sm.FIN = true;
    if(msg.RST()) 
      sm.RST = true;
    }
    msgQueue.push(sm);
    Segment seg(sm);
    buffer.insert(make_pair(t,seg));
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  debug( "unimplemented tick({}, ...) called", ms_since_last_tick );
  
}
