#include "reassembler.hh"
#include "debug.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( is_last_substring ) {
    pos = first_index + data.size();
  }
  if ( output_.writer().available_capacity() + output_.writer().bytes_pushed() <= first_index ) {
    return;
  }

  if ( data.empty() ) {
    if ( is_last_substring )
      if ( output_.writer().bytes_pushed() == pos )
        output_.writer().close();
  }

  if ( first_index < output_.writer().bytes_pushed() ) {
    size_t tem = output_.writer().bytes_pushed() - first_index;
    if ( tem >= data.size() ) {
      if ( is_last_substring )
        output_.writer().close();
      return;
    }
    if ( tem > 0 ) {
      data = data.substr( tem );
      first_index += tem;
    }
  }
  size_t n = first_index + data.size() - output_.writer().bytes_pushed() - output_.writer().available_capacity();
  if ( n < data.size() && n > 0 ) {
    data = data.substr( 0, data.size() - n );
  }
  if ( !buffer.empty() ) {
    for ( size_t i = 1; i <= buffer.size(); ++i ) {
      if ( first_index < buffer[i - 1].first + buffer[i - 1].second.size() ) {
        if ( first_index + data.size() <= buffer[i - 1].first ) {
          auto it = buffer.begin() + i - 1;
          buffer.insert( it, make_pair( first_index, data ) );
          break;
        } else {
          if ( first_index <= buffer[i - 1].first ) {
            string s = {};
            size_t t = first_index + data.size();
            if ( t <= buffer[i - 1].first + buffer[i - 1].second.size() )
              s = data + buffer[i - 1].second.substr( t - buffer[i - 1].first );
            else
              s = data;
            buffer[i - 1].second = s;
            buffer[i - 1].first = first_index;
          } else {
            string s = {};
            size_t t = buffer[i - 1].first + buffer[i - 1].second.size();
            if ( t <= first_index + data.size() )
              s = buffer[i - 1].second + data.substr( t - first_index );
            else
              s = buffer[i - 1].second;
            buffer[i - 1].second = s;
          }
          break;
        } //??????
      }
      if ( i == buffer.size() ) {
        auto it = buffer.end();
        buffer.insert( it, make_pair( first_index, data ) );
      }
    }
  } else {
    buffer.push_back( make_pair( first_index, data ) );
  }

  for ( size_t i = 2; i <= buffer.size(); ++i ) {
    if ( buffer[i - 1].first < buffer[i - 2].first + buffer[i - 2].second.size() ) {
      //??
      if ( buffer[i - 1].first + buffer[i - 1].second.size() > buffer[i - 2].first + buffer[i - 2].second.size() ) {
        buffer[i - 2].second = buffer[i - 2].second
                               + buffer[i - 1].second.substr( buffer[i - 2].first + buffer[i - 2].second.size()
                                                              - buffer[i - 1].first );
        buffer.erase( buffer.begin() + i - 1 );
        --i;
      } else {
        buffer.erase( buffer.begin() + i - 1 );
        --i;
      }
    }
  }

  while ( !buffer.empty() && buffer[0].first == output_.writer().bytes_pushed() ) {
    output_.writer().push( buffer[0].second );
    if ( buffer[0].first + buffer[0].second.size() == pos )
      output_.writer().close();
    buffer.erase( buffer.begin() );
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  debug( "unimplemented count_bytes_pending() called" );
  size_t total = 0;
  for ( auto& it : buffer ) {
    total += it.second.size();
  }
  return total;
}