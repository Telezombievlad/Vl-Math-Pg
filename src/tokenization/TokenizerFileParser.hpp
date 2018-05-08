// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_TOKENIZER_FILE_PARSER
#define VL_MATH_PG_TOKENIZER_FILE_PARSER

#include <limits>

#include "TokenizerFileReader.hpp"

namespace TokenizeFParser
{
	using namespace TokenizeFReader;

	// Set of language constructs that really set up the language token system
	std::vector<TokenizeFParser::TokenRegex> LANG_CONSTRUCTS = 
	{
		TokenRegex("failmatch^"),
		TokenRegex("[ \t\n]+"),
		TokenRegex("#[a-z_#]+"),
		TokenRegex("[a-z][a-zA-Z0-9_]*"),
		TokenRegex("[A-Z][a-zA-Z0-9]*"),
		TokenRegex("[!%%&*+\\-./:<=>?@^|~]+"),
		TokenRegex("-?(0|[1-9][0-9]*)(\\.[0-9]+)?"),
		TokenRegex("[\\(\\)\\{\\}]"),
		TokenRegex(","),
		TokenRegex(";")
	};

	enum LangConstructsTypes : unsigned short
	{
		UNDEFINED        = 0,
		SPACE            = 1,
		PREPROCESSOR_CMD = 2,
		VARIABLE         = 3, 
		TYPE             = 4,
		OPERATOR         = 5, 
		NUMBER           = 6,
		BRACKET          = 7,
		COMMA            = 8,
		SEMICOLON        = 9
	};

	class TokenizerFileParser
	{
	private:
		TokenizerFileReader reader_;
		Token cur_;
		bool finished_;
		size_t lastCommentLine_;

		void eatSpacesAndComments()
		{
			if (reader_.finished())
			{
				finished_ = true;
				return;
			}

			do
			{
				cur_ = reader_.getToken();

				if (cur_.is(PREPROCESSOR_CMD)) lastCommentLine_ = cur_.pos.line;
			}
			while (cur_.is(SPACE) || cur_.is(PREPROCESSOR_CMD) || lastCommentLine_ == cur_.pos.line);
		}

	public:
		TokenizerFileParser(const char* filename, std::vector<TokenRegex> tokenPatterns) :
			reader_ (TokenizerFileReader(filename, tokenPatterns)),
			cur_ (),
			finished_ (false),
			lastCommentLine_ (std::numeric_limits<size_t>::max())
		{
			eatSpacesAndComments();
		}

		bool finished() const
		{
			return finished_;
		}

		const Token& peek() const
		{
			if (finished_)
			{
				throw Exception("TokenizeFileParser::peek(): Can't peek when parsing is finished"_msg, VAEXC_POS);
			}

			return cur_;
		}

		Token move()
		{
			if (finished_)
			{
				throw Exception("TokenizeFileParser::move(): Can't move when parsing is finished"_msg, VAEXC_POS);
			}

			Token toReturn = cur_;
		
			eatSpacesAndComments();

			return toReturn;
		}
	};
}

namespace VMPG
{
	TokenizeFParser::TokenizerFileParser createParser(const char* filename)
	{
		return TokenizeFParser::TokenizerFileParser(filename, TokenizeFParser::LANG_CONSTRUCTS);
	}
}

#endif // VL_MATH_PG_TOKENIZER_FILE_PARSER
