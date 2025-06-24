-include nemu/Makefile.git

export NEMU_HOME ?= $(shell pwd)/nemu
export AM_HOME ?= $(shell pwd)/nexus-am
export NAVY_HOME ?= $(shell pwd)/navy-apps
export ICS_HOME ?= $(shell pwd)

default:
	@echo "Please run 'make' under any subprojects to compile."

clean:
	-$(MAKE) -C nemu clean
	-$(MAKE) -C nexus-am clean
	-$(MAKE) -C nanos-lite clean
	-$(MAKE) -C navy-apps clean

submit: clean
	git gc
	cd .. && tar cj $(shell basename `pwd`) > $(STU_ID).tar.bz2

.PHONY: default clean submit all

.PHONY: format
format:
	@find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.hh" \) -exec clang-format -i {} +
