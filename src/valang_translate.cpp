// Copyright 2016 Aleinik Vladislav
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <strstream>
#include <fstream>

#include "ast/RecursiveDescent.hpp"
#include "ast/AST_Print.hpp"
#include "asm_translation/AsmTranslation.hpp"

int main(int argc, const char* argv[])
{
	using namespace VaExc;

	try
	{
		if (argc != 3) throw Exception("Input pattern: valang_translate <src> <dest>"_msg);

		auto parser = VMPG::createParser(argv[1]);

		auto ast = VMPG::parsePg(parser);

		VlMathPG_AST::AsmTranslator translator;
		std::strstream assembled_stream;
		ast->translate(assembled_stream, translator);

		std::fstream file;
		file.open(argv[2], std::fstream::out);

		file << assembled_stream.rdbuf();

		file.close();
	}
	catch (VaExc::Exception& exc)
	{
		std::printf("%s\n", exc.what());
	}

	return EXIT_SUCCESS;
}