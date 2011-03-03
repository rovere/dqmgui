#include "classlib/utils/SearchPath.h"
#include <stdlib.h>
#include <iostream>

using namespace lat;

int
main(int, char **)
{
  SearchPath p(getenv("PATH"));
  for (SearchPath::iterator i = p.begin(), e = p.end(); i != e; ++i)
    std::cout << "'" << *i << "'\n";

  return 0;
}
