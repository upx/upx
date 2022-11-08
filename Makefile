#
# UPX top-level Makefile - needs GNU make and CMake >= 3.13
#

# NOTE: if you only have CMake 3.10 then you can invoke cmake manually like this:
#   mkdir -p build/release
#   cd build/release
#   cmake ../..
#   cmake --build .

CMAKE = cmake
UPX_CMAKE_BUILD_FLAGS += --parallel
ifneq ($(VERBOSE),)
  UPX_CMAKE_BUILD_FLAGS += --verbose
endif

# check git submodules
ifeq ($(wildcard ./vendor/doctest/doctest/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/lzma-sdk/C/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/ucl/include/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/zlib/crc32.c),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif

#***********************************************************************
# default
#***********************************************************************

__check_cache = $(if $(wildcard $1/CMakeCache.txt),@true,)
run_config = $(call __check_cache,$1) $(CMAKE) -S . -B $1 $(UPX_CMAKE_CONFIG_FLAGS) -DCMAKE_BUILD_TYPE=$2
run_build  =                          $(CMAKE) --build $1 $(UPX_CMAKE_BUILD_FLAGS) --config $2

.DEFAULT_GOAL = build/release

build/debug: PHONY
	$(call run_config,$@,Debug)
	$(call run_build,$@,Debug)

build/release: PHONY
	$(call run_config,$@,Release)
	$(call run_build,$@,Release)

# shortcuts
debug: build/debug
release: build/release

#***********************************************************************
# some pre-defined build configurations
#***********************************************************************

# force building with gcc/g++
build/%-gcc: export CC=gcc
build/%-gcc: export CXX=g++
build/debug-gcc: PHONY
	$(call run_config,$@,Debug)
	$(call run_build,$@,Debug)
build/release-gcc: PHONY
	$(call run_config,$@,Release)
	$(call run_build,$@,Release)

# force building with clang/clang++
build/%-clang: export CC=clang
build/%-clang: export CXX=clang++
build/debug-clang: PHONY
	$(call run_config,$@,Debug)
	$(call run_build,$@,Debug)
build/release-clang: PHONY
	$(call run_config,$@,Release)
	$(call run_build,$@,Release)

# cross compiler: Windows mingw-w64
build/%-cross-mingw64: export CC=x86_64-w64-mingw32-gcc
build/%-cross-mingw64: export CXX=x86_64-w64-mingw32-g++
#   disable sanitize to avoid link errors on GitHub CI
build/%-cross-mingw64: UPX_CMAKE_CONFIG_FLAGS += -DUPX_CONFIG_DISABLE_SANITIZE=1
build/debug-cross-mingw64: PHONY
	$(call run_config,$@,Debug)
	$(call run_build,$@,Debug)
build/release-cross-mingw64: PHONY
	$(call run_config,$@,Release)
	$(call run_build,$@,Release)

.PHONY: PHONY
