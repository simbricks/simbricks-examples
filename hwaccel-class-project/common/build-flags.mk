CPPFLAGS= -I../common/ -I/simbricks/lib $(EXTRA_CPPFLAGS)
CFLAGS= -Wall -Wextra -Wno-unused-parameter -O3 $(EXTRA_CFLAGS)
CXXFLAGS= -Wall -Wextra -O3 $(EXTRA_CXXFLAGS)
LDFLAGS= -L/simbricks/lib $(EXTRA_LDFLAGS)

SIMBRICKS_FLAGS:=

ifeq ($(PARALLEL),y)
  SIMBRICKS_FLAGS += --parallel
endif
