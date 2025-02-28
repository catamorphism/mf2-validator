all: mf2validate

rootdir = $(realpath .)
ICU_DIR=$(rootdir)/icu_release
DEBUG ?= 0
CXXFLAGS += -std=c++20

ifeq ($(DEBUG), 1)
	CXXFLAGS += -g -DDEBUG
	ICUCONFIGUREFLAGS += --enable-debug --disable-release
else
	ICUCONFIGUREFLAGS += --enable-release --disable-debug
endif

CXX = clang++ $(CXXFLAGS)
CLANGVERSION = $(shell bash getclangversion.sh)

mf2validate: mf2validate.cpp checkversion
	$(CXX) -Ithird_party -I$(ICU_DIR)/usr/local/include -L$(ICU_DIR)/usr/local/lib -licuuc -licudata -licui18n -o mf2validate mf2validate.cpp

.PHONY: checkversion
.SILENT: checkversion
checkversion:
	if [ $(CLANGVERSION) -lt 17 ]; then \
		echo "Error: Found Clang version $(CLANGVERSION). Version 17 or greater is required." ; \
		exit 1; \
	fi

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


