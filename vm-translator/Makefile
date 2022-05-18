CC    = g++
FLAGS = -std=c++2a

VMTranslator: VMTranslator.o VMParser.o AsmMapper.o
	$(CC) $(FLAGS) -o VMTranslator VMTranslator.o VMParser.o AsmMapper.o

VMTranslator.o: VMTranslator.cc
	$(CC) $(FLAGS) -c VMTranslator.cc

VMParser.o: VMParser.cc VMParser.h
	$(CC) $(FLAGS) -c VMParser.cc

AsmMapper.o: AsmMapper.cc AsmMapper.h
	$(CC) $(FLAGS) -c AsmMapper.cc
