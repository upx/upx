#
# UPX top-level Makefile - needs GNU make and CMake >= 3.13
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

ifeq ($(UPX_MAKEFILE_EXTRA_MK_INCLUDED),)
override UPX_MAKEFILE_EXTRA_MK_INCLUDED := 1

override check_defined   = $(foreach 1,$1,$(if $(filter undefined,$(origin $1)),$(error ERROR: variable '$1' is not defined),))
override check_undefined = $(foreach 1,$1,$(if $(filter undefined,$(origin $1)),,$(error ERROR: variable '$1' is already defined)))
$(call check_defined,run_config run_build)
$(call check_undefined,run_config_and_build)

#***********************************************************************
# build and test
#***********************************************************************

CTEST = ctest

build/debug+test:     $$(dir $$@)debug PHONY; cd "$(dir $@)debug" && $(CTEST)
build/%/debug+test:   $$(dir $$@)debug PHONY; cd "$(dir $@)debug" && $(CTEST)
build/release+test:   $$(dir $$@)release PHONY; cd "$(dir $@)release" && $(CTEST)
build/%/release+test: $$(dir $$@)release PHONY; cd "$(dir $@)release" && $(CTEST)
build/%/all+test:     $$(dir $$@)debug+test $$(dir $$@)release+test PHONY ;

# shortcuts
debug+test:   build/debug+test PHONY
release+test: build/release+test PHONY
all+test build/all+test: build/debug+test build/release+test PHONY

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
build/extra/clang-m32/%: export CC  = clang   -m32
build/extra/clang-m32/%: export CXX = clang++ -m32

# force building with clang/clang++ -mx32
build/extra/clang-mx32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-mx32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-mx32/%: export CC  = clang   -mx32
build/extra/clang-mx32/%: export CXX = clang++ -mx32

# force building with clang/clang++ -m64
build/extra/clang-m64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-m64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-m64/%: export CC  = clang   -m64
build/extra/clang-m64/%: export CXX = clang++ -m64

# force building with clang/clang++ -static
build/extra/clang-static/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-static/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-static/%: export CC  = clang   -static
build/extra/clang-static/%: export CXX = clang++ -static

# force building with clang/clang++ -static-pie
build/extra/clang-static-pie/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-static-pie/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-static-pie/%: export CC  = clang   -static-pie -fPIE -Wno-unused-command-line-argument
build/extra/clang-static-pie/%: export CXX = clang++ -static-pie -fPIE -Wno-unused-command-line-argument

# force building with clang/clang++ -static -flto
build/extra/clang-static-lto/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-static-lto/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-static-lto/%: export CC  = clang   -static -flto
build/extra/clang-static-lto/%: export CXX = clang++ -static -flto

# force building with clang/clang++ C17/C++20
build/extra/clang-std-cxx20/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-std-cxx20/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-std-cxx20/%: export CC  = clang   -std=gnu17
build/extra/clang-std-cxx20/%: export CXX = clang++ -std=gnu++20
build/extra/clang-std-cxx20/%: export UPX_CONFIG_DISABLE_C_STANDARD=ON
build/extra/clang-std-cxx20/%: export UPX_CONFIG_DISABLE_CXX_STANDARD=ON

# force building with clang/clang++ C23/C++23
build/extra/clang-std-cxx23/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/clang-std-cxx23/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/clang-std-cxx23/%: export CC  = clang   -std=gnu2x
build/extra/clang-std-cxx23/%: export CXX = clang++ -std=gnu++2b
build/extra/clang-std-cxx23/%: export UPX_CONFIG_DISABLE_C_STANDARD=ON
build/extra/clang-std-cxx23/%: export UPX_CONFIG_DISABLE_CXX_STANDARD=ON

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

# force building with gcc/g++ -static
build/extra/gcc-static/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-static/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-static/%: export CC  = gcc -static
build/extra/gcc-static/%: export CXX = g++ -static

# force building with gcc/g++ -static-pie
build/extra/gcc-static-pie/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-static-pie/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-static-pie/%: export CC  = gcc -static-pie -fPIE
build/extra/gcc-static-pie/%: export CXX = g++ -static-pie -fPIE

# force building with gcc/g++ -static -flto
build/extra/gcc-static-lto/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-static-lto/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-static-lto/%: export CC  = gcc -static -flto
build/extra/gcc-static-lto/%: export CXX = g++ -static -flto

# force building with gcc/g++ C17/C++20
build/extra/gcc-std-cxx20/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-std-cxx20/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-std-cxx20/%: export CC  = gcc -std=gnu17
build/extra/gcc-std-cxx20/%: export CXX = g++ -std=gnu++20
build/extra/gcc-std-cxx20/%: export UPX_CONFIG_DISABLE_C_STANDARD=ON
build/extra/gcc-std-cxx20/%: export UPX_CONFIG_DISABLE_CXX_STANDARD=ON

# force building with gcc/g++ C23/C++23
build/extra/gcc-std-cxx23/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/gcc-std-cxx23/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/gcc-std-cxx23/%: export CC  = gcc -std=gnu2x
build/extra/gcc-std-cxx23/%: export CXX = g++ -std=gnu++2b
build/extra/gcc-std-cxx23/%: export UPX_CONFIG_DISABLE_C_STANDARD=ON
build/extra/gcc-std-cxx23/%: export UPX_CONFIG_DISABLE_CXX_STANDARD=ON

# cross compiler: Linux glibc aarch64-linux-gnu (arm64)
build/extra/cross-linux-gnu-aarch64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-linux-gnu-aarch64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-linux-gnu-aarch64/%: export CC  = aarch64-linux-gnu-gcc
build/extra/cross-linux-gnu-aarch64/%: export CXX = aarch64-linux-gnu-g++
build/extra/cross-linux-gnu-aarch64/%: CMAKE_SYSTEM_NAME ?= Linux
build/extra/cross-linux-gnu-aarch64/%: CMAKE_CROSSCOMPILING_EMULATOR ?= qemu-aarch64

# cross compiler: Linux glibc arm-linux-gnueabihf
build/extra/cross-linux-gnu-arm-eabihf/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-linux-gnu-arm-eabihf/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-linux-gnu-arm-eabihf/%: export CC  = arm-linux-gnueabihf-gcc
build/extra/cross-linux-gnu-arm-eabihf/%: export CXX = arm-linux-gnueabihf-g++ -Wno-psabi
build/extra/cross-linux-gnu-arm-eabihf/%: CMAKE_SYSTEM_NAME ?= Linux
build/extra/cross-linux-gnu-arm-eabihf/%: CMAKE_CROSSCOMPILING_EMULATOR ?= qemu-arm

# cross compiler: Windows x86 win32 MinGW (i386)
build/extra/cross-windows-mingw32/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-windows-mingw32/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-windows-mingw32/%: export CC  = i686-w64-mingw32-gcc -static -D_WIN32_WINNT=0x0501
build/extra/cross-windows-mingw32/%: export CXX = i686-w64-mingw32-g++ -static -D_WIN32_WINNT=0x0501
build/extra/cross-windows-mingw32/%: CMAKE_SYSTEM_NAME ?= Windows
build/extra/cross-windows-mingw32/%: CMAKE_SYSTEM_PROCESSOR ?= X86
build/extra/cross-windows-mingw32/%: CMAKE_CROSSCOMPILING_EMULATOR ?= wine

# cross compiler: Windows x64 win64 MinGW (amd64)
build/extra/cross-windows-mingw64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-windows-mingw64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-windows-mingw64/%: export CC  = x86_64-w64-mingw32-gcc -static -D_WIN32_WINNT=0x0501
build/extra/cross-windows-mingw64/%: export CXX = x86_64-w64-mingw32-g++ -static -D_WIN32_WINNT=0x0501
build/extra/cross-windows-mingw64/%: CMAKE_SYSTEM_NAME ?= Windows
build/extra/cross-windows-mingw64/%: CMAKE_SYSTEM_PROCESSOR ?= AMD64
build/extra/cross-windows-mingw64/%: CMAKE_CROSSCOMPILING_EMULATOR ?= wine64

# cross compiler: macOS arm64 (aarch64)
build/extra/cross-darwin-arm64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-darwin-arm64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-darwin-arm64/%: export CC  = clang   -target arm64-apple-darwin
build/extra/cross-darwin-arm64/%: export CXX = clang++ -target arm64-apple-darwin
build/extra/cross-darwin-arm64/%: CMAKE_SYSTEM_NAME ?= Darwin
build/extra/cross-darwin-arm64/%: CMAKE_SYSTEM_PROCESSOR ?= arm64

# cross compiler: macOS x86_64 (amd64)
build/extra/cross-darwin-x86_64/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/extra/cross-darwin-x86_64/release: PHONY; $(call run_config_and_build,$@,Release)
build/extra/cross-darwin-x86_64/%: export CC  = clang   -target x86_64-apple-darwin
build/extra/cross-darwin-x86_64/%: export CXX = clang++ -target x86_64-apple-darwin
build/extra/cross-darwin-x86_64/%: CMAKE_SYSTEM_NAME ?= Darwin
build/extra/cross-darwin-x86_64/%: CMAKE_SYSTEM_PROCESSOR ?= x86_64

#***********************************************************************
# C/C++ static analyzers
#***********************************************************************

# force building with clang Static Analyzer (scan-build)
SCAN_BUILD = scan-build
build/analyze/clang-analyzer/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/analyze/clang-analyzer/release: PHONY; $(call run_config_and_build,$@,Release)
build/analyze/clang-analyzer/%: override CMAKE := $(SCAN_BUILD) $(CMAKE)
build/analyze/clang-analyzer/%: export CCC_CC  ?= clang
build/analyze/clang-analyzer/%: export CCC_CXX ?= clang++

# run clang-tidy: uses file compile_commands.json from an existing clang build
# does not create any actual files, so purely PHONY
RUN_CLANG_TIDY = time python3 ./misc/analyze/clang-tidy/run-clang-tidy.py -p $<
RUN_CLANG_TIDY_WERROR = $(RUN_CLANG_TIDY) '-warnings-as-errors=*'
build/analyze/clang-tidy-upx/debug build/analyze/clang-tidy-upx/release: build/extra/clang/$$(notdir $$@) PHONY
	$(RUN_CLANG_TIDY_WERROR) -config-file ./.clang-tidy '/src/.*\.cpp'
build/analyze/clang-tidy-bzip2/debug build/analyze/clang-tidy-bzip2/release: build/extra/clang/$$(notdir $$@) PHONY
	$(RUN_CLANG_TIDY)        -config-file ./misc/analyze/clang-tidy/clang-tidy-bzip2.yml /vendor/bzip2/
build/analyze/clang-tidy-ucl/debug build/analyze/clang-tidy-ucl/release: build/extra/clang/$$(notdir $$@) PHONY
	$(RUN_CLANG_TIDY_WERROR) -config-file ./misc/analyze/clang-tidy/clang-tidy-ucl.yml   /vendor/ucl/
build/analyze/clang-tidy-zlib/debug build/analyze/clang-tidy-zlib/release: build/extra/clang/$$(notdir $$@) PHONY
	$(RUN_CLANG_TIDY_WERROR) -config-file ./misc/analyze/clang-tidy/clang-tidy-zlib.yml  /vendor/zlib/
build/analyze/clang-tidy-zstd/debug build/analyze/clang-tidy-zstd/release: build/extra/clang/$$(notdir $$@) PHONY
	$(RUN_CLANG_TIDY)        -config-file ./misc/analyze/clang-tidy/clang-tidy-zstd.yml  /vendor/zstd/
build/analyze/clang-tidy/debug build/analyze/clang-tidy/release: build/analyze/clang-tidy-upx/$$(notdir $$@)
build/analyze/clang-tidy/debug build/analyze/clang-tidy/release: build/analyze/clang-tidy-bzip2/$$(notdir $$@)
build/analyze/clang-tidy/debug build/analyze/clang-tidy/release: build/analyze/clang-tidy-ucl/$$(notdir $$@)
build/analyze/clang-tidy/debug build/analyze/clang-tidy/release: build/analyze/clang-tidy-zlib/$$(notdir $$@)
build/analyze/clang-tidy/debug build/analyze/clang-tidy/release: build/analyze/clang-tidy-zstd/$$(notdir $$@)

# OLD names [deprecated]
build/extra/scan-build/debug:   build/analyze/clang-analyzer/debug
build/extra/scan-build/release: build/analyze/clang-analyzer/release

#***********************************************************************
# advanced: generic eXtra target
#***********************************************************************

# usage:
#   make UPX_XTARGET=my-target CC="my-cc -flags" CXX="my-cxx -flags"
#   make UPX_XTARGET=my-target CC="my-cc -flags" CXX="my-cxx -flags" xtarget/debug

ifneq ($(UPX_XTARGET),)
ifneq ($(CC),)
ifneq ($(CXX),)

UPX_XTARGET := $(UPX_XTARGET)
build/$(UPX_XTARGET)/debug:   PHONY; $(call run_config_and_build,$@,Debug)
build/$(UPX_XTARGET)/release: PHONY; $(call run_config_and_build,$@,Release)
build/$(UPX_XTARGET)/%: export CC  := $(CC)
build/$(UPX_XTARGET)/%: export CXX := $(CXX)
# shortcuts
xtarget/all:     xtarget/debug xtarget/release PHONY
xtarget/debug:   build/$(UPX_XTARGET)/debug PHONY
xtarget/release: build/$(UPX_XTARGET)/release PHONY
xtarget/all+test:     xtarget/debug+test xtarget/release+test PHONY
xtarget/debug+test:   build/$(UPX_XTARGET)/debug+test PHONY
xtarget/release+test: build/$(UPX_XTARGET)/release+test PHONY
# set new default
.DEFAULT_GOAL := build/$(UPX_XTARGET)/release

endif
endif
endif

#***********************************************************************
# assemble cmake config flags; useful for CI jobs
#
# info: by default CMake only honors the CC and CXX environment variables; make
# it easy to set other variables like CMAKE_AR or CMAKE_RANLIB
#***********************************************************************

ifneq ($(origin UPX_CMAKE_CONFIG_FLAGS),command line) # GNU make bug work-around
# GNU make bug, see https://savannah.gnu.org/bugs/index.php?64822
# and commit https://git.savannah.gnu.org/cgit/make.git/commit/?id=07187db947ba25e6c59b55f10660a04f8e9c5229

$(call check_undefined,__add_cmake_config)
__add_cmake_config = $(and $($1),-D$1="$($1)")
# pass common CMake settings from environment/make to cmake
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_VERBOSE_MAKEFILE)
# pass common CMake toolchain settings from environment/make to cmake
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_ADDR2LINE)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_AR)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_DLLTOOL)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_LINKER)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_NM)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_OBJCOPY)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_OBJDUMP)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_RANLIB)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_READELF)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_STRIP)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_TAPI)
# pass common CMake LTO toolchain settings from environment/make to cmake (for use with "-flto")
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_C_COMPILER_AR)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_C_COMPILER_RANLIB)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_CXX_COMPILER_AR)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_CXX_COMPILER_RANLIB)
# pass common CMake cross compilation settings from environment/make to cmake
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_SYSTEM_NAME)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_SYSTEM_PROCESSOR)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,CMAKE_CROSSCOMPILING_EMULATOR)
# pass UPX config options from environment/make to cmake; see CMakeLists.txt
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_DISABLE_GITREV)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_DISABLE_SANITIZE)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_DISABLE_WSTRICT)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_DISABLE_WERROR)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_DISABLE_SELF_PACK_TEST)
# pass UPX extra compile options from environment/make to cmake; see CMakeLists.txt
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_EXTRA_COMPILE_OPTIONS_BZIP2)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_EXTRA_COMPILE_OPTIONS_UCL)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_EXTRA_COMPILE_OPTIONS_UPX)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_EXTRA_COMPILE_OPTIONS_ZLIB)
build/%: UPX_CMAKE_CONFIG_FLAGS += $(call __add_cmake_config,UPX_CONFIG_EXTRA_COMPILE_OPTIONS_ZSTD)

endif # GNU make bug work-around

#***********************************************************************
# check git submodules
#***********************************************************************

SUBMODULES = doctest lzma-sdk ucl valgrind zlib

$(foreach 1,$(SUBMODULES),$(if $(wildcard vendor/$1/[CL]*),,\
    $(error ERROR: missing git submodule '$1'; run 'git submodule update --init')))

endif # UPX_MAKEFILE_EXTRA_MK_INCLUDED
