# Directory definitions
BUILD_DIR = bin/
VPATH = $(shell find src -type d)

# File definitions
SRC_FILES = $(shell find src -name *.cc -printf "%f ")
OBJ_FILES = $(addprefix $(BUILD_DIR), $(SRC_FILES:.cc=.o))
BIN_FILE = simulate_cache
CONFIGS_FILE = configs.txt
RESULTS_FILE = results.csv

# Compiler flag definition
CPPFLAGS = -Wall -g $(addprefix -I, $(VPATH))

# Incremental build
all: $(BIN_FILE)

$(BIN_FILE): $(OBJ_FILES)
	$(CXX) -o $(BIN_FILE) $(OBJ_FILES) -lm

$(BUILD_DIR)%.o: %.cc
	@mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CPPFLAGS) -o $@ $<

# Build from scratch
rebuild: clean all

# Incremental build & process trace file
%.bin: $(BIN_FILE)
	./$(BIN_FILE) $(CONFIGS_FILE) $@ > $(RESULTS_FILE)

# Remove build and run results
clean:
	rm -fr $(BUILD_DIR) $(BIN_FILE) $(RESULTS_FILE)
