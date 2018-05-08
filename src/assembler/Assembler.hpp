// Copyright 2016 Aleinik Vladislav
#ifndef HEADER_GUARD_MY_COMPILER_STD1_ASSEMBLER_HPP_INCLUDED
#define HEADER_GUARD_MY_COMPILER_STD1_ASSEMBLER_HPP_INCLUDED

// Includes:

#include <vector>
#include <string>
#include <map>

#include <algorithm>
#include <utility>

#include <boost/algorithm/string/predicate.hpp>

#include "../libs/MyException.hpp"
#include "../libs/FileWork_Old.hpp"
#include "../assembler_std/Standard1.hpp"

// Defines:

#define FILENAME "std1/Assembler.hpp"
#define PROGRAM_POS FILENAME, __FUNCTION__, __LINE__

// Code:
namespace AssemblerStd1
{
	using namespace MyExceptionCharStringRepresentation;

	namespace _preprocess
	{
		// Preprocessing: 

			std::vector<FileWork::Word> preprocessAndSeperate(const char* filename)
			{
				FileWork::ReadTextFile stream{filename};

				std::vector<FileWork::Word> toReturn{};

				bool inComment = false;
				size_t commentLine = 0;
				for (FileWork::Word curWord = stream.getWord(); curWord != FileWork::NULL_WORD; curWord = stream.getWord())
				{
					if (commentLine != curWord.line) inComment = false;

					if (!inComment) 
					{
						inComment = std::strstr(curWord.word, MyStd1::SINGLE_LINE_COMMENT) == curWord.word;
						commentLine = curWord.line;
					}

					if (!inComment)
					{
						toReturn.push_back(curWord);
					}

				}

				return toReturn;
			}

	} // namespace _preprocess

	namespace _additional
	{
		template <typename ToWrite>
		void writeToProgramme(std::vector<unsigned char>& programme, ToWrite toWrite)
		{
			unsigned char* bytes = reinterpret_cast<unsigned char*>(&toWrite);

			programme.insert(programme.end(), bytes, bytes + sizeof(toWrite));
		}

	} // namespace _additional

	namespace _registers
	{
		void writeAdressByWord(std::vector<unsigned char>& programme, const FileWork::Word& word)
		{
			using namespace MyStd1;
			using namespace MyStd1::_registers;

			for (RegAdr_t addrI = 0; addrI < REGISTER_COUNT; ++addrI)
			{
				if (boost::iequals(word.word, REGISTERS[addrI])) 
				{
					_additional::writeToProgramme<RegAdr_t>(programme, addrI);
					return;
				}
			}
			
			throw Exception("Unable to recognise adress", word.file, word.word, word.line);
		}

	} // namespace _address

	namespace _value
	{
		void writeValueByWord(std::vector<unsigned char>& programme, const FileWork::Word& word)
		{
			try
			{
				MyStd1::Val_t toRead = 0;

				std::sscanf(word.word, MyStd1::INPUT_FORMAT, &toRead);

				_additional::writeToProgramme<MyStd1::Val_t>(programme, toRead);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to recognise value", word.file, word.word, word.line, exc);
			}
		}

	} // namespace _value

	namespace _commandNum
	{
		void writeCmdNumByWord(std::vector<unsigned char>& programme, MyStd1::CmdNum_t cmdNum)
		{
			_additional::writeToProgramme<MyStd1::CmdNum_t>(programme, cmdNum);
		}

	} // namespace _commandNum

	namespace _nameTag
	{
		bool isNameTag(const FileWork::Word& word)
		{
			for (size_t i = 0; i < FileWork::MAX_WORD_SIZE; ++i)
			{
				if (word.word[i] == ':' && word.word[i+1] == '\0') return true;
			}

			return false;
		}

		std::string replaceColon(const FileWork::Word& word)
		{
			std::string toReturn{};

			size_t colonPos = 0;
			for (; colonPos < FileWork::MAX_WORD_SIZE && word.word[colonPos] != ':'; ++colonPos);

			toReturn.append(word.word, colonPos);

			return toReturn;
		}

		void insertNameTagsWhereNecessary
		(
			std::vector<unsigned char>& programme,
			std::map<std::string, std::vector<size_t>>& placesToInsertNameTag,
			std::map<std::string, MyStd1::CmdNum_t> nameTags
		)
		{
			for (auto& strArrPair : placesToInsertNameTag)
			{
				auto whatToInsert = nameTags.find(strArrPair.first);

				if (whatToInsert == nameTags.end())
				{
					throw Exception("Unable to find corresponding nametag", "", strArrPair.first.c_str(), 0);
				}

				MyStd1::CmdNum_t toInsert = whatToInsert->second;
				for (auto& placeToInsert : strArrPair.second)
				{
					unsigned char* bytes = reinterpret_cast<unsigned char*>(&toInsert);

					for (size_t curByte = 0; curByte < sizeof(MyStd1::CmdNum_t); ++curByte)
					{
						programme[placeToInsert + curByte] = bytes[curByte];
					}
				}
			}
		}

	}

	namespace _memory
	{
		void writeMemAddressByWord(std::vector<unsigned char>& programme, const FileWork::Word& word)
		{
			try
			{
				MyStd1::MemAdr_t toRead = 0;

				std::sscanf(word.word, "%hd", &toRead);

				_additional::writeToProgramme<MyStd1::MemAdr_t>(programme, toRead);
			}
			catch (std::exception& exc)
			{
				throw Exception("Unable to recognise memory address", word.file, word.word, word.line, exc);
			}
		}
	}

	namespace _command
	{
		void writeCmdByWord
		(
			std::vector<unsigned char>& programme,
			const std::vector<FileWork::Word>& words, 
			size_t& wordPos,
			std::map<std::string, std::vector<size_t>>& placesToInsertNameTag
		)
		{
			using namespace MyStd1::_command;

			// Linear search through CommandType array
			for (MyStd1::Cmd_t cmdI = 0; cmdI < COMMAND_COUNT; ++cmdI)
			{
				// case insensetive strcmp
				//  vvvvvvvvvvvvvv
				if (boost::iequals(words[wordPos].word, COMMANDS[cmdI].name.word))
				{
					_additional::writeToProgramme<MyStd1::Cmd_t>(programme, cmdI);

					// For assertion purposes
					MyStd1::Cmd_t cmdNamePos = wordPos;

					// Parsing command arguments
					for (auto argType : COMMANDS[cmdI].argTypes)
					{
						++wordPos;

						if (wordPos >= words.size())
						{
							throw Exception("Argument mismatch", words[cmdNamePos].file, COMMANDS[cmdI].name.word, words[cmdNamePos].line);
						}

						if (argType == ArgType::REGISTER_ADDRESS)
						{
							_registers::writeAdressByWord(programme, words[wordPos]);
						}
						else if (argType == ArgType::VALUE)
						{
							_value::writeValueByWord(programme, words[wordPos]);
						}
						else if (argType == ArgType::NAMETAG)
						{
							// Inserting current position as a place to insert
							auto& placeToInsert = placesToInsertNameTag[words[wordPos].word];

							placeToInsert.push_back(programme.size());

							_additional::writeToProgramme<MyStd1::CmdNum_t>(programme, 0);
						}
						else if (argType == ArgType::MEMORY_ADDRESS)
						{
							_memory::writeMemAddressByWord(programme, words[wordPos]);
						}
						else
						{
							throw Exception("Unexpected argType", PROGRAM_POS);
						}
					}

					return;
				}
			}	

			throw Exception("Unable to recognise command name", words[wordPos].file, words[wordPos].word, words[wordPos].line);	
		}

	} // namespace _command

	void assemble(const char* src, const char* dest)
	{
		FileWork::WriteBinaryFile stream{dest};

		// Appending Standard number and magical number
		stream.writeBytes(std::vector<unsigned char>(1, MyStd1::MAGIC_NUM));
		stream.writeBytes(std::vector<unsigned char>(1, MyStd1::STD_NUM));

		// Preprocessing 
		std::vector<FileWork::Word> words{_preprocess::preprocessAndSeperate(src)};

		// Place to store programme bytes
		std::vector<unsigned char> programme{};
		
		// NameTags support
		std::map<std::string, std::vector<size_t>> placesToInsertNameTag{};
		std::map<std::string, MyStd1::CmdNum_t> nameTags{}; 

		MyStd1::CmdNum_t curCmd = 0;

		// Going through preprocessed and seperated into words text
		for (size_t curWord = 0; curWord < words.size(); ++curWord)
		{
			if (_nameTag::isNameTag(words[curWord]))
			{
				// NameTags support
				std::string curNameTag = _nameTag::replaceColon(words[curWord]);  

				if (nameTags.find(curNameTag) != nameTags.end())
				{
					throw Exception("Two equivalent nametags found", src, words[curWord].word, words[curWord].line);
				}
				else
				{
					nameTags[curNameTag] = curCmd;
				}
			}
			else 
			{
				++curCmd;
				// Parsing command and its arguments
				_command::writeCmdByWord(programme, words, curWord, placesToInsertNameTag);
			}
		}

		// Appending END function to allow nametags at the end of the programme
		size_t zero = 0; // lvalue references to rvalue are forbidden
		                 // so i couldn't write writeCmdByWord(..., 0, ...);
		                 //                          there vvvv
		_command::writeCmdByWord(programme, {FileWork::Word("END")}, zero, placesToInsertNameTag);

		_nameTag::insertNameTagsWhereNecessary(programme, placesToInsertNameTag, nameTags);

		stream.writeBytes(programme);
	}
} // namespace AssemblerStd1

// Undefs:

#undef FILENAME
#undef PROGRAM_POS

#endif /*HEADER_GUARD_MY_COMPILER_STD1_ASSEMBLER_HPP_INCLUDED*/