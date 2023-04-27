#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
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
  uint64_t l_i_d = first_index + data.size() - 1;
  // discard any bytes that lie beyond the stream's available capacity
  // or are already assembled
  if (l_i_d < f_uasm_i_)
  {
    return;
  }

  if (l_i_d >= f_uacp_i)
  {
    data.resize(f_uacp_i - first_index);
    l_i_d = f_uacp_i - 1;
  }

  if (first_index < f_uasm_i_)
  {
    data = data.substr(f_uasm_i_ - first_index);
    first_index = f_uasm_i_;
  }

  if (data.empty())
  {
    return;
  }

  if (f_uasm_i_ == first_index)
  {
    bool need_close = is_last_substring;
    
    f_uasm_i_ += data.size();
    output.push(std::move(data));
    
    for (auto it = segments_.begin(); it != segments_.end();)
    {
      if (f_uasm_i_ != it->first_index)
      {
        break;
      }

      f_uasm_i_ += it->data.size();
      output.push(std::move(it->data));
      need_close = it->is_last_substring;

      it = segments_.erase(it);
    }
    
    if (need_close)
    {
      output.close();
    }
    return;
  }
  
  auto it = segments_.begin();
  for (; it != segments_.end(); ++it)
  {
    // deal with the overlapped substring
    uint64_t it_l_i_d = it->first_index + it->data.size() - 1;
    if (it->first_index <= l_i_d &&
        l_i_d <= it_l_i_d)
    {
      if (it->first_index <= first_index)
      {
        break;
      }
      data.resize(it->first_index - first_index);
      segments_.insert(it, {first_index, is_last_substring, std::move(data)});
      break;
    }

    if (l_i_d < it->first_index)
    {
      segments_.insert(it, {first_index, is_last_substring, std::move(data)});
      break;
    }
  }

  if (segments_.end() == it)
  {
    segments_.push_back({first_index, is_last_substring, std::move(data)});
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
