#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  (void)data; // Your code here.
  uint64_t len = data.size();
  if(available_capacity()>=len){
    stream.append(data);
    pushed_capacity_ += len;
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
  return capacity_ - pushed_capacity_; // Your code here.
}

uint64_t Writer::bytes_pushed() const
{
  return pushed_capacity_; // Your code here.
}

string_view Reader::peek() const
{
   if(!stream.empty()){
     string_view str(stream.substr(0,1));
     return str;
   }else{
     string_view str("Endddd!");
     return str;
   }
}

void Reader::pop( uint64_t len )
{
  (void)len; // Your code here.
  if(len<=stream.size()){
    stream.erase(0,len);
  }
}

bool Reader::is_finished() const
{
  if(write_closed){
    string_view str("Endddd!");
    return {peek()== str ? true: false
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
  return pushed_capacity_-stream.size(); // Your code here.
}

