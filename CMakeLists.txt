cmake_minimum_required(VERSION 3.16)

project(sfa)

set(CMAKE_CXX_STANDARD 17)

add_executable(utest-linux
    "sfa/sv/Element.cpp"
    "sfa/sv/Region.cpp"
    "sfa/sv/StateVector.cpp"
    "sfa/sv/StateVectorParser.cpp"
    "sfa/sv/utest/UTestElement.cpp"
    "sfa/sv/utest/UTestRegion.cpp"
    "sfa/sv/utest/UTestStateVectorAccess.cpp"
    "sfa/sv/utest/UTestStateVectorCreate.cpp"
    "sfa/sv/utest/UTestStateVectorParserParse.cpp"
    "sfa/sv/utest/UTestStateVectorParserError.cpp"
    "sfa/sm/StateMachine.cpp"
    "sfa/sm/TransitionAction.cpp"
    "sfa/sm/IAction.cpp"
    "sfa/sm/utest/UTestStateMachineCreate.cpp"
    "sfa/sm/utest/UTestStateMachineStep.cpp"
    "sfa/sm/utest/UTestExpressionNode.cpp"
    "sfa/sm/utest/UTestTransitionAction.cpp"
    "sfa/sm/utest/UTestAssignmentAction.cpp"
    "sfa/task/ITask.cpp"
    "sfa/task/utest/UTestITask.cpp"
    "sfa/config/ConfigTokenizer.cpp"
    "sfa/config/ConfigErrorInfo.cpp"
    "sfa/config/utest/UTestConfigTokenizer.cpp"
    "sfa/util/MemOps.cpp"
    "sfa/util/utest/UTestMemOps.cpp"
    "psl/linux/Socket.cpp"
    "psl/linux/Console.cpp"
    "psl/linux/Clock.cpp"
    "psl/linux/Thread.cpp"
    "psl/linux/ThreadPriorityRanges.cpp"
    "psl/linux/Spinlock.cpp"
    "pal/utest/UTestUdpSocket.cpp"
    "pal/utest/UTestSocketSelect.cpp"
    "pal/utest/UTestClock.cpp"
    "pal/utest/UTestConsole.cpp"
    "pal/utest/UTestThread.cpp"
    "pal/utest/UTestThreadFair.cpp"
    "pal/utest/UTestThreadRealTime.cpp"
    "pal/utest/UTestSpinlock.cpp"
    "utest/UTestMain.cpp"
)

target_link_libraries(utest-linux CppUTest pthread)
target_include_directories(utest-linux PRIVATE . utest)
