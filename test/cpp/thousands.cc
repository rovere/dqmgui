#include "classlib/utils/Regexp.h"
#include "classlib/utils/StringOps.h"
#include <iostream>

using namespace lat;
static Regexp RX_THOUSANDS("(\\d)(\\d{3}($|\\D))");
static std::string
thousands(const std::string &arg)
{
  std::string result;
  std::string tmp(arg);
  result.reserve(arg.size() + arg.size()/3 + 1);

  while (true)
  {
    result = StringOps::replace(tmp, RX_THOUSANDS, "\\1'\\2");
    if (result == tmp)
      break;

    tmp = result;
  }

  return result;
}

int
main(int argc, char **argv)
{
  for (int i = 1; i < argc; ++i)
    std::cout << "thousands('" << argv[i] << "') = <" << thousands(argv[i]) << ">\n";

  return 0;
}
