#include "reassembler.hh"

using namespace std;

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
  
  auto it = segments_.begin();
  for (; it != segments_.end();)
  {
    // deal with the overlapped substring
    uint64_t it_fi = it->fi;
    uint64_t it_lid = it_fi + it->data.size() - 1;
    if (it_fi <= lid &&
        lid <= it_lid)
    {
      if (it_fi <= fi)
      {
        break;
      }
      data.resize(it_fi - fi);
      segments_.insert(it, {fi, is_last_substring, std::move(data)});
      break;
    }

    if (fi <= it_fi && 
        it_lid <= lid)
    {
      it = segments_.erase(it);
      continue;
    }

    if (lid < it_fi)
    {
      segments_.insert(it, {fi, is_last_substring, std::move(data)});
      break;
    }

    if (fi <= it_lid &&
        it_lid < lid)
    {
      data = data.substr(it_lid - fi + 1);
      fi = it_lid + 1;
      segments_.insert(next(it), {fi, is_last_substring, std::move(data)});
      break;
    }

    ++it;
  }

  if (segments_.end() == it)
  {
    segments_.push_back({fi, is_last_substring, std::move(data)});
  }
  
  for (it = segments_.begin(); it != segments_.end();)
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
