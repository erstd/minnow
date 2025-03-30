#pragma once
#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <queue>

class Timer
{
public:
  uint32_t pos = 0;
  size_t time = 0;
  void reset( uint32_t t )
  {
    time = 0;
    pos = t;
    finished = false;
  }
  bool finished = false;
};

class Segment
{
public:
  Segment() {};
  Segment( TCPSenderMessage& m )
  {
    msg.seqno.set_value( m.seqno.get_value() );
    msg.SYN = m.SYN;
    msg.payload = m.payload;
    msg.FIN = m.FIN;
    msg.RST = m.RST;
  };
  TCPSenderMessage msg {};
  size_t resend = 0;
  // size_t time = 0;
  bool ok = false; // 次段是否传输完成
  // bool timer = false;  //定时器是否启动
  bool is_win_zero = false;
};

class TCPSender
{
public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( ByteStream&& input, Wrap32 isn, uint64_t initial_RTO_ms )
    : input_( std::move( input ) ), isn_( isn ), initial_RTO_ms_( initial_RTO_ms )
  {
    time_tw = initial_RTO_ms_;
  }

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage make_empty_message() const;

  /* Receive and process a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Type of the `transmit` function that the push and tick methods can use to send messages */
  using TransmitFunction = std::function<void( const TCPSenderMessage& )>;

  /* Push bytes from the outbound stream */
  void push( const TransmitFunction& transmit );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called */
  void tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit );

  // Accessors
  uint64_t sequence_numbers_in_flight() const;  // For testing: how many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // For testing: how many consecutive retransmissions have happened?
  const Writer& writer() const { return input_.writer(); }
  const Reader& reader() const { return input_.reader(); }
  Writer& writer() { return input_.writer(); }

private:
  Reader& reader() { return input_.reader(); }
  ByteStream input_;
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;
  uint64_t time_tw = 0;
  std::deque<TCPSenderMessage> msgQueue {};
  std::vector<uint32_t> ackno_pos {};
  size_t bytes_send = 0;
  size_t max_cap = 1; // 最多可以push的bytes,包含ISN
  // uint64_t max_ackno = 0;  //当前定时器追踪的segment
  std::map<uint32_t, Segment> buffer {};
  bool start = false;
  bool end = false;
  uint32_t is_win_zero = 0;
  Timer timer {};
};
