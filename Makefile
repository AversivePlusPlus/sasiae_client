PACKAGE_NAME = sasiae
CMAKE ?= cmake

################################
.PHONY: test all

################################
all: build
	@cd build && $(CMAKE) .. && make

################################
TESTS =

define TEST_RULE
test: test_$(1)

test_$(1):
	@cd test/$(1) && $(MAKE) -s test
endef

ifeq (,$(TESTS))
$(eval $(call TEST_RULE,))
endif

$(foreach t,$(TESTS),$(eval $(call TEST_RULE,$(t))))

################################
clean:
	@rm -rf build

define CLEAN_RULE
clean: clean_$(1)

clean_$(1):
	@cd test/$(1) && $(MAKE) -s clean
endef

$(foreach t,$(TESTS),$(eval $(call CLEAN_RULE,$(t))))

mrproper: clean
	@rm -f *~
	@rm -f *.pyc

define MRPROPER_RULE
mrproper: mrproper_$(1)

mrproper_$(1):
	@cd test/$(1) && $(MAKE) -s mrproper
endef

$(foreach t,$(TESTS),$(eval $(call MRPROPER_RULE,$(t))))

################################
build:
	@mkdir $@
