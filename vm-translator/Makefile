VMTranslator: VMTranslator.o VMParser.o AsmMapper.o
	g++ -o VMTranslator VMTranslator.o VMParser.o AsmMapper.o

VMTranslator.o: VMTranslator.cc
	g++ -c VMTranslator.cc

VMParser.o: VMParser.cc VMParser.h
	g++ -c VMParser.cc

AsmMapper.o: AsmMapper.cc AsmMapper.h
	g++ -c AsmMapper.cc
