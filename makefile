cloc:
	cloc src

cmd = $(expr '(' $(date -d 2022/5/7 +%s) - $(date +%s) + 86399 ')' / 86400) " days left in semester"
deadline:
	@echo $(value cmd)

utest:
	rm -rf build
	mkdir build
	cd build && cmake .. && make utest && ./utest -v

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

.PHONY: ci
ci: cppcheck utest sfa sfasup cloc deadline
