#include "reassembler.hh"
#include <tuple>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <unistd.h>

using namespace std;

string prettify( string_view str, size_t max_length )
{
  ostringstream ss;
  const string_view str_prefix = str.substr( 0, max_length );
  for ( const uint8_t ch : str_prefix ) {
    if ( isprint( ch ) ) {
      ss << ch;
    } else {
      ss << "\\x" << fixed << setw( 2 ) << setfill( '0' ) << hex << static_cast<size_t>( ch );
    }
  }
  if ( str.size() > str_prefix.size() ) {
    ss << "...";
  }
  return ss.str();
}

void Reassembler::insert( uint64_t fi, string data, bool is_last_substring, Writer& output )
{
  if (data.empty())
  {
    if (is_last_substring)
    {
      output.close();
    }
    return;
  }

  //first unacceptable index
  uint64_t f_uacp_i = f_uasm_i_ + output.available_capacity();
  // last index of data
  uint64_t lid = fi + data.size() - 1;
  // discard any bytes that lie beyond the stream's available capacity
  // or are already assembled
  if (lid < f_uasm_i_)
  {
    return;
  }

  if (lid >= f_uacp_i)
  {
    data.resize(f_uacp_i - fi);
    lid = f_uacp_i - 1;
  }

  if (fi < f_uasm_i_)
  {
    data = data.substr(f_uasm_i_ - fi);
    fi = f_uasm_i_;
  }

  if (data.empty())
  {
    return;
  }
  
  auto fi_it = segments_.end();
  auto lid_it = segments_.end();
  for (auto it = segments_.begin(); it != segments_.end();)
  {
    // deal with the overlapped substring
    uint64_t it_fi = it->fi;
    uint64_t it_lid = it_fi + it->data.size() - 1;

    if (fi <= it_fi && it_lid <= lid)
    {
        it = segments_.erase(it);
        if (it == segments_.end() && fi_it == segments_.end()) {
          segments_.push_back({fi, is_last_substring, std::move(data)});
          break;
        }
        continue;
    }

    if (it == segments_.begin() && lid < it_fi)
    {
      segments_.push_front({fi, is_last_substring, std::move(data)});
      break;
    }

    auto n_it = next(it);
    // this segment is in the end
    if (n_it == segments_.end() && it_lid < fi)
    {
      segments_.push_back({fi, is_last_substring, std::move(data)});
      break;
    }
    else if (n_it != segments_.end() && 
             (it_lid < fi && lid < n_it->fi))
    {
      segments_.insert(n_it, {fi, is_last_substring, std::move(data)});
      break;
    }

    if (it_fi <= fi && fi <= it_lid)
    {
      fi_it = it;
    }

    if (it_fi <= lid && lid <= it_lid)
    {
      lid_it = it;
    }

    ++it;
  }

  if (fi_it != segments_.end() && fi_it == lid_it)
  {
    return;// this segment is inside a existing segment
  }

  auto new_fi = fi;
  tuple<size_t, size_t> new_range{0, 0};
  if (segments_.empty())
  {
    segments_.push_back({fi, is_last_substring, move(data)});
  }
  else if (fi_it == segments_.end() && lid_it != segments_.end()) 
  {
    new_fi = fi;
    new_range = {0, lid_it->fi - fi};
  }
  else if (fi_it != segments_.end() && lid_it == segments_.end())
  {
    auto fi_it_lid = fi_it->fi + fi_it->data.size() - 1;
    new_fi = fi_it_lid + 1;
    new_range = {fi_it_lid - fi + 1, data.size() - (fi_it_lid - fi + 1)};
  }
  else if (fi_it != segments_.end() && lid_it != segments_.end())
  {
    auto fi_it_lid = fi_it->fi + fi_it->data.size() - 1;
    new_fi = fi_it_lid + 1;
    new_range = {fi_it_lid - fi + 1, lid_it->fi - fi_it_lid - 1};
  }

  if (std::get<1>(new_range) > 0)
  {
    segments_.insert(lid_it, { new_fi, is_last_substring, string{ data, std::get<0>(new_range), std::get<1>(new_range) } });
  }

  cout << "---------------------" << endl;
  for (const auto& seg : segments_)
  {
    cout << "seg.fi:" << seg.fi << ", " << "seg.lid: " << seg.fi + seg.data.size() - 1 << ", data.size(): " << seg.data.size() << endl;
    // cout << prettify(seg.data, 2048) << endl;
  }
  cout << "********************" << endl;

  // cout << "%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
  // for (const auto& seg : segments_)
  // {
  //   cout << prettify(seg.data, 2048);
  // }
  // cout << "￥￥￥￥￥￥￥￥￥￥￥￥￥￥" << endl;

  for (auto it = segments_.begin(); it != segments_.end();)
  {
    if (it->fi != f_uasm_i_)
    {
      break;
    }

    f_uasm_i_ += it->data.size();
    output.push(std::move(it->data));
    if (it->is_last_substring)
    {
      output.close();
    }
    it = segments_.erase(it);
  }
}

uint64_t Reassembler::bytes_pending() const
{
  uint64_t bytes_pending_num = 0;
  for (auto it = segments_.begin(); it != segments_.end(); ++it)
  {
    bytes_pending_num += it->data.size();
  }
  return bytes_pending_num;
}
