cloc:
	cloc src

cmd = $(expr '(' $(date -d 2022/5/7 +%s) - $(date +%s) + 86399 ')' / 86400) " days left in semester"
deadline:
	@echo $(value cmd)

clean:
	rm -rf build

utest:
	rm -rf build
	mkdir build
	cd build && cmake .. && make utest && sudo ./utest -v

sfa:
	rm -rf build
	mkdir build
	cd build && cmake .. && make sfa

sfasup:
	rm -rf build
	mkdir build
	cd build && cmake .. && make sfasup

cppcheck:
	cppcheck src

black:
	black -l 80 ./

arduino-example:
	rm -rf build
	mkdir build
	-cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=../Arduino-CMake-Toolchain/Arduino-toolchain.cmake -DSFA_ARDUINO_MAC_ADDR=0xA8610AAE759C
	cd build && python3 ../examples/arduino/select_board.py
	cd build && cmake .. -DCMAKE_TOOLCHAIN_FILE=../Arduino-CMake-Toolchain/Arduino-toolchain.cmake -DSFA_ARDUINO_MAC_ADDR=0xA8610AAE759C && make arduino-example

.PHONY: ci
ci: cppcheck black utest sfa sfasup arduino-example clean cloc deadline
