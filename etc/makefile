include $(dir $(lastword $(MAKEFILE_LIST)))/makefile.ext

PLATFORM    := $(shell uname)
SRCDIR       = .
CXX          = c++
LDFLAGS_Darwin = -mmacosx-version-min=10.6
LDFLAGS_Linux  = -Wl,-z,defs
LDFLAGS      = $(LDFLAGS_$(PLATFORM)) -L. $(patsubst %,-L%,$(LIBRARY_DIRS))

CXXFLAGS     = -D_GNU_SOURCE -D_DARWIN_C_SOURCE -DWITHOUT_CMS_FRAMEWORK=1 -D__STDC_LIMIT_MACROS \
	       -D__STDC_FORMAT_MACROS -U__DEPRECATED -fPIC -g -O2 -W -Wall -Wno-long-long \
	       -ansi -pedantic -I$(SRCDIR) $(patsubst %,-I%,$(INCLUDE_DIRS))
ROOTC_LIBS   = -lCint -lCore -lRIO -lNet -lHist -lMatrix -lThread -lTree -lMathCore
ROOTG_LIBS   = $(ROOTC_LIBS) -lTreePlayer -lGpad -lGraf3d -lGraf \
               -lPhysics -lPostscript -lASImage -ljpeg -lpng
OTHER_LIBS_Darwin =
OTHER_LIBS_Linux  = -lrt
OTHER_LIBS   = -lclasslib -lpcre -lz -lpthread -ldl $(OTHER_LIBS_$(PLATFORM))
PYTHON_LIBS  = -lboost_python -lpython2.6

LIB_SRC      = DQM/DQMRenderPlugin.cc \
               DQM/VisDQMBuf.cc \
               DQM/VisDQMCache.cc \
               DQM/VisDQMError.cc \
               DQM/VisDQMFile.cc \
               DQM/VisDQMIndex.cc
LIB_OBJ      = $(patsubst %.cc,%.o,$(notdir $(LIB_SRC)))
LIB_LINK     = $(CXX) $(CXXFLAGS) -shared -o $@ $(LIB_OBJ) \
               $(LDFLAGS) $(OTHER_LIBS)

COLL_SRC     = DQM/DQMCollector.cc # DQM/DQMNet.cc DQM/DQMError.cc
COLL_OBJ     = $(patsubst %.cc,%.o,$(notdir $(COLL_SRC)))
COLL_LINK    = $(CXX) $(CXXFLAGS) -fPIE -o $@ $(COLL_OBJ) \
               $(LDFLAGS) $(OTHER_LIBS)

EXT_SRC      = DQM/serverext.cc DQM/DQMNet.cc DQM/DQMError.cc
EXT_OBJ      = $(patsubst %.cc,%.o,$(notdir $(EXT_SRC)))
EXT_LINK     = $(CXX) $(CXXFLAGS) -shared -o $@ $(EXT_OBJ) \
               $(LDFLAGS) -lDQMGUI $(OTHER_LIBS) $(PYTHON_LIBS) -lpng

INDEX_SRC    = DQM/index.cc DQM/DQMNet.cc DQM/DQMError.cc \
               DQM/DQMStore.cc DQM/MonitorElement.cc \
               DQM/QTest.cc DQM/QReport.cc
INDEX_OBJ    = $(patsubst %.cc,%.o,$(notdir $(INDEX_SRC)))
INDEX_LINK   = $(CXX) $(CXXFLAGS) -fPIE -o $@ $(INDEX_OBJ) \
               $(LDFLAGS) -lDQMGUI $(ROOTC_LIBS) $(OTHER_LIBS)

RENDER_SRC   = DQM/render.cc DQM/DQMNet.cc DQM/DQMError.cc
RENDER_OBJ   = $(patsubst %.cc,%.o,$(notdir $(RENDER_SRC)))
RENDER_LINK  = $(CXX) $(CXXFLAGS) -fPIE -o $@ $(RENDER_OBJ) \
               $(LDFLAGS) -lDQMGUI $(ROOTG_LIBS) $(OTHER_LIBS) -ljpeg

vpath %.cc $(SRCDIR)/DQM $(MONITOR_ROOT)/xinclude/DQM $(MONITOR_ROOT)/include/DQM
all: visDQMIndex visDQMRender DQMCollector Accelerator.so libDQMGUI.so
clean:; rm -f *.o *.so visDQMIndex visDQMRender
%.o: %.cc; $(CXX) $(CXXFLAGS) -c $< -o $@
libDQMGUI.so: $(LIB_OBJ); $(LIB_LINK)
Accelerator.so: $(EXT_OBJ) libDQMGUI.so; $(EXT_LINK)
DQMCollector: $(COLL_OBJ); $(COLL_LINK)
visDQMIndex: $(INDEX_OBJ) libDQMGUI.so; $(INDEX_LINK)
visDQMRender: $(RENDER_OBJ) libDQMGUI.so; $(RENDER_LINK)
