.PHONY: utest
utest:
	g++ -std=c++11 utest/UTestMain.cpp -o utest.exe \
	sfa/sv/utest/UTestElement.cpp \
	sfa/sv/utest/UTestStateVector.cpp \
	sfa/sv/utest/UTestRegion.cpp \
	sfa/utest/UTestConfigTokenizer.cpp \
	sfa/sv/utest/UTestStateVectorParser.cpp \
	sfa/sv/utest/UTestStateVectorConfigErrors.cpp \
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
	sfa/ConfigTokenizer.cpp \
	sfa/ConfigErrorInfo.cpp \
	sfa/util/MemoryOps.cpp \
	-I. \
	-lCppUTest

clean:
	rm *.exe
