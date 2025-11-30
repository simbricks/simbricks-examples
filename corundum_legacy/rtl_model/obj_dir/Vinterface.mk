# Verilated -*- Makefile -*-
# DESCRIPTION: Verilator output: Makefile for building Verilated archive or executable
#
# Execute this makefile from the object directory:
#    make -f Vinterface.mk

default: Vinterface

### Constants...
# Perl executable (from $PERL)
PERL = perl
# Path to Verilator kit (from $VERILATOR_ROOT)
VERILATOR_ROOT = /usr/local/share/verilator
# SystemC include directory with systemc.h (from $SYSTEMC_INCLUDE)
SYSTEMC_INCLUDE ?= 
# SystemC library directory with libsystemc.a (from $SYSTEMC_LIBDIR)
SYSTEMC_LIBDIR ?= 

### Switches...
# SystemC output mode?  0/1 (from --sc)
VM_SC = 0
# Legacy or SystemC output mode?  0/1 (from --sc)
VM_SP_OR_SC = $(VM_SC)
# Deprecated
VM_PCLI = 1
# Deprecated: SystemC architecture to find link library path (from $SYSTEMC_ARCH)
VM_SC_TARGET_ARCH = linux

### Vars...
# Design prefix (from --prefix)
VM_PREFIX = Vinterface
# Module prefix (from --prefix)
VM_MODPREFIX = Vinterface
# User CFLAGS (from -CFLAGS on Verilator command line)
VM_USER_CFLAGS = \
	-I/simbricks/lib -iquote /simbricks -O3 -g -Wall -Wno-maybe-uninitialized \

# User LDLIBS (from -LDFLAGS on Verilator command line)
VM_USER_LDLIBS = \
	/simbricks/lib/simbricks/nicif/libnicif.a \
	/simbricks/lib/simbricks/network/libnetwork.a \
	/simbricks/lib/simbricks/pcie/libpcie.a \
	/simbricks/lib/simbricks/base/libbase.a \
	/simbricks/lib/simbricks/parser/libparser.a \

# User .cpp files (from .cpp's on Verilator command line)
VM_USER_CLASSES = \
	corundum_verilator \
	dma \
	mem \

# User .cpp directories (from .cpp's on Verilator command line)
VM_USER_DIR = \
	/workspaces/simbricks-examples-tmp/corundum_legacy/rtl_model \


### Default rules...
# Include list of all generated classes
include Vinterface_classes.mk
# Include global rules
include $(VERILATOR_ROOT)/include/verilated.mk

### Executable rules... (from --exe)
VPATH += $(VM_USER_DIR)

corundum_verilator.o: /workspaces/simbricks-examples-tmp/corundum_legacy/rtl_model/corundum_verilator.cc
	$(OBJCACHE) $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OPT_FAST) -c -o $@ $<
dma.o: /workspaces/simbricks-examples-tmp/corundum_legacy/rtl_model/dma.cc
	$(OBJCACHE) $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OPT_FAST) -c -o $@ $<
mem.o: /workspaces/simbricks-examples-tmp/corundum_legacy/rtl_model/mem.cc
	$(OBJCACHE) $(CXX) $(CXXFLAGS) $(CPPFLAGS) $(OPT_FAST) -c -o $@ $<

### Link rules... (from --exe)
Vinterface: $(VK_USER_OBJS) $(VK_GLOBAL_OBJS) $(VM_PREFIX)__ALL.a
	$(LINK) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@ $(LIBS) $(SC_LIBS)


# Verilated -*- Makefile -*-
