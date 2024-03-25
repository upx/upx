#
# UPX top-level Makefile - needs GNU make and CMake >= 3.13
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# INFO: this Makefile is just a convenience wrapper for calling CMake

# HINT: if you only have an older CMake 3.x then you can invoke cmake manually like this:
#   mkdir -p build/release
#   cd build/release
#   cmake ../..         # run config
#   make -j             # and run build

CMAKE = cmake
UPX_CMAKE_BUILD_FLAGS += --parallel
ifneq ($(VERBOSE),)
  #UPX_CMAKE_BUILD_FLAGS += --verbose # requires CMake >= 3.14
  UPX_CMAKE_CONFIG_FLAGS += -DCMAKE_VERBOSE_MAKEFILE=ON
endif
# enable this if you prefer Ninja for the actual builds:
#UPX_CMAKE_CONFIG_FLAGS += -G Ninja

#***********************************************************************
# default
#***********************************************************************

.DEFAULT_GOAL = build/release

run_cmake_config = $(CMAKE) -S . -B $1 $(UPX_CMAKE_CONFIG_FLAGS) -DCMAKE_BUILD_TYPE=$2
run_cmake_build  = $(CMAKE) --build $1 $(UPX_CMAKE_BUILD_FLAGS) --config $2
# avoid re-running run_cmake_config if .upx_cmake_config_done.txt already exists
run_config       = $(if $(wildcard $1/CMakeFiles/.upx_cmake_config_done.txt),,$(call run_cmake_config,$1,$2))
run_build        = $(call run_cmake_build,$1,$2)

build/debug: PHONY
	$(call run_config,$@,Debug)
	$(call run_build,$@,Debug)

build/release: PHONY
	$(call run_config,$@,Release)
	$(call run_build,$@,Release)

.NOTPARALLEL: # because the actual builds use "cmake --parallel"
.PHONY: PHONY
.SECONDEXPANSION:
.SUFFIXES:

# shortcuts (all => debug + release)
debug:   build/debug PHONY
release: build/release PHONY
all build/all: build/debug build/release PHONY
build/%/all:   $$(dir $$@)debug $$(dir $$@)release PHONY ;

#
# END of Makefile
#

# extra pre-defined build configurations and some utility; optional
ifneq ($(MAKEFILE_LIST),)
include $(dir $(lastword $(MAKEFILE_LIST)))/misc/make/Makefile-extra.mk
else
include ./misc/make/Makefile-extra.mk
endif

# developer convenience
test:: build/all+test PHONY
ifneq ($(wildcard /usr/bin/env),) # need Unix utils like bash, perl, sed, xargs, etc.
ifneq ($(wildcard ./misc/scripts/.),)
check-whitespace clang-format run-testsuite run-testsuite-debug run-testsuite-release: src/Makefile PHONY
	$(MAKE) -C src $@
endif
endif
