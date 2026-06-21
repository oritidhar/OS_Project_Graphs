BUILD_DIR = build

.PHONY: all milestone1 milestone2 milestone3 milestone4 milestone5 milestone6 milestone7 clean
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
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(MAKE) -C $(BUILD_DIR) sim_m4
	cp $(BUILD_DIR)/sim_m4 ./sim

milestone5:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(MAKE) -C $(BUILD_DIR) sim_m5
	cp $(BUILD_DIR)/sim_m5 ./sim

milestone6:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(MAKE) -C $(BUILD_DIR) sim_m6
	cp $(BUILD_DIR)/sim_m6 ./sim

milestone7:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	$(MAKE) -C $(BUILD_DIR) sim_m7
	cp $(BUILD_DIR)/sim_m7 ./sim
	
clean:
	rm -rf $(BUILD_DIR) dijkstra sim sim_static

