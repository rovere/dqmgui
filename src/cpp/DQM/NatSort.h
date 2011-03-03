#ifndef DQM_NATSORT_H
# define DQM_NATSORT_H

# include "DQM/StringAtom.h"
# include <string>
# include <cctype>
# include <stdint.h>

/** Compare two strings and return @c true if the first one is less
    than the second one.  This is intended to be used as a comparison
    function in std::sort().

    The comparison is done "naturally", sorting number sequences
    numerically and text sequences in dictionary order, rather than
    the normal lexicographical sort.  */
inline bool
natcmp(const char *a, const char *b)
{
  // Scan both strings until we reach the end of one of them.
  while (*a && *b)
  {
    bool anum = isdigit(*a) || ((*a == '-' || *a == '+') && isdigit(a[1]));
    bool bnum = isdigit(*b) || ((*b == '-' || *b == '+') && isdigit(b[1]));

    // Sort numbers before text.
    if (anum != bnum)
      return anum;

    // Sort as numbers
    if (anum && bnum)
    {
      int64_t avalue = strtoll(a, (char **) &a, 10);
      int64_t bvalue = strtoll(b, (char **) &b, 10);
      if (avalue != bvalue)
	return avalue < bvalue;

      // Already at next character.
      continue;
    }

    // Sort as text
    if (*a != *b)
      return *a < *b;

    // Equal, proceed to next character.
    ++a;
    ++b;
  }

  // Of two equal strings, the shorter one comes first.
  return *a != *b && *a == 0;
}

/** Compare two strings and return @c true if the first one is less
    than the second one.  This is intended to be used as a comparison
    function in std::sort().

    The comparison is done "naturally", sorting number sequences
    numerically and text sequences in dictionary order, rather than
    the normal lexicographical sort.  */
inline bool
natcmpstr(const std::string &a, const std::string &b)
{ return natcmp(a.c_str(), b.c_str()); }

/** Compare two strings and return @c true if the first one is less
    than the second one.  This is intended to be used as a comparison
    function in std::sort().

    The comparison is done "naturally", sorting number sequences
    numerically and text sequences in dictionary order, rather than
    the normal lexicographical sort.  */
inline bool
natcmpstratom(const StringAtom &a, const StringAtom &b)
{ return natcmpstr(a.string(), b.string()); }

#endif // DQM_LINEBUFFER_H
