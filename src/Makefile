# List of architectures to build for
ARCHS := x86_64 # aarch64 armv7l i686

# Build directory
BUILD_DIR := ../build

# Release directory
RELEASE_DIR := ../release

# compilers
CC_armv7l := arm-linux-gnueabihf-g++
CC_aarch64 := aarch64-linux-gnu-g++
CC_x86_64 := g++
CC_i686 := g++ -m32

# Build command
define build
	mkdir -p $(BUILD_DIR)/$(1)
	$(CC_$(1)) --std=c++17 cpp/main.cpp -o $(BUILD_DIR)/$(1)/successor;
endef

# Release command
define release
	mkdir -p $(RELEASE_DIR)
	cp ../LICENSE $(BUILD_DIR)/$(1)/
	cp ../README.md $(BUILD_DIR)/$(1)/
	cp -r scripts/* $(BUILD_DIR)/$(1)/
	ls $(BUILD_DIR)/$(1)/
	tar -czf $(RELEASE_DIR)/successor-$(1).tar.gz -C $(BUILD_DIR)/$(1)/ .;
endef

.PHONY: clean build test release

build:
	mkdir -p $(BUILD_DIR)
	$(foreach arch,$(ARCHS),$(call build,$(arch)))

test:
	mkdir -p $(BUILD_DIR)
	$(CC_x86_64) --std=c++17 cpp/tests/all.cpp -o $(BUILD_DIR)/test
	$(BUILD_DIR)/test

clean:
	rm -rf $(BUILD_DIR) $(RELEASE_DIR)

release: build
	$(foreach arch,$(ARCHS),$(call release,$(arch)))
	