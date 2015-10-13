SRC_DIR		= $(CURDIR)/src
BUILD_DIR	!= echo /tmp/snowman.`echo $(CURDIR) | sha1sum | awk '{print $$1}'`
BUILD_DIR_LINK	= $(CURDIR)/build
BUILD_SCRIPT	= $(BUILD_DIR)/build.ninja
DECOMPILER	= $(BUILD_DIR)/nocode/nocode
TEST_DIR	= $(BUILD_DIR)/tests
TEST_SCRIPT	= $(TEST_DIR)/build.ninja

.PHONY: all
all: tags build

.PHONY: build
build: $(BUILD_DIR)/build.ninja
	cmake --build $(BUILD_DIR)

$(BUILD_DIR) $(BUILD_DIR_LINK):
	mkdir -m 700 $(BUILD_DIR)
	ln -s $(BUILD_DIR) $(BUILD_DIR_LINK)

$(BUILD_SCRIPT): $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake -G Ninja $(SRC_DIR)

.PHONY: test
test: build $(TEST_SCRIPT)
	ninja -C $(TEST_DIR) -k 100

.PHONY: check
check: build $(TEST_SCRIPT)
	ninja -C $(TEST_DIR) -t clean
	ninja -C $(TEST_DIR) -k 100 check

.PHONY: update-tests
update-answers: check
	ninja -C $(TEST_DIR) update

$(TEST_SCRIPT):
	tests/configure.py --decompiler $(DECOMPILER) $(TEST_DIR)

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
	rm -rf $(TEST_DIR)

.PHONY:
distclean: clean
	rm -rf $(BUILD_DIR) $(BUILD_DIR_LINK) doxydoc gitstats
