// Copyright 2018 Vladislav Aleinik
// TokenizerFileReader class opens a file and reads tokens in a stream-like fashion
#ifndef VL_MATH_PG_TOKENIZER_FILE_READER
#define VL_MATH_PG_TOKENIZER_FILE_READER

#include "../libs/VaException.hpp"

#include <cstdio>
#include <cstring>
#include <regex>

namespace TokenizeFReader
{
	using namespace VaExc;

	const size_t MAX_TOKEN_SIZE = 63;
	const size_t CHUNK_SIZE = 1024;

	struct CodePos
	{
	public:
		const char* file;
		size_t line;
		size_t col;
	};

	struct Token
	{
	public:
		// Variables:
		CodePos pos;
		char token[MAX_TOKEN_SIZE + 1];
		unsigned short type;

		// Ctor:
		Token() : pos (), token (), type (0)
		{
			std::memset(token, '\0', sizeof(token));
		}

		Token& operator=(const Token& that)
		{
			if (&that == this) return *this;

			pos  = that.pos;
			type = that.type;

			std::memcpy(token, that.token, sizeof(token));

			return *this;
		}

		inline bool is(const char* str) const
		{
			return std::strcmp(token, str) == 0;
		}

		inline bool is(unsigned short expectedType) const
		{
			return type == expectedType;
		}
	};

	struct TokenRegex
	{
	public:
		TokenRegex(const char* str) : 
			regex_ (std::regex(str))
		{}

		// Returns an iterator to the r+1 of matching sequence or nullptr if no match found
		const char* fit(const char* left, const char* right) const
		{
			std::cmatch results;
			if (!std::regex_search(left, right, results, regex_, std::regex_constants::match_continuous)) return nullptr;

			if (results[0].first == results[0].second) return nullptr;

			return results[0].second;
		}

	private:
		std::regex regex_;
	};

	class TokenizerFileReader
	{
	private:
		const char* filename_;
		size_t line_;
		size_t col_;

		std::FILE* file_;

		char buf_[CHUNK_SIZE]; // Without a \0
		size_t size_;
		size_t index_;

		std::vector<TokenRegex> patterns_;

	public:
		explicit TokenizerFileReader(const char* filename, std::vector<TokenRegex> tokenPatterns) :
			filename_ (filename),
			line_     (0),
			col_      (0),
			file_     (std::fopen(filename, "r")),
			buf_      (),
			size_     (0),
			index_    (0),
			patterns_ (tokenPatterns)
		{
			if (file_ == nullptr)
			{
				throw Exception(ArgMsg("TokenizerFileReader::ctor(): Unable to open file %s", filename_));
			}

			std::memset(buf_, 0, sizeof(buf_));

			size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
		}

		~TokenizerFileReader()
		{
			std::fclose(file_);
		}

		bool finished()
		{
			if (index_ < size_) return false;

			size_ = std::fread(buf_, sizeof(*buf_), sizeof(buf_), file_);
			index_ = 0;
			
			return size_ == 0;
		}

		Token getToken()
		{
			Token toReturn{};

			// Try to upload more if needed and possible 
			if (size_ == CHUNK_SIZE && index_ + MAX_TOKEN_SIZE > size_)
			{
				for (size_t l = 0, r = index_; r < size_; ++l, ++r) buf_[l] = buf_[r];

				size_t charsRead = std::fread(buf_ + size_ - index_, sizeof(*buf_), index_, file_);

				size_ = charsRead + size_ - index_;
				index_ = 0;
			}

			const char* right = std::min(buf_ + index_ + MAX_TOKEN_SIZE, buf_ + size_);
			for (size_t type = 0; type < patterns_.size(); ++type)
			{
				const char* curFit = patterns_[type].fit(buf_ + index_, right);
				if (curFit == nullptr) continue;

				toReturn.pos = {filename_, line_, col_};
				toReturn.type = type;

				for (size_t tokenI = 0; buf_ + index_ < curFit; ++tokenI, ++index_)
				{
					char curChar = *(buf_ + index_);

					toReturn.token[tokenI] = curChar;

					if (curChar == '\n')
					{
						++line_;
						col_ = 0;
					}
					else ++col_;
				}

				return toReturn;
			}

			throw Exception(ArgMsg("[%s %04zu %03hu] TokenizerFileReader::getToken(): Unknown token",
			                filename_, line_, col_));
		}
	};
}

#endif  // VL_MATH_PG_TOKENIZER_FILE_READER