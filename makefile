TOPDIR = $(shell pwd)
export TOPDIR

TGT = rf4

CONFIGS_DIR = $(TOPDIR)/configs
export CONFIGS_DIR

CONFIG_NAMES_FULL = $(wildcard $(CONFIGS_DIR)/*)
CONFIG_NAMES      = $(strip $(notdir $(CONFIG_NAMES_FULL)))
export CONFIG_NAMES

TARGET_NAMES      = $(CONFIG_NAMES) clean distclean

CHOSEN_TARGET     = $(findstring $(MAKECMDGOALS),$(TARGET_NAMES))

ifneq ("x","x$(CHOSEN_TARGET)")
    REALGOAL = $(CHOSEN_TARGET)
    ifneq (,$(findstring $(CHOSEN_TARGET),"clean distclean"))
        CHOSEN_TARGET =
    else
        include $(CONFIGS_DIR)/$(CHOSEN_TARGET)
    endif
else
    REALGOAL = not_valid
endif

export CHOSEN_TARGET


SUBDIRS = FEM GEO MSH UTL 

OBJS_PATTERN = $(foreach subdir,$(SUBDIRS),$(subdir)/*.o)


.PHONY:	first
first:	check_config

distclean clean:
	@ for i in $(SUBDIRS); do \
            ( cd $$i && $(MAKE) $@ ) \
        done
	$(RM) $(TGT)

not_valid:
	@ echo "Possible configurations are '$(CONFIG_NAMES)'."

.PHONY:	check_config
check_config:	$(REALGOAL)


.PHONY:	$(CONFIG_NAMES)
$(CONFIG_NAMES):
	@ echo "Real goal: $(REALGOAL)"
	@ echo "Config names $(CONFIG_NAMES)"
	@ echo "Chosen target '$(CHOSEN_TARGET)'"
	@ for i in $(SUBDIRS); do \
            $(MAKE) -C $$i $(CHOSEN_TARGET) || exit 1; \
        done
	$(CXX_LD) $(CXX_LDFLAGS) -o $(TGT) $(OBJS_PATTERN) $(CXX_LIBS) $(LIS_LIBS) $(GEM_LIBS) $(PQC_LIBS)

