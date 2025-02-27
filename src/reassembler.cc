#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(output_.writer().available_capacity() + output_.writer().bytes_pushed() <= first_index){
    return;
  }
  if(!data.empty()){
      if(first_index < output_.writer().bytes_pushed()){
        size_t tem = output_.writer().bytes_pushed() - first_index;
        if(tem >= data.size())
            {if(is_last_substring)  output_.writer().close();   return;}
        data = data.substr(tem);
        first_index += tem;
      }
      size_t n = first_index + data.size()- output_.writer().bytes_pushed() - output_.writer().available_capacity();
        if( n > 0){
            data = data.substr(0,data.size()-n);
        }
      if(!buffer.empty()){
          for(size_t i=1;i <= buffer.size();++i){
            if(first_index < buffer[i-1].first + buffer[i-1].second.size()){
                if(first_index + data.size()<=buffer[i-1].first){
                    auto it = buffer.begin() + i-1;
                    buffer.insert(it,make_pair(first_index,data));
                    break;
                }else{
                    if(first_index <= buffer[i-1].first){
                        string s = data + buffer[i-1].second.substr(first_index + data.size()-buffer[i-1].first);
                        buffer[i-1].second = s;
                        buffer[i-1].first = first_index;
                    } else {
                        string s = data.substr(buffer[i-1].first + buffer[i-1].second.size()-first_index) + buffer[i-1].second;
                        buffer[i-1].second = s;
                    }
                    break;
                }   //??????
            } if(i==buffer.size()){
                 auto it = buffer.end();
                    buffer.insert(it,make_pair(first_index,data));
            }   
          } 
      }else{
            buffer.push_back(make_pair(first_index,data));
        }
      while(buffer[0].first == output_.writer().bytes_pushed()){
        output_.writer().push(buffer[0].second);
        buffer.erase(buffer.begin());
      }
  }
  if(is_last_substring){
        output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "unimplemented count_bytes_pending() called" );
  return buffer.size();
}