#ifndef DQM_VISDQMBUF_H
# define DQM_VISDQMBUF_H

# include <stddef.h>
# include <stdlib.h>
# include <string.h>
# include <algorithm>

class VisDQMBuf
{
  void		*buf_;
  size_t	len_;
  size_t	cap_;

public:
  VisDQMBuf(void)
    : buf_(0),
      len_(0),
      cap_(0)
    {}

  VisDQMBuf(size_t n)
    : buf_(0),
      len_(0),
      cap_(0)
    {
      resize(n);
    }

  VisDQMBuf(const VisDQMBuf &x)
    : buf_(0),
      len_(0),
      cap_(0)
    {
      reserve(x.cap_);
      len_ = x.len_;
      if (buf_)
        memcpy(buf_, x.buf_, len_);
    }

  ~VisDQMBuf(void)
    {
      free(buf_);
    }

  void resize(size_t n)
    {
      reserve(n);
      len_ = n;
    }

  void reserve(size_t n)
    {
      if (n <= cap_)
	;
      else if (! (buf_ = realloc(buf_, n)))
	die();
      else
        cap_ = n;
    }

  unsigned char *buf(void) const
    {
      return (unsigned char *) buf_;
    }

  unsigned char &operator[](ptrdiff_t x) const
    {
      return *(buf() + x);
    }

  size_t size(void) const
    {
      return len_;
    }

  bool empty(void) const
    {
      return len_ == 0;
    }

  void release(void)
    {
      free(buf_);
      buf_ = 0;
      len_ = 0;
      cap_ = 0;
    }

  void append(const void *data, size_t len)
    {
      size_t pos = len_;
      resize(len_ + len);
      memcpy(buf() + pos, data, len);
    }

  void swap(VisDQMBuf &x)
    {
      std::swap(x.buf_, buf_);
      std::swap(x.len_, len_);
      std::swap(x.cap_, cap_);
    }

private:
  void die(void);
  void operator=(VisDQMBuf &);
};

#endif // DQM_VISDQMBUF_H
