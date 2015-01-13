#ifndef DQM_VISDQMUNHEXLIFY_H
#define DQM_VISDQMUNHEXLIFY_H

static int to_int(int c)
{
  if (isdigit(c))
    return c - '0';
  else
  {
    if (isupper(c))
      c = tolower(c);
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
  }
  return -1;
}

/** Utility function to recover the original string, before converion
    to HEX notation. */
static std::string unhexlify(const std::string& s)
{
  size_t len = s.size();
  assert(len > 0);
  assert(len % 2 == 0);
  std::string result;
  result.reserve(len/2);

  size_t i=0;
  for (; i < len; i += 2)
  {
    int top = to_int((unsigned char)(s[i] & 0xff));
    int bot = to_int((unsigned char)(s[i+1] & 0xff));
    result += (top << 4) + bot;
  }
  return result;
}

#endif  // DQM_VISDQMUNHEXLIFY_H
