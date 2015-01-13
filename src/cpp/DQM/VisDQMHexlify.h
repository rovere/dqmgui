#ifndef DQM_VISDQMHEXLIFY_H
#define DQM_VISDQMHEXLIFY_H

/** Utility function to print in HEX format an arbitrary string. */
template<class T >
static std::string hexlify (const T & x)
{
  std::string result;
  result.reserve(2*x.size());
  for (size_t i = 0, e = x.size(); i != e; ++i)
  {
    char buf[3];
    sprintf(buf, "%02x", (unsigned) (unsigned char) x[i]);
    result += buf[0];
    result += buf[1];
  }
  return result;
}


#endif  // DQM_VISDQMHEXLIFY_H
