#ifndef DQM_WALKDIRS_H
# define DQM_WALKDIRS_H

# include "classlib/iobase/DirIterator.h"
# include "classlib/iobase/StringList.h"
# include "classlib/iobase/Filename.h"
# include <stack>

/** Simple iterator-like utility class to walk a directory trees, with
    interface similar to python's "os.walk()".  For each subdirectory
    level provides the directory name and a list of files.  Symlinks
    are skipped entirely on both lists.  Each directory is guaranteed
    to be returned before any of its subdirectories (= pre-order walk).  */
class WalkDirs
{
  // Implicit destructor, copy constructor and assignment operator
  struct DirState
  {
    lat::Filename	dir;
    lat::StringList	dirs;
    size_t		next;
  };

  void push (const lat::Filename &dir)
  {
    files_.clear ();
    state_.push (DirState ());
    DirState &top = state_.top ();
    top.dir = dir;
    top.next = 0;
    for (lat::DirIterator i (dir), e; i != e; ++i)
    {
      lat::Filename x (dir, *i);
      if (x.isSymlink())
	/* skip */;
      else if (x.isDirectory())
	top.dirs.push_back (*i);
      else
	files_.push_back (*i);
    }
  }

  std::stack<DirState>	state_;
  lat::StringList	files_;
public:
  WalkDirs (void)
  {}

  WalkDirs (const lat::Filename &dir)
  { push (dir); }

  bool operator== (const WalkDirs &x) const
  {
    if (! state_.size () && ! x.state_.size ())
      return true;
    if (state_.size () != x.state_.size ())
      return false;
    if (state_.top ().dir != x.state_.top ().dir)
      return false;
    if (state_.top().next != x.state_.top ().next)
      return false;
    return true;
  }

  bool operator!= (const WalkDirs &x) const
  { return ! (*this == x); }

  WalkDirs &operator++ (void)
  {
    DirState *top = 0;
    while (state_.size ())
    {
      top = &state_.top ();
      if (top->next < top->dirs.size ())
        break;
      state_.pop ();
    }

    if (state_.size ())
      push (lat::Filename (top->dir, top->dirs[top->next++]));

    return *this;
  }

  WalkDirs operator++ (int)
  { WalkDirs x (*this); ++*this; return x; }

  const lat::Filename &dir (void) const
  { return state_.top ().dir; }

  const lat::StringList &dirs (void) const
  { return state_.top ().dirs; }

  const lat::StringList &files (void) const
  { return files_; }
};
