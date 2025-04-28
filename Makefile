.PHONY: all build_dir configure build clean run test check

all: build_dir configure build

build_dir:
	@if [ ! -d "build" ]; then \
	    mkdir build; \
	fi

configure: build_dir
	@cd build && cmake ..

build: configure
	@cd build && make -j4

clean:
	@if [ -d "build" ]; then \
	    cd build && make clean; \
	fi

clean_logs:
	rm -r logs;
	mkdir logs;

run: build
	@if [ -n "$(ARGS)" ]; then \
	    ./build/popnet $(ARGS); \
	else \
	    echo "Error: No argument provided. Use 'make run ARGS=\"<ARGSname>\"'"; \
	    ./build/popnet -h;\
		echo "Running default test: make test"; \
	    ./build/popnet -JSON ./config.json; \
	fi

test:
	@if [ ! -e "build/popnet" ]; then \
		$(MAKE) build; \
		./build/popnet -JSON ./config.json; \
	else \
		./build/popnet -JSON ./config.json; \
	fi

check:
	@if [ ! -e "build/popnet" ]; then \
	    $(MAKE) build; \
	fi
	@if [ -n "$(ARGS)" ]; then \
	    valgrind --tool=memcheck --leak-check=full ./build/popnet $(ARGS) > /dev/null; \
	else \
	    echo "Error: No arguments provided. Use 'make run ARGS=\"<arguments>\"'"; \
	    echo "Running default test: make test"; \
	    valgrind --tool=memcheck --leak-check=full ./build/popnet -JSON ./config.json > /dev/null; \
	fi


