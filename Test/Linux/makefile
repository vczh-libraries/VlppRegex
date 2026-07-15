.PHONY: all clean pre-build
.DEFAULT_GOAL := all

CPP_COMPILE_OPTIONS=-I ../../Import
include $(VCPROOT)/vl/makefile-cpp

pre-build:
	if ! [ -d ./Bin ]; then mkdir ./Bin; fi
	if ! [ -d ./Obj ]; then mkdir ./Obj; fi
	if ! [ -d ./Coverage ]; then mkdir ./Coverage; fi
	if ! [ -d ../Output ]; then mkdir ../Output; fi

clean:
	if [ -d ./Bin ]; then rm -r ./Bin; fi
	if [ -d ./Obj ]; then rm -r ./Obj; fi
	if [ -d ./Coverage ]; then rm -r ./Coverage; fi
	if [ -d ./../Output ]; then rm -r ../Output; fi

all:pre-build ./Bin/UnitTest

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/Vlpp.Linux.o ./Obj/VlppOS.o ./Obj/VlppOS.Linux.o ./Obj/RegexExpression.o ./Obj/RegexExpression_CanTreatAsPure.o ./Obj/RegexExpression_CharSet.o ./Obj/RegexExpression_GenerateEpsilonNfa.o ./Obj/RegexExpression_HasNoExtension.o ./Obj/RegexExpression_IsEqual.o ./Obj/RegexParser.o ./Obj/RegexWriter.o ./Obj/RegexAutomaton.o ./Obj/Regex.o ./Obj/RegexPure.o ./Obj/RegexRich.o ./Obj/TestAutomaton.o ./Obj/TestColorizer.o ./Obj/TestExtendProc.o ./Obj/TestLexer.o ./Obj/TestParser.o ./Obj/TestPure.o ./Obj/TestRegex.o ./Obj/TestRich.o ./Obj/TestWalker.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp
	$(CPP_COMPILE)

./Obj/Vlpp.Linux.o: ../../Import/Vlpp.Linux.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp
	$(CPP_COMPILE)

./Obj/VlppOS.Linux.o: ../../Import/VlppOS.Linux.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression.o: ../../Source/Regex/AST/RegexExpression.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression_CanTreatAsPure.o: ../../Source/Regex/AST/RegexExpression_CanTreatAsPure.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression_CharSet.o: ../../Source/Regex/AST/RegexExpression_CharSet.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression_GenerateEpsilonNfa.o: ../../Source/Regex/AST/RegexExpression_GenerateEpsilonNfa.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression_HasNoExtension.o: ../../Source/Regex/AST/RegexExpression_HasNoExtension.cpp
	$(CPP_COMPILE)

./Obj/RegexExpression_IsEqual.o: ../../Source/Regex/AST/RegexExpression_IsEqual.cpp
	$(CPP_COMPILE)

./Obj/RegexParser.o: ../../Source/Regex/AST/RegexParser.cpp
	$(CPP_COMPILE)

./Obj/RegexWriter.o: ../../Source/Regex/AST/RegexWriter.cpp
	$(CPP_COMPILE)

./Obj/RegexAutomaton.o: ../../Source/Regex/Automaton/RegexAutomaton.cpp
	$(CPP_COMPILE)

./Obj/Regex.o: ../../Source/Regex/Regex.cpp
	$(CPP_COMPILE)

./Obj/RegexPure.o: ../../Source/Regex/RegexPure.cpp
	$(CPP_COMPILE)

./Obj/RegexRich.o: ../../Source/Regex/RegexRich.cpp
	$(CPP_COMPILE)

./Obj/TestAutomaton.o: ../Source/TestAutomaton.cpp
	$(CPP_COMPILE)

./Obj/TestColorizer.o: ../Source/TestColorizer.cpp
	$(CPP_COMPILE)

./Obj/TestExtendProc.o: ../Source/TestExtendProc.cpp
	$(CPP_COMPILE)

./Obj/TestLexer.o: ../Source/TestLexer.cpp
	$(CPP_COMPILE)

./Obj/TestParser.o: ../Source/TestParser.cpp
	$(CPP_COMPILE)

./Obj/TestPure.o: ../Source/TestPure.cpp
	$(CPP_COMPILE)

./Obj/TestRegex.o: ../Source/TestRegex.cpp
	$(CPP_COMPILE)

./Obj/TestRich.o: ../Source/TestRich.cpp
	$(CPP_COMPILE)

./Obj/TestWalker.o: ../Source/TestWalker.cpp
	$(CPP_COMPILE)

./Obj/Main.o: Main.cpp
	$(CPP_COMPILE)
