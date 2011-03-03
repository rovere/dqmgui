#ifndef DQM_STRINGATOM_H
# define DQM_STRINGATOM_H

# include <string>
# include <vector>
# include <cassert>
# include <iostream>

/** Simple patricia tree implementation for a ultra-fast strings.
    Maintains a patricia tree of strings; an individual string is
    just an index into this table.  */
class StringAtomTree
{
public:
  /** Return value indicating the key was not found. */
  static const size_t	npos = ~(size_t)0;

  /** Number of bits used at a time from the key for each tree level. */
  static const size_t	BITS = 1;

  /** Number of child nodes per tree level.  */
  static const size_t	NODES = 1 << BITS;

private:
  /** Patricia tree node. */
  struct Node
  {
    std::string	s;
    size_t	bit, maxbit;
    size_t	kids[NODES];

    Node(void)
      : bit(0), maxbit(0)
    {
      for (size_t i = 0; i < NODES; ++i)
        kids[i] = 0;
    };
  };

  /** Actual patricia tree data structure.  This is just a simple array of
      individual nodes; nodes refer to each other by index.  */
  std::vector<Node>	tree_;
  pthread_mutex_t	lock_;

  /** Get @a n bits in character array @a p, limited to the
      maximum bit position @a maxbit, starting from position
      @a from.

      Several assumptions are made:
      - bits are counted from left regardless of machine order
      - selected bits do not straddle byte boundaries
      - if @a from does not exceed @a maxbit, neither does from+n,
        or otherwise put, @a maxbit falls only on byte boundaries.  */
  static inline unsigned int
  bits(const char *p, unsigned int maxbit, unsigned int from, unsigned int n)
  {
    return (from > maxbit ? 0 :
	    (((unsigned int) *(p+from/CHAR_BIT) >> (CHAR_BIT-from%CHAR_BIT-n))
	     & ~(~0 << n)));
  }

public:
  /** Initialise an empty patricia tree. */
  StringAtomTree(size_t capacity = 1024*1024)
  {
    tree_.reserve(capacity);
    tree_.push_back(Node());
    pthread_mutex_init(&lock_, 0);
  }

  /** Remove all strings from the tree.  Invalidates all outstanding
      StringAtom references to this tree. */
  void
  clear(void)
    {
      pthread_mutex_lock(&lock_);
      tree_.resize(1);
      tree_[0].bit = 0;
      tree_[0].maxbit = 0;
      for (size_t i = 0; i < NODES; ++i)
        tree_[0].kids[i] = 0;
      pthread_mutex_unlock(&lock_);
    }

  /** Insert a potentially new string into the tree.  Returns the index
      of the string in the tree, either an existing key or a new one. */
  size_t
  insert(const std::string &s)
  {
    size_t maxbit = s.size() * CHAR_BIT - 1;
    size_t t = tree_[0].kids[0];
    size_t p = 0;
    size_t i = 0;

    while (tree_[p].bit < tree_[t].bit)
    {
      size_t kid = bits(s.data(), maxbit, tree_[t].bit, BITS);
      p = t;
      t = tree_[t].kids[kid];
    }

    if (s == tree_[t].s)
      return t;

    pthread_mutex_lock(&lock_);
    // Find the first differing bit. This will hang if tree_[t]
    // is a prefix of s and s only contains null bytes beyond
    // tree_[t]. This is trivially true if s only contains null
    // bytes, as then the root of the tree will be the prefix.
    // Not an issue for true strings, but matters for raw data.
    while (bits(tree_[t].s.data(), tree_[t].maxbit, i, 1)
	   == bits(s.data(), maxbit, i, 1))
      ++i;

    p = 0;
    size_t x = tree_[0].kids [0];
    while (tree_[p].bit < tree_[x].bit && tree_[x].bit < i)
    {
      size_t kid = bits(s.data(), maxbit, tree_[x].bit, BITS);
      p = x;
      x = tree_[x].kids[kid];
    }

    assert(tree_.size() < tree_.capacity());
    size_t ibit = bits(s.data(), maxbit, i, 1);
    tree_.push_back(Node());
    t = tree_.size()-1;
    Node &n = tree_.back();
    n.s = s;
    n.bit = i;
    n.maxbit = maxbit;
    n.kids[0] = ibit ? x : t;
    n.kids[1] = ibit ? t : x;
    // FIXME: write barrier
    tree_[p].kids[bits(s.data(), maxbit, tree_[p].bit, 1)] = t;
    pthread_mutex_unlock(&lock_);
    return t;
  }

  /** Locate a key in the tree.  Returns the index of the string if it
      is known in the tree, or @c npos if not found.  */
  size_t
  search(const std::string &s)
  {
    size_t maxbit = s.size() * CHAR_BIT - 1;
    size_t p = 0, x = tree_[0].kids[0];
    while (tree_[p].bit < tree_[x].bit)
    {
      size_t kid = bits(s.data(), maxbit, tree_[x].bit, BITS);
      p = x;
      x = tree_[x].kids[kid];
    }

    if (s == tree_[x].s)
      return x;
    else
      return npos;
  }

  /** Return the actual string key associated with tree index position.
      Note that tree index 0 is the root of the tree and does not have
      a valid key.  */
  const std::string &
  key(size_t index)
  { return tree_[index].s; }

  /** Return the size of the patricia tree.  This is the number of the
      keys stored into the tree plus 1 for a tree root node.  */
  size_t
  size(void) const
  { return tree_.size(); }

  /** Return the maximum capacity of the patricia tree.  This is the
      maximum number of the keys that can be stored into the tree
      given at construction time. */
  size_t
  capacity(void) const
  { return tree_.capacity(); }
};

/** Class for ultra-fast non-modifiable strings.  The string object itself
    is simply an index into a #StringAtomTree.  The objects do not
    support almost any operations except retrieving the actual string
    and various order comparisons.  Note that comparisons sort strings
    into the order of the underlying patricia tree, not alphabetically.  */
class StringAtom
{
  StringAtomTree *tree_;
  size_t index_;
public:
  enum TestOnlyTag { TestOnly };
  StringAtom(void)
    : tree_(0), index_(0)
  {}

  StringAtom(StringAtomTree *tree, const std::string &str)
    : tree_(tree), index_(tree->insert(str))
  {}

  StringAtom(StringAtomTree *tree, const std::string &str, TestOnlyTag)
    : tree_(tree), index_(tree->search(str))
  { if (index_ == StringAtomTree::npos) index_ = 0; }

  StringAtomTree *
  tree(void) const
  { return tree_; }

  size_t index(void) const
  { return index_; }

  const std::string &string(void) const
  {
    assert(tree_ != 0);
    assert(index_ != StringAtomTree::npos);
    return tree_->key(index_);
  }

  size_t size(void) const
  { return string().size(); }
};

inline bool
operator== (const StringAtom &x, const StringAtom &y)
{ return x.index() == y.index(); }

inline bool
operator!= (const StringAtom &x, const StringAtom &y)
{ return x.index() != y.index(); }

inline bool
operator< (const StringAtom &x, const StringAtom &y)
{ return x.index() < y.index(); }

inline bool
operator<= (const StringAtom &x, const StringAtom &y)
{ return x.index() <= y.index(); }

inline bool
operator> (const StringAtom &x, const StringAtom &y)
{ return x.index() > y.index(); }

inline bool
operator>= (const StringAtom &x, const StringAtom &y)
{ return x.index() >= y.index(); }

#endif // DQM_STRINGATOM_H
