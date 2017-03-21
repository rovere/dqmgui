#ifndef DQM_VISDQMTOOLS_H
# define DQM_VISDQMTOOLS_H

# include "DQM/VisDQMFile.h"
# include "DQM/StringAtom.h"
# include "classlib/utils/DebugAids.h"
# include "classlib/utils/Signal.h"
# if !VISDQM_NO_ROOT
#  include "TBufferFile.h"
#  include "TStreamerInfo.h"
#  include "TClass.h"
#  include "TDataMember.h"
#  include "TROOT.h"
# endif
# include <iostream>
# include <set>
# include <string>
# include <cstdio>


/// Options on how we want fatal signals to be handled.
static const int FATAL_OPTS = (lat::Signal::FATAL_DEFAULT
			       & ~(lat::Signal::FATAL_ON_INT
				   | lat::Signal::FATAL_ON_QUIT
				   | lat::Signal::FATAL_DUMP_CORE));

// ----------------------------------------------------------------------
/** Assertion failure handler.  Always abort without any input. */
inline char
onAssertFail(const char *message)
{
  std::cout.flush(); fflush(stdout);
  std::cerr.flush(); fflush(stderr);
  std::cerr << message << "ABORTING\n";
  return 'a';
}

//----------------------------------------------------------------------
/** Extract a string table with top 32-bit key @a idx in DQM file @a f
    into @a tree.  The full string contents are stored as is, without
    a terminating null character, so binary data blobs are also ok. */
inline void
readStrings(StringAtomTree &tree, VisDQMFile *f, IndexKey idx)
{
  void *begin;
  void *end;
  IndexKey key;

  for (VisDQMFile::ReadHead rdhead(f, idx); ! rdhead.isdone(); rdhead.next())
  {
    rdhead.get(&key, &begin, &end);
    if (key.catalogIndex() != idx.catalogIndex())
      break;

    std::string data((const char *) begin, (const char *) end);
    VERIFY(tree.insert(data) == key.objectIndex());
  }
}

/** Write a string table @a tree into a file at @a wrhead with the top
    32-bits equal to @a idx and low 32-bits the tree key index.  The
    full string contents is written out with a terminating null byte,
    so binary data blobs with embedded nulls are also ok.  If @a base
    is given, strings are written starting from that index, otherwise
    starting from index 1. */
inline void
writeStrings(VisDQMFile::WriteHead &wrhead, StringAtomTree &tree, IndexKey idx, size_t base = 1)
{
  for (size_t i = base, e = tree.size(); i < e; ++i)
  {
    void *wrbegin, *wrend;
    const std::string &str = tree.key(i);
    wrhead.allocate(idx + i, str.size(), &wrbegin, &wrend);
    memcpy(wrbegin, str.c_str(), str.size());
  }
}

/** Utility function to print in HEX format an arbitrary string. */
template<class T >
static std::string hexlify (const T & x)
{
  std::string result;
  result.reserve(2*x.size());
  for (size_t i = 0, e = x.size(); i != e; ++i)
  {
    char buf[3];
    sprintf(buf, "%02x", (unsigned) (unsigned char) x[i]);
    result += buf[0];
    result += buf[1];
  }
  return result;
}

static int to_int(int c)
{
  if (isdigit(c))
    return c - '0';
  else
  {
    if (isupper(c))
      c = tolower(c);
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
  }
  return -1;
}

/** Utility function to recover the original string, before converion
    to HEX notation. */
static std::string unhexlify(const std::string& s)
{
  size_t len = s.size();
  assert(len > 0);
  assert(len % 2 == 0);
  std::string result;
  result.reserve(len/2);

  size_t i=0;
  for (; i < len; i += 2)
  {
    int top = to_int((unsigned char)(s[i] & 0xff));
    int bot = to_int((unsigned char)(s[i+1] & 0xff));
    result += (top << 4) + bot;
  }
  return result;
}

std::ostream &operator<<(std::ostream &out, IndexKey c)
{
  out << std::hex << (c.sampleAndType >> 4) << "-"
      << (c.sampleAndType & 0xf) << ":";
  out << std::hex << (c.lumiAndObjname >> 32) << "-"
      << (c.lumiAndObjname & 0xffffffff) << std::dec;
  return out;
}

#if !VISDQM_NO_ROOT
void analyseClass(TClass *, std::set<std::string> &);
void loopOnDataMembers(TClass *, std::set<std::string> &);
void loopOnBases(TClass *, std::set<std::string> &);

void loopOnDataMembers(TClass *tcl,
                       std::set<std::string> &unique_classes)
{
  TList *dms = tcl->GetListOfDataMembers();
  TIter next(dms);
  while (TObject *obj = next()) {
    TClass *cl = TClass::GetClass(((TDataMember *)obj)->GetFullTypeName());
    if (cl && cl->HasDictionary()) {
      unique_classes.insert(cl->GetName());
      analyseClass(cl, unique_classes);
    }
  }
}


void loopOnBases(TClass *tcl,
                 std::set<std::string> &unique_classes)
{
  TList *bases = tcl->GetListOfBases();
  TIter next(bases);
  while (TObject *obj = next()) {
    TClass *cl = TClass::GetClass(obj->GetName());
    if (cl && cl->HasDictionary()) {
      unique_classes.insert(cl->GetName());
      analyseClass(cl, unique_classes);
    }
  }
}


void analyseClass(TClass *cl,
                  std::set<std::string> &unique_classes)
{
  loopOnBases(cl, unique_classes);
  loopOnDataMembers(cl, unique_classes);
}

//----------------------------------------------------------------------
/** Extract the next serialised ROOT object from @a buf.  Returns null
    if there are no more objects in the buffer, or a null pointer was
    serialised at this location. */
inline TObject *
extractNextObject(TBufferFile &buf)
{
  if (buf.Length() == buf.BufferSize())
    return 0;
  buf.InitMap();
  return (TObject *) buf.ReadObjectAny(0);
}

/** Load ROOT streamer info from a serialised blob @a data.  It is
    assumed the serialised blob was created by #buildStreamerInfo(). */
inline void
loadStreamerInfo(const void *data, size_t len)
{
  TBufferFile buf(TBufferFile::kRead, len, (void *) data, kFALSE);
  buf.Reset();
  while (buf.Length() != buf.BufferSize())
  {
    TObject *obj = extractNextObject(buf);
    if (TStreamerInfo *si = dynamic_cast<TStreamerInfo *>(obj))
    {
      Int_t v = si->GetClassVersion();
      if (TClass *c = TClass::GetClass(si->GetName(), kTRUE))
      {
	c->GetStreamerInfo();
	if (c->GetStreamerInfos()->At(v))
	{
	  DEBUG(2, "skipping already present streamer info version " << v
		<< " for '" << si->GetName() << "'\n");
	  delete obj;
	  continue;
	}
      }

      si->BuildCheck();
      DEBUG(2, "importing streamer info version " << v
	    << " for '" << si->GetName() << "'\n");

      if (si->TestBit(kCanDelete))
	delete obj;
    }
    else if (! obj)
      std::cerr << "warning: unexpected null streamer info\n";
    else
    {
      std::cerr << "warning: unexpected object embedded in streamer info\n";
      delete obj;
    }
  }
}

inline void
loadStreamerInfo(const std::string &data)
{
  loadStreamerInfo(&data[0], data.size());
}

/** Build a streamer info data blob corresponding to the current ROOT
    version into @a data.  This represents the ROOT version of the
    presently running program, not the version of the ROOT objects
    read in: ROOT will convert whatever we read into the version we
    run now.  This function should be called before any ROOT files
    have been opened as file opens may modify the streamer info. This
    function properly saves all the classes needed to decode the ROOT
    objects handled by the DQM GUI, including all base classes of the
    objects themselves and of all their streamed members. */
inline void
buildCompleteStreamerInfo(std::string &data)
{
  std::set<std::string> unique_classes;
  static const char *classes[] =
    {
      "TH1F", "TH1S", "TH1D", "TH1I",
      "TH2F", "TH2S", "TH2D", "TH2I",
      "TH3F", "TH3S", "TH3D", "TH3I",
      "TProfile", "TProfile2D", 0
    };

  int i = 0;
  while (classes[i])
  {
    TClass *tcl = TClass::GetClass(classes[i]);
    if (!tcl)
      continue;
    unique_classes.insert(classes[i]);
    analyseClass(tcl, unique_classes);
    ++i;
  }
  TBufferFile buf(TBufferFile::kWrite);
  for (auto & cl : unique_classes)
    TClass::GetClass(cl.data())->GetStreamerInfo();

  TIter next(gROOT->GetListOfStreamerInfo());
  TStreamerInfo * si;
  while ((si = (TStreamerInfo*)next()))
    buf.WriteObject(si);

  data.resize(buf.Length());
  memcpy(&data[0], buf.Buffer(), buf.Length());
}


#endif // VISDQM_NO_ROOT

#endif // DQM_VISDQMTOOLS_H
