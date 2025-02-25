#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(first_index- poped >= output_.capacity_()){
    return;
  }
  buffer_pairs_.push_back(make_pair(first_index,data));

  sort(buffer_pairs_.begin(), buffer_pairs_.end(), [](const pair<int,string>& a, const pair<int, string>& b) {
        return a.first < b.first; // 如果要降序排序，则改为 return a.first > b.first;
    });

  for(auto it=buffer_pairs_.begin();it!=buffer_pairs_.end();it++){
     if(it->first ==output_.stream.size()+ poped){
       output_.write().push(it->second);
       buffer_pairs_.erase(it);
     }else break;
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "unimplemented count_bytes_pending() called" );
  return buffer_pairs_.size();
}
