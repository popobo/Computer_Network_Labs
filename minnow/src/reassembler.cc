#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t fi, std::string data, bool is_last_substring, Writer& output )
{
  if ( data.empty() ) {
    if ( is_last_substring ) {
      output.close();
    }
    return;
  }

  if ( output.is_closed() ) {
    return;
  }

  // Find first unacceptable index
  uint64_t f_uacp_i = output.bytes_pushed() + output.available_capacity();
  // Last index of data
  uint64_t lid = fi + data.size() - 1;

  // Discard any bytes that lie beyond the stream's available capacity or are already assembled
  if ( lid < output.bytes_pushed() || fi > f_uacp_i) {
    return;
  }

  if ( lid >= f_uacp_i ) {
    
    data.resize( f_uacp_i - fi );
    lid = f_uacp_i - 1;
  }

  if ( fi < output.bytes_pushed() ) {
    data = data.substr( output.bytes_pushed() - fi );
    fi = output.bytes_pushed();
  }

  if ( data.empty() ) {
    return;
  }

  for ( auto it = segments_.begin(); it != segments_.end(); ) {
    uint64_t it_fi = it->fi;
    uint64_t it_lid = it_fi + it->data.size() - 1;

    if ( fi <= it_fi && it_lid <= lid ) {
      it = segments_.erase( it );
    } else {
      it++;
    }
  }

  // Merge data to existing segments or create a new segment
  bool merged = false;
  for ( auto it = segments_.begin(); it != segments_.end(); ) {
    uint64_t it_fi = it->fi;
    uint64_t it_lid = it_fi + it->data.size() - 1;

    if ( it_lid < fi ) {
      ++it;
    } else if ( lid < it_fi ) {
      segments_.insert( it, { fi, is_last_substring, std::move( data ) } );
      merged = true;
      break;
    } else {
      if ( fi < it_fi ) {
        if ( it == segments_.begin() ) {
          it->data.insert( 0, data.substr( 0, it_fi - fi ) );
          it->fi = fi;
        } else {
          auto prev_it = prev( it );
          auto prev_it_lid = prev_it->fi + prev_it->data.size() - 1;
          if ( prev_it_lid < fi ) {
            it->data.insert( 0, data.substr( 0, it_fi - fi ) );
            it->fi = fi;
          } else {
            it->data.insert( 0, data.substr( prev_it_lid - fi + 1, it_fi - prev_it_lid - 1 ) );
            it->fi = prev_it_lid + 1;
          }
        }
      }
      if ( lid > it_lid ) {
        if ( it == segments_.end() ) {
          it->data.append( data.substr( it_lid - fi + 1 ) );
        } else {
          auto next_it = next( it );
          if ( lid < next_it->fi ) {
            it->data.append( data.substr( it_lid - fi + 1 ) );
          } else {
            it->data.append( data.substr( it_lid - fi + 1, next_it->fi - it_lid - 1 ) );
          }
        }
      }
      it->is_last_substring |= is_last_substring;
      merged = true;
      break;
    }
  }

  if ( !merged ) {
    segments_.push_back( { fi, is_last_substring, std::move( data ) } );
  }

  // Write the assembled data to output
  while ( !segments_.empty() && segments_.front().fi == output.bytes_pushed() ) {
    auto& front_segment = segments_.front();
    output.push( std::move( front_segment.data ) );
    if ( front_segment.is_last_substring ) {
      output.close();
    }
    segments_.pop_front();
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t bytes_pending_num = 0;
  for ( auto it = segments_.begin(); it != segments_.end(); ++it ) {
    bytes_pending_num += it->data.size();
  }
  return bytes_pending_num;
}
