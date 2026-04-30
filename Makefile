BUILD_DIR = build

.PHONY: all milestone1 milestone2 clean

all: milestone1 milestone2

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

milestone1: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) dijkstra
	cp $(BUILD_DIR)/dijkstra ./dijkstra

milestone2: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) sim
	cp $(BUILD_DIR)/sim ./sim

clean:
	rm -rf $(BUILD_DIR) dijkstra sim
