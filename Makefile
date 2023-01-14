#
# UPX top-level Makefile - needs GNU make and CMake >= 3.13
#

# INFO: this Makefile is just a convenience wrapper for calling CMake

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

run_cmake_config = $(CMAKE) -S . -B $1 $(UPX_CMAKE_CONFIG_FLAGS) -DCMAKE_BUILD_TYPE=$2
run_cmake_build  = $(CMAKE) --build $1 $(UPX_CMAKE_BUILD_FLAGS) --config $2
# avoid re-running run_cmake_config if CMakeCache.txt already exists
run_config       = $(if $(wildcard $1/CMakeCache.txt),,$(call run_cmake_config,$1,$2))
run_build        = $(call run_cmake_build,$1,$2)

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
.NOTPARALLEL: # because the actual builds use "cmake --parallel"

#***********************************************************************
# extra builds: some pre-defined build configurations
#***********************************************************************

define run_config_and_build
	$(call run_config,$1,$2)
	$(call run_build,$1,$2)
endef

# force building with clang/clang++
build/extra/clang/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang/%: export CC  = clang
build/extra/clang/%: export CXX = clang++

# force building with clang/clang++ -m32
build/extra/clang-m32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-m32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-m32/%: export CC  = clang -m32
build/extra/clang-m32/%: export CXX = clang++ -m32

# force building with clang/clang++ -mx32
build/extra/clang-mx32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-mx32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-mx32/%: export CC  = clang -mx32
build/extra/clang-mx32/%: export CXX = clang++ -mx32

# force building with clang/clang++ -m64
build/extra/clang-m64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-m64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-m64/%: export CC  = clang -m64
build/extra/clang-m64/%: export CXX = clang++ -m64

# force building with gcc/g++
build/extra/gcc/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc/%: export CC  = gcc
build/extra/gcc/%: export CXX = g++

# force building with gcc/g++ -m32
build/extra/gcc-m32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-m32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-m32/%: export CC  = gcc -m32
build/extra/gcc-m32/%: export CXX = g++ -m32

# force building with gcc/g++ -mx32
build/extra/gcc-mx32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-mx32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-mx32/%: export CC  = gcc -mx32
build/extra/gcc-mx32/%: export CXX = g++ -mx32

# force building with gcc/g++ -m64
build/extra/gcc-m64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-m64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-m64/%: export CC  = gcc -m64
build/extra/gcc-m64/%: export CXX = g++ -m64

# cross compiler: Linux glibc aarch64-linux-gnu
build/extra/cross-linux-aarch64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-linux-aarch64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-linux-aarch64/%: export CC  = aarch64-linux-gnu-gcc
build/extra/cross-linux-aarch64/%: export CXX = aarch64-linux-gnu-g++

# cross compiler: Linux glibc arm-linux-gnueabihf
build/extra/cross-linux-arm/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-linux-arm/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-linux-arm/%: export CC  = arm-linux-gnueabihf-gcc
build/extra/cross-linux-arm/%: export CXX = arm-linux-gnueabihf-g++ -Wno-psabi

# cross compiler: Windows x86 win32 MinGW
build/extra/cross-windows-mingw32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-windows-mingw32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-windows-mingw32/%: export CC  = i686-w64-mingw32-gcc
build/extra/cross-windows-mingw32/%: export CXX = i686-w64-mingw32-g++
# disable sanitize to avoid link errors with current MinGW-w64 versions
build/extra/cross-windows-mingw32/%: UPX_CMAKE_CONFIG_FLAGS += -DUPX_CONFIG_DISABLE_SANITIZE=1

# cross compiler: Windows x64 win64 MinGW
build/extra/cross-windows-mingw64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-windows-mingw64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-windows-mingw64/%: export CC  = x86_64-w64-mingw32-gcc
build/extra/cross-windows-mingw64/%: export CXX = x86_64-w64-mingw32-g++
# disable sanitize to avoid link errors with current MinGW-w64 versions
build/extra/cross-windows-mingw64/%: UPX_CMAKE_CONFIG_FLAGS += -DUPX_CONFIG_DISABLE_SANITIZE=1

#***********************************************************************
# check git submodules
#***********************************************************************

ifeq ($(wildcard ./vendor/boost-pfr/include/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/doctest/doctest/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/lzma-sdk/C/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/rangeless/include/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/ucl/include/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/valgrind/include/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/zlib/crc32.c),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
ifeq ($(wildcard ./vendor/zstd/lib/.),)
  $(error ERROR: missing git submodule; run 'git submodule update --init')
endif
