// Copyright 2016 Aleinik Vladislav
#ifndef HEADER_GUARD_CHAR_STRING_REPRESENTATION_EXCEPTION_HPP_INCLUDED
#define HEADER_GUARD_CHAR_STRING_REPRESENTATION_EXCEPTION_HPP_INCLUDED

#include <exception>
#include <cstdio>
#include <cstring>

namespace MyExceptionCharStringRepresentation
{
	namespace _detail
	{
		struct ErrorInfo
		{
		public:
			const char* msg;
			const char* file;
			const char* func;
			unsigned    line;
		};
	}

	struct Exception : std::exception
	{
	public:
		explicit Exception(const char* message, const char* file, const char* func, unsigned line) noexcept :
			errors_ 		{{message, file, func, line}},
			explanatoryStr_ ()
		{
			std::memset(explanatoryStr_, 0, 1000);
		}

		explicit Exception(const char* message, const char* file, const char* func, unsigned line, const Exception& causedBy) noexcept :
			Exception(message, file, func, line)
		{
			std::memcpy(errors_ + 1, causedBy.errors_, 4 * sizeof(_detail::ErrorInfo));
		}

		explicit Exception(const char* message, const char* file, const char* func, unsigned line, const std::exception& causedBy) noexcept :
			Exception(message, file, func, line)
		{
			errors_[1] = {causedBy.what(), "no info", "no info", 0};

			std::memset(errors_ + 2, 0, 3 * sizeof(_detail::ErrorInfo));
		}

		virtual const char* what() const noexcept override
		{
			for (size_t i = 0, writtenChars = 0; i < 5 && writtenChars < 1000; ++i)
			{
				if (errors_[i].msg  == nullptr) return explanatoryStr_;
				if (errors_[i].file == nullptr) return explanatoryStr_;
				if (errors_[i].func == nullptr) return explanatoryStr_;

				if (std::strlen(errors_[i].msg) + std::strlen(errors_[i].file) + std::strlen(errors_[i].func) > 150) return explanatoryStr_;

				int writtenNow = (i == 0)? 0 : std::sprintf(const_cast<char*>(explanatoryStr_) + writtenChars, "\nCaused by:\n");
				writtenChars += writtenNow;

				writtenNow = std::sprintf
				(
					const_cast<char*>(explanatoryStr_) + writtenChars,
					"Exception: %s\nIn file: %s\nIn function: %s\nIn line: %d\n",
					errors_[i].msg,
					errors_[i].file,
					errors_[i].func,
					errors_[i].line
				);

				if (writtenNow < 0) return explanatoryStr_;
				writtenChars += writtenNow;
			}

			return explanatoryStr_;
		}

	private:
		_detail::ErrorInfo errors_[5];
		char explanatoryStr_[1000];
	};

}

#endif /*HEADER_GUARD_CHAR_STRING_REPRESENTATION_EXCEPTION_HPP_INCLUDED*/
