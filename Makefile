BINDIR:=bin/
LIBDIR:=lib/
INCDIR:=include/
SRCDIR:=src/
SHAREDIR:=share/

BOOSTDIR:=../boost/
BOOST_INCDIR:=$(BOOSTDIR)
BOOST_LIBDIR:=$(BOOSTDIR)stage/lib/
BOOST_CXXFLAGS:=-I$(BOOST_INCDIR)
BOOST_LDFLAGS:=-L$(BOOST_LIBDIR) \
	-lboost_system -lboost_thread -lboost_filesystem -lssl -lcrypto

RPATH:=$(BOOST_LIBDIR):$(LIBDIR)

MKDIR:=@mkdir -p
CXXFLAGS:=$(CXXFLAGS) -std=c++11 -fpic -I$(INCDIR) $(BOOST_CXXFLAGS)
LDFLAGS:=$(LDFLAGS) $(BOOST_LDFLAGS) \
	-L$(LIBDIR) -Wl,-rpath,$(RPATH) \
	-lpthread -ldl

OBJ_EXT:=.o
DLL_EXT:=.so
EXE_EXT:=

APPLICATIONS:=main
MODULES:=
SUBMODULES:=print net client
OBJECTS:=$(foreach obj,$(APPLICATIONS) $(MODULES) $(SUBMODULES),\
	$(obj:%=$(LIBDIR)%$(OBJ_EXT)))
DLLS:=$(foreach obj,$(MODULES) $(SUBMODULES),\
	$(obj:%=$(LIBDIR)lib%$(DLL_EXT)))
EXES:=$(foreach app,$(APPLICATIONS),$(app:%=$(BINDIR)%$(EXE_EXT)))

SENTINEL_DIRS:=$(BINDIR) $(SHAREDIR) $(LIBDIR)\
	$(foreach mod,$(MODULES),$(mod:%=$(LIBDIR)%))

default:.sentinel $(OBJECTS) $(DLLS) $(EXES)
all:default
.sentinel:; $(MKDIR) $(SENTINEL_DIRS)
	@touch .sentinel

vpath %.cpp $(SRCDIR)
vpath %.hpp $(INCDIR)

$(LIBDIR)%$(OBJ_EXT):%.cpp %.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<
$(LIBDIR)*/%$(OBJ_EXT):*/%.cpp */%.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(LIBDIR)lib%$(DLL_EXT):$(LIBDIR)%$(OBJ_EXT)
	$(CXX) -shared -o $@ $< $(LDFLAGS)
$(LIBDIR)*/lib%$(DLL_EXT):$(LIBDIR)*/%$(OBJ_EXT)
	$(CXX) -shared -o $@ $< $(LDFLAGS)

$(BINDIR)%$(EXE_EXT):$(DLLS) $(LIBDIR)%$(OBJ_EXT)
	$(CXX) -o $@ $(LIBDIR)main$(OBJ_EXT) $(LDFLAGS) -lprint -lnet -lclient

clean:;@$(RM) $(OBJECTS) $(DLLS) $(EXES)
.PHONY:clean all
