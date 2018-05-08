// Copyright 2016 Aleinik Vladislav
#ifndef HEADER_GUARD_MY_COMPILER_CPU_HPP_INCLUDED
#define HEADER_GUARD_MY_COMPILER_CPU_HPP_INCLUDED

// Includes:

#include "../libs/MyException.hpp"
#include "../libs/FileWork_Old.hpp"
#include "../libs/Stack.hpp"

#include "../assembler_std/Standard1.hpp"

// Defines:

#define FILENAME "std1/CPU.hpp"
#define PROGRAM_POS FILENAME, __FUNCTION__, __LINE__

// Code:

namespace EmulatedProcessorStd1
{
	using namespace MyExceptionCharStringRepresentation;     
	using namespace MyStackStaticArrayRepresentation;    

	namespace _additional
	{
		template <typename ToRead> 
		ToRead getFromBinaryFile(FileWork::ReadBinaryFile& stream)
		{
			std::vector<unsigned char> bytes{stream.getBytes(sizeof(ToRead))};

			ToRead* data = reinterpret_cast<ToRead*>(bytes.data());

			return *data;
		}

	} // namespace _additional

	namespace _address
	{
		MyStd1::RegAdr_t getAddress(FileWork::ReadBinaryFile& stream)
		{
			try
			{
				return _additional::getFromBinaryFile<MyStd1::RegAdr_t>(stream);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to read register address from executable", PROGRAM_POS, exc);
			}
		}

	} // namespace _address

	namespace _value
	{
		MyStd1::Val_t getValue(FileWork::ReadBinaryFile& stream)
		{
			try
			{
				return _additional::getFromBinaryFile<MyStd1::Val_t>(stream);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to read value from executable", PROGRAM_POS, exc);
			}
		}

	} // namespace _value

	namespace _commandNumber
	{
		MyStd1::CmdNum_t getCommandNumber(FileWork::ReadBinaryFile& stream)
		{
			try
			{
				return _additional::getFromBinaryFile<MyStd1::CmdNum_t>(stream);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to read command number from executable", PROGRAM_POS, exc);
			}
		}

	} // namespace _commandNumber

	namespace _memory
	{
		MyStd1::MemAdr_t getMemoryAddress(FileWork::ReadBinaryFile& stream)
		{
			try
			{
				return _additional::getFromBinaryFile<MyStd1::MemAdr_t>(stream);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to read memory address from executable", PROGRAM_POS, exc);
			}
		}

	} // namespace _memory

	namespace _command
	{
		MyStd1::Cmd_t getCommand(FileWork::ReadBinaryFile& stream)
		{
			try
			{
				return _additional::getFromBinaryFile<MyStd1::Cmd_t>(stream);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to read command from executable", PROGRAM_POS, exc);
			}
		}

		//-----------------------------------------------------------------------------

		MyStd1::_command::Command* commandFromFile(FileWork::ReadBinaryFile& stream, MyStd1::_command::CPU& cpu)
		{
			using namespace _value;
			using namespace _address;
			using namespace _commandNumber;
			using namespace MyStd1::_command;

			switch (getCommand(stream))
			{
				case  0: 
				{
					if (cpu.curCmd == 0) cpu.curCmd = cpu.cmdArr.size();
					return new CmdBeg();
				}
				case  1: return new CmdEnd();
				case  2: return new CmdPush (getValue(stream));
				case  3: return new CmdPushR(getAddress(stream));
				case  4: return new CmdPop();
				case  5: return new CmdPopR (getAddress(stream));
				case  6: return new CmdAdd();                        
				case  7: return new CmdSub();                        
				case  8: return new CmdMul();                        
				case  9: return new CmdDiv();                        
				case 10: return new CmdSqrt(); 
				case 11: return new CmdOut();                       
				case 12: return new CmdIn();                        
				case 13: return new CmdJmp (getCommandNumber(stream));
				case 14: return new CmdJe  (getCommandNumber(stream));
				case 15: return new CmdJne (getCommandNumber(stream));
				case 16: return new CmdJa  (getCommandNumber(stream));
				case 17: return new CmdJae (getCommandNumber(stream));
				case 18: return new CmdJb  (getCommandNumber(stream));
				case 19: return new CmdJbe (getCommandNumber(stream));
				case 20: return new CmdCall(getCommandNumber(stream));
				case 21: return new CmdRet();
				case 22: return new CmdDump();
				case 23: return new CmdPrint();
				case 24: return new CmdIsL();
				case 25: return new CmdIsLE();
				case 26: return new CmdIsM();
				case 27: return new CmdIsME();
				case 28: return new CmdIsE();
				case 29: return new CmdIsNE();
				case 30: return new CmdAnd();
				case 31: return new CmdOr(); 
				case 32: return new CmdPushMem(_memory::getMemoryAddress(stream));
				case 33: return new  CmdPopMem(_memory::getMemoryAddress(stream));                   
				default: throw Exception("Unknown command number", PROGRAM_POS);
			}
		}

	} // namespace _command

	void execute(const char* filename)
	{
		FileWork::ReadBinaryFile stream{filename};

		MyStd1::_command::CPU cpu{};

		try
		{
			if (_additional::getFromBinaryFile<MyStd1::MagicNum_t>(stream) != MyStd1::MAGIC_NUM)
			{
				throw Exception("Unknown file format", filename, "-", 0);
			}

			if (_additional::getFromBinaryFile<MyStd1::StdNum_t>(stream) != MyStd1::STD_NUM)
			{
				throw Exception("Unknown standard", filename, "-", 0);
			}

			while (!stream.finished())
			{
				cpu.cmdArr.push_back(_command::commandFromFile(stream, cpu));
			}
		}
		catch (Exception& exc)
		{
			throw Exception("Unable to read file", filename, "-", 0, exc);
		}

		try
		{
			for (; cpu.curCmd < cpu.cmdArr.size(); ++cpu.curCmd)
			{
				cpu.cmdArr.at(cpu.curCmd)->execute(cpu);
			}
		}
		catch (Exception& exc)
		{
			throw Exception("Runtime error", filename, "-", 0, exc);
		}
	}

} // EmulatedProcessorStd0

#undef FILENAME
#undef PROGRAM_POS

#endif /*HEADER_GUARD_MY_COMPILER_CPU_HPP_INCLUDED*/