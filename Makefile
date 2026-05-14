BUILD_DIR = build

.PHONY: all milestone1 milestone2 milestone3 milestone4 clean

all: milestone1 milestone2

$(BUILD_DIR)/Makefile:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

milestone1: $(BUILD_DIR)/Makefile
	$(MAKE) -C $(BUILD_DIR) dijkstra
	cp $(BUILD_DIR)/dijkstra ./dijkstra

milestone2:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	make -C build sim_static
	cp build/sim_static ./sim_static

milestone3:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	make -C build sim
	cp build/sim ./sim

milestone4:
	cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	make -C build sim_m4
	cp build/sim_m4 ./sim

clean:
	rm -rf $(BUILD_DIR) dijkstra sim
