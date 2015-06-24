SRC_DIR		= $(CURDIR)/src
BUILD_DIR	= $(CURDIR)/build
DECOMPILER	= $(BUILD_DIR)/nocode/nocode

.PHONY: all
all: tags build

.PHONY: build
build: $(BUILD_DIR)/build.ninja
	cmake --build $(BUILD_DIR)

$(BUILD_DIR)/build.ninja:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake -G Ninja $(SRC_DIR)

.PHONY: test
test: build
	tests/configure.py --decompiler $(DECOMPILER) $(BUILD_DIR)/tests
	ninja -C $(BUILD_DIR)/tests -k 100

.PHONY: tags
tags:
	-ctags --c++-kinds=+p --fields=+iaS --extra=+q -R $(SRC_DIR)

.PHONY: doxydoc
doxydoc:
	doxygen

gitstats: .git
	gitstats . gitstats

.PHONY: clean
clean:
	rm -f tags gmon.out core core.* vgcore.* .ycm_extra_conf.pyc
	-cmake --build $(BUILD_DIR) --target clean
	-$(MAKE) -C doc clean

.PHONY:
distclean: clean
	rm -rf $(BUILD_DIR) doxydoc gitstats
