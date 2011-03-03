#ifndef DQM_LINEBUFFER_H
# define DQM_LINEBUFFER_H

# include <deque>
# include <string>

/** Simple queue class for holding partially processed data for
    asynchronous communication.  Input is assumed to be single lines
    of ASCII text, possibly broken down randomly due to buffering.
    Each line is stored into a queue of pending lines to process.
    The last line may be incompletely received and the first one
    may be incompletely processed.  New lines are added to the back
    of the queue, and are consumed for processing from the front.  */
class LineBuffer
{
  /** A buffered line.  Tracks the actual string data, the start
      position on where we are in consuming data from the line,
      and a flag to indicate whether the line should be considered
      "complete" and should no longer be appended to; this flag is
      automatically set whenever a newline is detected.  */
  struct Line
  {
    std::string		line;
    size_t		start;
    bool		complete;
  };

  /** The actual line buffer, suitable for changes at both ends.  */
  std::deque<Line>	queue_;

  /** Return a reference to the last line in the buffer to add to.
      If the queue is empty or the last line was complete, adds a
      new line and returns a reference to it.  Otherwise returns a
      reference to the last line.  */
  Line &last (void)
  {
    if (queue_.empty () || queue_.back ().complete)
    {
      queue_.push_back (Line ());
      Line &l = queue_.back ();
      l.start = 0;
      l.complete = false;
      return l;
    }
    else
      return queue_.back ();
  }

public:
  // Implicit destructor, copy constructor and assignment operator.

  /** Initialise an empty line buffer.  */
  LineBuffer (void)
  {}

  /** Clear all the contents of the buffer.  */
  void clear (void)
  {
    queue_.clear ();
  }

  /** Return the number of lines in the buffer.  Note that the last line
      in the buffer may be incomplete.  Use "length()" to determine the
      amount of data in the buffer.  */
  size_t size (void)
  {
    return queue_.size ();
  }

  /** Append more data to the buffer.  Appends to previous incomplete
      line if there is one, or adds another line if there isn't one.
      If @a complete is set (the default), the resulting line is
      forced to be complete.  Otherwise it's left incomplete and the
      next call to "queue()" will append to it.  */
  void queue (const std::string &line, bool complete = true)
  {
    Line &l = last ();
    l.line.append (line);
    if (complete)
      l.complete = true;
  }

  /** Append more data to the buffer by parsing for lines.  Finds the
      line breaks in @a data and appends each line to the buffer.  If
      the last line already in the buffer is incomplete, the first
      data is appended to it; otherwise buffers a new line.  If the
      supplied data does not end in a new-line, leaves the last line
      in the buffer incomplete.  */
  void parse (void *data, size_t len)
  {
    size_t	pos = 0;
    const char	*p = (const char *) data;

    while (pos < len)
    {
      if (queue_.empty () || queue_.back ().complete)
      {
	queue_.push_back (Line ());
        queue_.back ().start = 0;
        queue_.back ().complete = false;
      }

      Line &l = queue_.back ();
      size_t  start = pos;

      while (pos < len && p[pos] != '\n')
	++pos;

      l.line.append (p+start, p+pos);
      if (p[pos] == '\n')
      {
	l.complete = true;
        pos++;
      }
    }
  }

  /** Get the current buffer position.  Fills @a data with a pointer
      to the next buffered line and returns true if a complete line
      is available; if there is none, or the remaining last line is
      incomplete, returns false.  If @a start is non-null, sets it
      to the starting position in the buffer.  This is the amount
      previously consumed and indicated by call to "consumed()".  */
  bool nextLine (const std::string *&data, size_t *start = 0)
  {
    if (queue_.empty () || ! queue_.front ().complete)
      return false;

    Line &l = queue_.front();
    data = &l.line;
    if (start)
      *start = l.start;
    return true;
  }

  /** Get the current buffer position.  Fills @a data with a pointer
      to the next character to be consumed from the buffer, and @a
      len with the number of bytes left in the line to consume.  If
      there is no data left in the buffer, that is the buffer is
      empty or the last remaining line is incomplete, returns false,
      otherwise returns true.  */
  bool nextBuffer (void *&data, size_t &len)
  {
    if (queue_.empty () || ! queue_.front ().complete)
      return false;

    Line &l = queue_.front();
    data = &l.line[l.start];
    len = l.line.size () - l.start;
    return true;
  }

  /** Tell the buffer how much data was consumed from the buffer as
      a consequence of using the values returned by "nextLine()" or
      "nextBuffer()".  Advances the current position in the buffer
      by that amount.  Note that @a bytes must be less than or equal
      to the length of the line returned; it cannot be more.  */
  void consumed (size_t bytes)
  {
    Line &l = queue_.front();
    if ((l.start += bytes) == l.line.size ())
      queue_.pop_front();
  }

  /** Return the number of bytes of data in the buffer.  */
  size_t length (void) const
  {
    std::deque<Line>::const_iterator i = queue_.begin();
    std::deque<Line>::const_iterator e = queue_.end();
    size_t n = 0;
    for ( ; i != e; ++i)
      n += i->line.size ();

    return n;
  }
};

#endif // DQM_LINEBUFFER_H
