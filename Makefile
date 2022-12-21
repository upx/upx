#
# UPX top-level Makefile - needs GNU make and CMake >= 3.13
#

# NOTE: if you only have an older CMake 3.x then you can invoke cmake manually like this:
#   mkdir -p build/release
#   cd build/release
#   cmake ../..
#   cmake --build .

CMAKE = cmake
UPX_CMAKE_BUILD_FLAGS += --parallel
ifneq ($(VERBOSE),)
  UPX_CMAKE_BUILD_FLAGS += --verbose
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

.PHONY: PHONY

#***********************************************************************
# extra convenience: some pre-defined build configurations
#***********************************************************************

define run_config_and_build
	$(call run_config,$1,$2)
	$(call run_build,$1,$2)
endef

# force building with clang/clang++
build/debug-clang:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-clang: PHONY ; $(call run_config_and_build,$@,Release)
build/%-clang: export CC  = clang
build/%-clang: export CXX = clang++

# force building with clang/clang++ -m32
build/debug-clang-m32:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-clang-m32: PHONY ; $(call run_config_and_build,$@,Release)
build/%-clang-m32: export CC  = clang -m32
build/%-clang-m32: export CXX = clang++ -m32

# force building with clang/clang++ -m64
build/debug-clang-m64:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-clang-m64: PHONY ; $(call run_config_and_build,$@,Release)
build/%-clang-m64: export CC  = clang -m64
build/%-clang-m64: export CXX = clang++ -m64

# force building with gcc/g++
build/debug-gcc:   PHONY; $(call run_config_and_build,$@,Debug)
build/release-gcc: PHONY; $(call run_config_and_build,$@,Release)
build/%-gcc: export CC  = gcc
build/%-gcc: export CXX = g++

# force building with gcc/g++ -m32
build/debug-gcc-m32:   PHONY; $(call run_config_and_build,$@,Debug)
build/release-gcc-m32: PHONY; $(call run_config_and_build,$@,Release)
build/%-gcc-m32: export CC  = gcc -m32
build/%-gcc-m32: export CXX = g++ -m32

# force building with gcc/g++ -m64
build/debug-gcc-m64:   PHONY; $(call run_config_and_build,$@,Debug)
build/release-gcc-m64: PHONY; $(call run_config_and_build,$@,Release)
build/%-gcc-m64: export CC  = gcc -m64
build/%-gcc-m64: export CXX = g++ -m64

# cross compiler: Linux glibc aarch64-linux-gnu
build/debug-cross-linux-aarch64:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-cross-linux-aarch64: PHONY ; $(call run_config_and_build,$@,Release)
build/%-cross-linux-aarch64: export CC  = aarch64-linux-gnu-gcc
build/%-cross-linux-aarch64: export CXX = aarch64-linux-gnu-g++

# cross compiler: Linux glibc arm-linux-gnueabihf
build/debug-cross-linux-arm:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-cross-linux-arm: PHONY ; $(call run_config_and_build,$@,Release)
build/%-cross-linux-arm: export CC  = arm-linux-gnueabihf-gcc
build/%-cross-linux-arm: export CXX = arm-linux-gnueabihf-g++ -Wno-psabi

# cross compiler: Windows win32 mingw32
build/debug-cross-mingw32:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-cross-mingw32: PHONY ; $(call run_config_and_build,$@,Release)
build/%-cross-mingw32: export CC  = i686-w64-mingw32-gcc
build/%-cross-mingw32: export CXX = i686-w64-mingw32-g++
# disable sanitize to avoid link errors on GitHub CI
build/%-cross-mingw32: UPX_CMAKE_CONFIG_FLAGS += -DUPX_CONFIG_DISABLE_SANITIZE=1

# cross compiler: Windows win64 mingw64
build/debug-cross-mingw64:   PHONY ; $(call run_config_and_build,$@,Debug)
build/release-cross-mingw64: PHONY ; $(call run_config_and_build,$@,Release)
build/%-cross-mingw64: export CC  = x86_64-w64-mingw32-gcc
build/%-cross-mingw64: export CXX = x86_64-w64-mingw32-g++
# disable sanitize to avoid link errors on GitHub CI
build/%-cross-mingw64: UPX_CMAKE_CONFIG_FLAGS += -DUPX_CONFIG_DISABLE_SANITIZE=1

#***********************************************************************
# check git submodules
#***********************************************************************

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
