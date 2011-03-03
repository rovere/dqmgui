#ifndef DQM_VISDQMERROR_H
# define DQM_VISDQMERROR_H

# include "classlib/utils/Error.h"
# include <string>

/** A DQM plain string error. */
class VisDQMError : public lat::Error
{
public:
  VisDQMError(lat::Error *chain,
	      const std::string &context,
	      const std::string &message);
  VisDQMError(const VisDQMError &other);
  VisDQMError& operator=(const VisDQMError &other);
  ~VisDQMError(void);

  virtual std::string	explainSelf(void) const;
  virtual Error *	clone(void) const;
  virtual void		rethrow(void);

private:
  std::string		context_;
  std::string		message_;
};

#endif // DQM_VISDQMERROR_H
