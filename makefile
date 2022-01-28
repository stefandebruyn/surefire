.PHONY: utest
utest:
	g++ -std=c++11 utest/UTestMain.cpp -o utest.exe \
	sfa/sv/utest/UTestElement.cpp \
	sfa/sv/utest/UTestRegion.cpp \
	sfa/sv/utest/UTestStateVectorAccess.cpp \
	sfa/sv/utest/UTestStateVectorCreate.cpp \
	sfa/sv/utest/UTestStateVectorParserParse.cpp \
	sfa/sv/utest/UTestStateVectorParserError.cpp \
	sfa/sv/Element.cpp \
	sfa/sv/Region.cpp \
	sfa/sv/StateVector.cpp \
	sfa/sv/StateVectorParser.cpp \
	sfa/sm/StateMachine.cpp \
	sfa/sm/ExpressionNode.hpp \
	sfa/sm/TransitionAction.cpp \
	sfa/sm/IAction.cpp \
	sfa/sm/utest/UTestStateMachineCreate.cpp \
	sfa/sm/utest/UTestStateMachineStep.cpp \
	sfa/sm/utest/UTestExpressionNode.cpp \
	sfa/sm/utest/UTestTransitionAction.cpp \
	sfa/sm/utest/UTestAssignmentAction.cpp \
	sfa/task/ITask.cpp \
	sfa/task/utest/UTestITask.cpp \
	sfa/config/ConfigTokenizer.cpp \
	sfa/config/ConfigErrorInfo.cpp \
	sfa/config/utest/UTestConfigTokenizer.cpp \
	sfa/util/MemOps.cpp \
	sfa/util/utest/UTestMemOps.cpp \
	psl/linux/Socket.cpp \
	pal/utest/UTestUdpSocket.cpp \
	-I. \
	-Iutest \
	-lCppUTest

clean:
	rm *.exe
