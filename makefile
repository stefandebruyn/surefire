.PHONY: utest
utest:
	g++ -std=c++11 utest/UTestMain.cpp -o utest.exe \
	sfa/statevec/utest/UTestElement.cpp \
	sfa/statevec/utest/UTestStateVector.cpp \
	sfa/statevec/utest/UTestConfigTokenizer.cpp \
	sfa/statevec/utest/UTestStateVectorParser.cpp \
	sfa/statevec/Element.cpp \
	sfa/statevec/StateVector.cpp \
	sfa/statevec/StateVectorParser.cpp \
	sfa/util/MemoryOps.cpp \
	sfa/ConfigTokenizer.cpp \
	-I. \
	-lCppUTest

clean:
	rm *.exe
