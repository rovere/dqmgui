#include "DQM/VisDQMError.h"

using namespace lat;

/** Construct a new DQM plain string error.
    @param chain    Upstream reason error if any.
    @param context  Context where the error happened.
    @param message  The error explained as a string. */
VisDQMError::VisDQMError(Error *chain,
			 const std::string &context,
			 const std::string &message)
  : Error(chain),
    context_(context),
    message_(message)
{}

/** Clone a DQM plain string error. */
VisDQMError::VisDQMError(const VisDQMError &other)
  : Error(other),
    context_(other.context_),
    message_(other.message_)
{}

/** Clone a DQM plain string error. */
VisDQMError &
VisDQMError::operator=(const VisDQMError &other)
{
  Error::operator=(other);
  context_ = other.context_;
  message_ = other.message_;
  return *this;
}

/** Destroy a DQM plain string error. */
VisDQMError::~VisDQMError(void)
{}

/** Produce a message for this error itself.
    Usually clients should use #explain(). */
std::string
VisDQMError::explainSelf(void) const
{
  std::string result;
  result.reserve(context_.size() + message_.size() + 4);
  result += context_;
  result += ": ";
  result += message_;
  return result;
}

/** Clone the error object. */
Error *
VisDQMError::clone(void) const
{
  return new VisDQMError(*this);
}

/** Rethrow this error object. */
void
VisDQMError::rethrow(void)
{
  throw *this;
}
