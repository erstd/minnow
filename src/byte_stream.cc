#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{

  uint64_t len = data.size();
  uint64_t a = available_capacity();
  if(a>=len){
    stream.append(data);
    pushed_capacity_ += len;
  }else{
    stream.append(data.substr(0,a));
    pushed_capacity_ += a;
  }
}

void Writer::close()
{
  // Your code here.
  write_closed = true;
}

bool Writer::is_closed() const
{
  return write_closed; // Your code here.
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - uint64_t(stream.size()); // Your code here.
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_capacity_; // Your code here.
}

string_view Reader::peek() const
{
  string_view str;
  if(!stream.empty()){
    str = stream;
    return str;
  }else{
    return str;
  }
}

void Reader::pop( uint64_t len )
{
  if(len<=stream.size()){
    stream.erase(0,len);
    poped = poped + len;
  }
}

bool Reader::is_finished() const
{
  if(write_closed){
    return {peek().empty() ? true: false
    }; // Your code here.
  }else{
    return false;
  }
}

uint64_t Reader::bytes_buffered() const
{
  return stream.size(); // Your code here.
}

uint64_t Reader::bytes_popped() const
{
  return poped; // Your code here.
}



