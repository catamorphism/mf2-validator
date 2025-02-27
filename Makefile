all: mf2validate

rootdir = $(realpath .)
ICU_DIR=$(rootdir)/icu_release
DEBUG ?= 1
ifeq ($(DEBUG), 1)
	CXXFLAGS += -g
	ICUCONFIGUREFLAGS += --enable-debug --disable-release
else
	ICUCONFIGUREFLAGS += --enable-release --disable-debug
endif

CXX = clang++ $(CXXFLAGS)

mf2validate: mf2validate.cpp
	$(CXX) -Ithird_party -I$(ICU_DIR)/usr/local/include -L$(ICU_DIR)/usr/local/lib -licuuc -licudata -licui18n -o mf2validate mf2validate.cpp

test: mf2validate
	bash runTests.sh

clean:
	rm mf2validate

icu:
	echo "Cloning/building ICU; this takes a long time, but only needs to be done once"
	git clone https://github.com/unicode-org/icu.git $(ICU_DIR) || true
	mkdir -p $(ICU_DIR)/build
	cd $(ICU_DIR)/build; \
        ../icu4c/source/runConfigureICU Linux/clang $(ICUCONFIGUREFLAGS); \
	export DESTDIR=$(ICU_DIR); \
	make -j8 releaseDist; \
	cd ..


