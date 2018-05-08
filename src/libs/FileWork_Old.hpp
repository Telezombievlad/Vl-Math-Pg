// Copyright 2016 Aleinik Vladislav
#ifndef HEADER_GUARD_MY_COMPILER_FILE_WORK_HPP_INCLUDED
#define HEADER_GUARD_MY_COMPILER_FILE_WORK_HPP_INCLUDED

// Includes:

#include <cstdio>
#include <cstring>

#include <cctype>
#include <utility>

#include <vector>

#include "MyException.hpp"

// Defines:

#define FILENAME "standard/Standard0.hpp"
#define PROGRAM_POS FILENAME, __FUNCTION__, __LINE__

// Code:

namespace FileWork
{
	// Namespaces:

	using namespace MyExceptionCharStringRepresentation;

	// Constants:

	const size_t MAX_WORD_SIZE = 30;
	const size_t CHUNK_SIZE = 1024;

	// Single word:

		struct Word
		{
		public:
			// Variables:
				const char* file;
				size_t line;
				size_t col;
				char word[MAX_WORD_SIZE + 1]; // Must have \0

			// Ctor:

				explicit Word(const char* newWord) :
					file (""),
					line (0),
					col  (0),
					word ()
				{
					std::strcpy(word, newWord);
				}

			// Operator!=:
				bool operator!=(Word that) const
				{
					if (file != that.file) return true;
					if (line != that.line) return true;
					if (col  != that.col ) return true;
					if (std::strcmp(word, that.word) != 0) return true;

					return false;
				}
		};

		Word NULL_WORD{Word("")};

	// Reading from file:
	// (Text file)

		class ReadTextFile
		{
		public:
			// Ctors && dtor:
				explicit ReadTextFile(const char* filename);

				~ReadTextFile();

			// Functions:
				Word getWord();

		private:
			// Variables:
				const char* filename_;
				size_t line_;
				size_t col_;
				std::FILE* file_;
				size_t size_;
				char buf_[CHUNK_SIZE]; // Without a \0
				size_t index_;
		};

		// Ctor && dtor:
			ReadTextFile::ReadTextFile(const char* filename) :
				filename_ (filename),
				line_     (1),
				col_      (0),
				file_     (std::fopen(filename, "r")),
				size_     (0),
				buf_      (),
				index_    (0)
			{
				if (file_ == nullptr)
				{
					throw Exception("Unable to open file", filename, "", 0);
				}

				std::memset(buf_, 0, sizeof(buf_));

				size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
			}

			ReadTextFile::~ReadTextFile()
			{
				std::fclose(file_);
			}

		// Other func:
			Word ReadTextFile::getWord()
			{	
				Word toReturn{NULL_WORD};

				bool inWord = false;
				for (size_t wordIndex = 0; wordIndex < MAX_WORD_SIZE; ++index_, ++col_)
				{
					if (index_ == size_)
					{
						std::memset(buf_, 0, sizeof(buf_));

						size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);

						index_ = 0;

						if (size_ == 0) // File finished
						{
							break;
						}
					}

					if (buf_[index_] == '\n')
					{
						++index_;
						++line_;
						col_ = 0;
						if (inWord) break;
					}

					if (std::isspace(buf_[index_]) != 0)
					{
						if (inWord) break;
					}
					else if (!inWord)
					{
						inWord = true;

						toReturn.file = filename_;
						toReturn.line = line_;
						toReturn.col = col_;
					}
					
					if (inWord)
					{
						toReturn.word[wordIndex] = buf_[index_];
						++wordIndex;
					}
				}

				return toReturn;
			}

	// Reading from file:
	// (Binary file)

		class ReadBinaryFile
		{
		public:
			// Ctors && dtor:
				explicit ReadBinaryFile(const char* filename);

				~ReadBinaryFile();

			// Functions:
				std::vector<unsigned char> getBytes(size_t count);

				bool finished() const;

		private:
			// Variables:
				const char* filename_;
				std::FILE* file_;
				size_t size_;
				unsigned char buf_[CHUNK_SIZE];
				size_t index_;
		};

		// Ctor && dtor:
			ReadBinaryFile::ReadBinaryFile(const char* filename) :
				filename_ (filename),
				file_     (std::fopen(filename, "rb")),
				size_     (0),
				buf_      (),
				index_    (0)
			{
				if (file_ == nullptr)
				{
					throw Exception("Unable to open file", filename, "", 0);
				}

				std::memset(buf_, 0, sizeof(buf_));

				size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
			}

			ReadBinaryFile::~ReadBinaryFile()
			{
				std::fclose(file_);
			}

		// Other func:
			std::vector<unsigned char> ReadBinaryFile::getBytes(size_t count)
			{	
				std::vector<unsigned char> toReturn = {};

				for (size_t bytesRead = 0; ; ++index_, ++bytesRead)
				{
					if (index_ == size_)
					{
						std::memset(buf_, 0, sizeof(buf_));

						size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);

						index_ = 0;

						if (size_ == 0) // File finished
						{
							if (bytesRead < count)
							{
								throw Exception("Unable to read enough bytes from file", filename_, "", 0);
							}

							break;
						}
					}

					if (bytesRead == count) break;

					toReturn.push_back(buf_[index_]);
				}

				return toReturn;
			}

			bool ReadBinaryFile::finished() const
			{
				if (size_ == 0) return true;

				return false;
			}

	// Writing to file:
	// (Text file)

		class WriteTextFile
		{
		public:
			// Ctor && dtor:
				explicit WriteTextFile(const char* filename);

				~WriteTextFile();

			// Other func:
				void writeWord(      Word&& toWrite);
				void writeWord(const Word&  toWrite);

		private:
			// Variables:
				const char* filename_;
				std::FILE* file_;
				char buf_[CHUNK_SIZE];
				size_t size_;
		};

		// Ctor && dtor:
			WriteTextFile::WriteTextFile(const char* filename) :
				filename_ (filename),
				file_ (std::fopen(filename, "w")),
				buf_  (),
				size_ (0)
			{
				std::memset(buf_, 0, sizeof(buf_));

				if (file_ == nullptr)
				{
					throw Exception("Unable to create file", filename, 0, 0);;
				}
			}

			WriteTextFile::~WriteTextFile()
			{
				std::fwrite(buf_, sizeof(*buf_), size_, file_);

				std::fclose(file_);
			}

		// Other func:
			void WriteTextFile::writeWord(Word&& toWrite)
			{
				for 
				(
					size_t wordIndex = 0; 
					 wordIndex < MAX_WORD_SIZE && toWrite.word[wordIndex] != '\0'; 
					 ++wordIndex, ++size_
				)
				{
					if (size_ == CHUNK_SIZE)
					{
						size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

						std::memset(buf_, 0, sizeof(buf_));

						size_ = 0;

						if (written != CHUNK_SIZE)
						{
							throw Exception("Unable to write to file", filename_, "", 0);
						}
					}

					buf_[size_] = toWrite.word[wordIndex];
				}
			}

			void WriteTextFile::writeWord(const Word& toWrite)
			{
				for 
				(
					size_t wordIndex = 0; 
					 wordIndex < MAX_WORD_SIZE && toWrite.word[wordIndex] != '\0'; 
					 ++wordIndex, ++size_
				)
				{
					if (size_ == CHUNK_SIZE)
					{
						size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

						std::memset(buf_, 0, sizeof(buf_));

						size_ = 0;

						if (written != CHUNK_SIZE)
						{
							throw Exception("Unable to write to file", filename_, "", 0);
						}
					}

					buf_[size_] = toWrite.word[wordIndex];
				}
			}

	// Writing to file:
	// (Binary file)

		class WriteBinaryFile
		{
		public:
			// Ctor && dtor:
				explicit WriteBinaryFile(const char* filename);

				~WriteBinaryFile();

			// Other func:
				void writeBytes(const std::vector<unsigned char>& toWrite);

		private:
			// Variables:
				const char* filename_;
				std::FILE* file_;
				unsigned char buf_[CHUNK_SIZE];
				size_t size_;
		};

		// Ctor && dtor:
			WriteBinaryFile::WriteBinaryFile(const char* filename) :
				filename_ (filename),
				file_ (std::fopen(filename, "wb")),
				buf_  (),
				size_ (0)
			{
				std::memset(buf_, 0, sizeof(buf_));

				if (file_ == nullptr)
				{
					throw Exception("Unable to create file", filename, 0, 0);;
				}
			}

			WriteBinaryFile::~WriteBinaryFile()
			{
				std::fwrite(buf_, sizeof(*buf_), size_, file_);

				std::fclose(file_);
			}

		// Other func:
			void WriteBinaryFile::writeBytes(const std::vector<unsigned char>& toWrite)
			{
				for (size_t vectIndex = 0; vectIndex < toWrite.size(); ++vectIndex, ++size_)
				{
					if (size_ == CHUNK_SIZE)
					{
						size_t written = std::fwrite(buf_, sizeof(*buf_), sizeof(buf_), file_);

						std::memset(buf_, 0, sizeof(buf_));

						size_ = 0;

						if (written != CHUNK_SIZE)
						{
							throw Exception("Unable to write to file", filename_, "", 0);
						}
					}

					buf_[size_] = toWrite.at(vectIndex);
				}
			}

} // namespace FileWork 

#undef FILENAME
#undef PROGRAM_POS

#endif /*HEADER_GUARD_MY_COMPILER_FILE_WORK_HPP_INCLUDED*/