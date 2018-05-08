// Copyright 2016 Aleinik Vladislav
#ifndef HEADER_GUARD_NODE_REPRESENTATION_STACK_HPP_INCLUDED
#define HEADER_GUARD_NODE_REPRESENTATION_STACK_HPP_INCLUDED

#include "MyException.hpp"

#define PROGRAM_POS __FILE__, __FUNCTION__, __LINE__

namespace MyStackStaticArrayRepresentation
{
	namespace MyException = MyExceptionCharStringRepresentation;

	namespace _detail
	{
		enum class Guard : size_t
		{
			DESTRUCTED = 0xDE6D3ECD,
			GUARD0 	   = 0x12052016,
			GUARD1     = 0xFEEDDCAD,
			GUARD2     = 0xFABACABA
		};
	}

	template <class T, size_t stackSize_>
	class Stack
	{
	private:
		// Variables:
			_detail::Guard guard0_;
			size_t elementCount_;
			_detail::Guard guard1_;
			T stack_[stackSize_];
			_detail::Guard guard2_;

	public:
		// Ctors && dtors:
			Stack() :
				guard0_       (_detail::Guard::GUARD0),
				elementCount_ (0),
				guard1_ 	  (_detail::Guard::GUARD1),
				stack_        (),
				guard2_       (_detail::Guard::GUARD2)
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}
			}

			Stack(const Stack& that) = default;

			Stack(Stack&& that) = default;

			~Stack()
			{
				if (guard0_ == _detail::Guard::DESTRUCTED) return;
				if (guard1_ == _detail::Guard::DESTRUCTED) return;
				if (guard2_ == _detail::Guard::DESTRUCTED) return;
				
				guard0_ = _detail::Guard::DESTRUCTED;
				guard1_ = _detail::Guard::DESTRUCTED;
				guard2_ = _detail::Guard::DESTRUCTED;
			}

		// Operator=:
			Stack& operator=(const Stack&  that) = default;
			Stack& operator=(      Stack&& that) = default;

		// Getters and headers:
			// Strong ExcG
			const T& head() const
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (elementCount_ == 0)
				{
					throw MyException::Exception("Stack is empty", PROGRAM_POS);
				}

				return stack_[elementCount_ - 1];
			}

			// Strong ExcG
			T& head()
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (elementCount_ == 0)
				{
					throw MyException::Exception("Stack is empty", PROGRAM_POS);
				}

				return stack_[elementCount_ - 1];
			}

			T& at(size_t index)
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (index >= stackSize_)
				{
					throw MyException::Exception("Access out of stack", PROGRAM_POS);
				}

				return stack_[index];
			}

			size_t filledSize() const
			{
				return elementCount_;
			}

			// Strong ExcG
			bool empty() const
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				return elementCount_ == 0;
			}

			// Strong ExcG
			bool full() const
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				return elementCount_ == stackSize_;
			}

		// Push && pop:
			// Strong ExcG
			Stack& push(const T& toPush)
			{	
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (elementCount_ == stackSize_)
				{
					throw MyException::Exception("Stack is full", PROGRAM_POS);
				}

				size_t elementCount = elementCount_;

				try
				{
					stack_[elementCount_] = toPush;
				}
				catch (const std::exception& exception)
				{
					elementCount_ = elementCount;
					guard0_ = _detail::Guard::GUARD0;
					guard1_ = _detail::Guard::GUARD1;
					guard2_ = _detail::Guard::GUARD2;					

					throw MyException::Exception("Exception caught while pushing", PROGRAM_POS, exception);
				}
				catch (...)
				{
					elementCount_ = elementCount;
					guard0_ = _detail::Guard::GUARD0;
					guard1_ = _detail::Guard::GUARD1;
					guard2_ = _detail::Guard::GUARD2;					

					throw MyException::Exception("Exception caught while pushing", PROGRAM_POS);
				}

				elementCount_++;

				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				return *this;
			}

			// Strong ExcG
			Stack& push(T&& toPush)
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (elementCount_ == stackSize_)
				{
					throw MyException::Exception("Stack is full", PROGRAM_POS);
				}

				size_t elementCount = elementCount_;

				try
				{
					stack_[elementCount_] = std::move(toPush);
				}
				catch (const std::exception& exception)
				{
					elementCount_ = elementCount;
					guard0_ = _detail::Guard::GUARD0;
					guard1_ = _detail::Guard::GUARD1;
					guard2_ = _detail::Guard::GUARD2;					

					throw MyException::Exception("Exception caught while pushing", PROGRAM_POS, exception);
				}
				catch (...)
				{
					elementCount_ = elementCount;
					guard0_ = _detail::Guard::GUARD0;
					guard1_ = _detail::Guard::GUARD1;
					guard2_ = _detail::Guard::GUARD2;					

					throw MyException::Exception("Exception caught while pushing", PROGRAM_POS);
				}

				elementCount_++;

				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				return *this;
			}

			// Strong ExcG
			T pop()
			{
				try { throwIfNotOk(); }
				catch (const MyException::Exception& exception)
				{
					throw MyException::Exception("Assertion failed", PROGRAM_POS, exception);
				}

				if (elementCount_ == 0)
				{
					throw MyException::Exception("Stack is empty", PROGRAM_POS);
				}

				elementCount_--;

				return stack_[elementCount_];
			}

		// Debugging:
			// Strong ExcG
			void throwIfNotOk() const
			{
				if (guard0_       == _detail::Guard::DESTRUCTED ||
					guard1_       == _detail::Guard::DESTRUCTED ||
					guard2_       == _detail::Guard::DESTRUCTED)
					throw MyException::Exception("Destructor has been called", PROGRAM_POS);


				if (guard0_ != _detail::Guard::GUARD0) 
					throw MyException::Exception("Guard 0 is dead, stack was touched and might be spoilt", PROGRAM_POS);

				if (guard1_ != _detail::Guard::GUARD1) 
					throw MyException::Exception("Guard 1 is dead, the stack was touched and might be spoilt", PROGRAM_POS);
				
				if (guard2_ != _detail::Guard::GUARD2) 
					throw MyException::Exception("Guard 2 is dead, the stack was touched and might be spoilt", PROGRAM_POS);

				if (elementCount_ > stackSize_)
					throw MyException::Exception("Stack has more elements than its size provides, the stack was touched", PROGRAM_POS); 
			}
	};
}

#undef PROGRAM_POS

#endif /*HEADER_GUARD_NODE_REPRESENTATION_STACK_HPP_INCLUDED*/