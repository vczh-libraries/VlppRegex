.PHONY: all clean pre-build
.DEFAULT_GOAL := all

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

./Bin/UnitTest:./Obj/Vlpp.o ./Obj/VlppOS.o ./Obj/Regex.o ./Obj/RegexAutomaton.o ./Obj/RegexData.o ./Obj/RegexExpression.o ./Obj/RegexParser.o ./Obj/RegexPure.o ./Obj/RegexRich.o ./Obj/RegexWriter.o ./Obj/TestRegex.o ./Obj/Main.o
	$(CPP_LINK)

./Obj/Vlpp.o: ../../Import/Vlpp.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/VlppOS.o: ../../Import/VlppOS.cpp ../../Import/VlppOS.h ../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/Regex.o: ../../Source/Regex/Regex.cpp ../../Source/Regex/Regex.h ../../Source/Regex/../../Import/Vlpp.h ../../Source/Regex/RegexExpression.h ../../Source/Regex/RegexData.h ../../Source/Regex/RegexAutomaton.h ../../Source/Regex/RegexPure.h ../../Source/Regex/RegexRich.h
	$(CPP_COMPILE)

./Obj/RegexAutomaton.o: ../../Source/Regex/RegexAutomaton.cpp ../../Source/Regex/RegexAutomaton.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/RegexData.o: ../../Source/Regex/RegexData.cpp ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/RegexExpression.o: ../../Source/Regex/RegexExpression.cpp ../../Source/Regex/RegexExpression.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h ../../Source/Regex/RegexAutomaton.h
	$(CPP_COMPILE)

./Obj/RegexParser.o: ../../Source/Regex/RegexParser.cpp ../../Source/Regex/RegexExpression.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h ../../Source/Regex/RegexAutomaton.h
	$(CPP_COMPILE)

./Obj/RegexPure.o: ../../Source/Regex/RegexPure.cpp ../../Source/Regex/RegexPure.h ../../Source/Regex/RegexAutomaton.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/RegexRich.o: ../../Source/Regex/RegexRich.cpp ../../Source/Regex/RegexRich.h ../../Source/Regex/RegexAutomaton.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h
	$(CPP_COMPILE)

./Obj/RegexWriter.o: ../../Source/Regex/RegexWriter.cpp ../../Source/Regex/RegexWriter.h ../../Source/Regex/RegexExpression.h ../../Source/Regex/RegexData.h ../../Source/Regex/../../Import/Vlpp.h ../../Source/Regex/RegexAutomaton.h
	$(CPP_COMPILE)

./Obj/TestRegex.o: ../Source/TestRegex.cpp ../Source/../../Source/Regex/RegexExpression.h ../Source/../../Source/Regex/RegexData.h ../Source/../../Source/Regex/../../Import/Vlpp.h ../Source/../../Source/Regex/RegexAutomaton.h ../Source/../../Source/Regex/RegexWriter.h ../Source/../../Source/Regex/RegexPure.h ../Source/../../Source/Regex/RegexRich.h ../Source/../../Source/Regex/Regex.h ../Source/../../Import/VlppOS.h
	$(CPP_COMPILE)

./Obj/Main.o: Main.cpp ../../Import/Vlpp.h
	$(CPP_COMPILE)
