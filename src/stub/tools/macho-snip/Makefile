# Required: Apple Command Line Developer Tools: "xcode-select --install"
#    Downloads a few megabytes.  Installs in a couple minutes.
# Required: Apple Xcode Developer Tools from Apple App Store.
#    Must accept the license:  Launch Xcode app,
#    or run "xcodebuild -license" from Terminal after install.
#    Downloads about 13GB (~20 minutes at 1Gb/s).  Install takes
#    ~45 minutes more on SSD, and ~40GB of space in the filesystem.

CFLAGS += -g

macho-snip: macho-snip.o
	$(CC) $(CFLAGS) -o macho-snip macho-snip.o

udf.o: udf.s
	gcc -c $<

# shortest main program;  used as an example for Apple Mach-O
udf: udf.o
	gcc --verbose -nostartfiles -o udf $<
	codesign --remove-signature udf
	strip -N udf  # removes all symbols
