// Copyright 2016 Aleinik Vladislav
#include <cstdlib>
#include <iostream>

#include "virt_proc_execute/CPU.hpp"

#define FILENAME "std1/valang.cpp"
#define PROGRAM_POS FILENAME, __FUNCTION__, __LINE__

using namespace MyExceptionCharStringRepresentation;

int main(int argc, const char* argv[])
{
	try
	{
		if (argc != 2)
		{
			throw Exception("Input pattern: valang_execute <src>", PROGRAM_POS);
		}

		EmulatedProcessorStd1::execute(argv[1]);
	}
	catch (Exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}	

	return EXIT_SUCCESS;
}

#undef FILENAME
#undef PROGRAM_POS