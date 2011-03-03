#include "DQM/VisDQMBuf.h"
#include <new>

void
VisDQMBuf::die(void)
{ throw std::bad_alloc(); }
