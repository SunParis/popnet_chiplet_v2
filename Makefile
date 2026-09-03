.PHONY: all configure build test run check clean clean_logs

BUILD_DIR ?= build
BUILD_TYPE ?= Release

all: build

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DPOPNET_BUILD_TESTS=ON

build: configure
	cmake --build $(BUILD_DIR) --parallel

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

run: build
	@if [ -n "$(ARGS)" ]; then \
		./$(BUILD_DIR)/popnet $(ARGS); \
	else \
		./$(BUILD_DIR)/popnet -JSON ./config.json; \
	fi

check:
	cmake -S . -B $(BUILD_DIR)-san -DCMAKE_BUILD_TYPE=Debug \
		-DPOPNET_BUILD_TESTS=ON -DPOPNET_ENABLE_SANITIZERS=ON
	cmake --build $(BUILD_DIR)-san --parallel
	ctest --test-dir $(BUILD_DIR)-san --output-on-failure

clean:
	cmake --build $(BUILD_DIR) --target clean

clean_logs:
	find logs -mindepth 1 -maxdepth 1 -type f -delete
