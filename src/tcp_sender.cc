#include "tcp_sender.hh"
#include "debug.hh"
#include "tcp_config.hh"
#include <iostream>
using namespace std;

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::sequence_numbers_in_flight() const
{
  size_t sum = 0;
  for(auto & it: buffer){
    if(!it.second.ok){
      sum += (it.second.msg.sequence_length());
    }
  }
  return sum;
}

// This function is for testing only; don't add extra state to support it.
uint64_t TCPSender::consecutive_retransmissions() const
{
  size_t sum =0;
  for(auto & it: buffer){
    if(!it.second.ok)
      sum +=it.second.resend;
  }
  return sum;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  while(true){
    TCPSenderMessage sm;
    uint32_t t = (isn_.get_value() + bytes_send) % (1UL << 32);
    sm.seqno.set_value(t);
  	if(!start){
    	sm.SYN = true;
		start = true;
    	++bytes_send;
    	timer.pos = isn_.get_value();
  	}
      size_t maxs = max_cap - bytes_send;
      read(input_.reader(),min(maxs,TCPConfig::MAX_PAYLOAD_SIZE),sm.payload);
      if(reader().is_finished() && end == false && sm.payload.size() < maxs){
        sm.FIN =true;
        ++bytes_send;
        end = true;
      }
      if(reader().has_error())  sm.RST = true;
	  if(!sm.payload.empty()){
        bytes_send +=sm.payload.size();
        msgQueue.push_back(sm);
        buffer.emplace(t,Segment(sm));
        if(is_win_zero !=0 ){
        if(sm.seqno.get_value() == is_win_zero){
          buffer[is_win_zero].is_win_zero = true;  is_win_zero =0;
        }
        }
	  }
      else if(sm.payload.empty() && (sm.FIN||sm.SYN)){
        msgQueue.push_back(sm);
        buffer.emplace(t,Segment(sm));
        if(is_win_zero !=0 ){
        if(sm.seqno.get_value() == is_win_zero){
          buffer[is_win_zero].is_win_zero = true;  is_win_zero =0;
        }
        }
      }
      else break;
      if(find(ackno_pos.begin(),ackno_pos.end(),t) != ackno_pos.end()){
        if(t >= timer.pos)
          timer.reset(t);
      }
  }
  while(!msgQueue.empty()){
    transmit(msgQueue.front());
    msgQueue.pop_front();
	debug("出队一个消息");
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  debug( "unimplemented make_empty_message() called" );
  TCPSenderMessage sm;
  if(!start)
  	sm.SYN = true;
  if(reader().has_error())  sm.RST = true;
  sm.seqno.set_value(isn_.get_value() + start + reader().bytes_popped() + end);
  return sm;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  uint32_t t = 0;
  uint16_t win = msg.window_size;
  if(msg.ackno.has_value())
    t = msg.ackno.value().get_value();
  else t = isn_.get_value();
  if(win ==  0){
    is_win_zero = t;
    if(!msg.ackno.has_value())  {reader().set_error();  return;}
  }
  if(reader().bytes_buffered() + bytes_send < Wrap32(t).unwrap(isn_,bytes_send))
    return;
  if(find(ackno_pos.begin(),ackno_pos.end(),t) == ackno_pos.end()){
    if(ackno_pos.empty() || t > ackno_pos.back())  //
      ackno_pos.push_back(t);
  }
  if(find(ackno_pos.begin(),ackno_pos.end(),t) != ackno_pos.end()){
     auto it = buffer.find(t);
     if(it != buffer.end() && timer.pos < t){     //
       timer.reset(t);
     }
     else if(it ==buffer.end() && t == isn_.get_value() + bytes_send){
       timer.finished =true;
     }
  }
  max_cap = max(max_cap,Wrap32(t).unwrap(isn_,bytes_send)+uint64_t(max(uint16_t(1),win)));
  for(auto &it: buffer){
    if((it.second.msg.payload.size() + it.second.msg.SYN + it.second.msg.FIN <= Wrap32(t).unwrap(Wrap32(it.first),bytes_send)) && it.second.ok == false){
      debug("收到一条消息");
      it.second.ok = true;
      if(win!=0)
        time_tw = initial_RTO_ms_;
	}
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  debug("tick called!");
  if(buffer.find(timer.pos) == buffer.end() || timer.finished){
    return;
  }
  timer.time +=ms_since_last_tick;
  if(timer.time >= time_tw){
    debug("超时重传！");
    transmit(buffer[timer.pos].msg);
    ++buffer[timer.pos].resend;
    timer.time = 0;
    if(!buffer[timer.pos].is_win_zero)
      time_tw *= 2;
  }
}
