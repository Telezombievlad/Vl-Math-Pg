// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_OPERATOR_CONSTRUCTS_PARSER
#define VL_MATH_PG_OPERATOR_CONSTRUCTS_PARSER

#include <list>
#include <functional>
#include <utility>
#include <memory>

#include "AST.hpp"
#include "../tokenization/TokenizerFileParser.hpp"

namespace VlMathPG_OperatorConstructsParser
{
	using namespace VlMathPG_AST;
	using namespace TokenizeFParser;

	class OperatorParser
	{
	private:
		using Priorities = std::vector<std::pair<std::list<Operator>, OpType>>;
		using LowerParsingFunc = std::function<std::shared_ptr<Node>(TokenizerFileParser&)>;

		Priorities precedence_;
		LowerParsingFunc callAfter_;

		bool bracketsIncluded_;
		Operator lBr_;
		Operator rBr_;

	public:
		OperatorParser(
			Priorities precedence,
			LowerParsingFunc callAfter, 
			bool bracketsIncluded,
			Operator lBr, Operator rBr
		) :
			precedence_ (precedence),
			callAfter_ (callAfter),
			bracketsIncluded_ (bracketsIncluded),
			lBr_ (lBr),
			rBr_ (rBr)
		{}

		std::shared_ptr<Node> parse(TokenizerFileParser& parser, size_t layer = 0)
		{
			if (parser.finished())
			{
				throw Exception("OperatorParser::parse(): Parsing process is finished"_msg, VAEXC_POS);
			}
			
			// Brackets work just like in math
			// Here is the implementation of bracket case
			if (bracketsIncluded_ && layer == precedence_.size() && lBr_ == parser.peek())
			{
				parser.move();

				std::shared_ptr<Node> toReturn = parse(parser, 0);

				if (parser.finished() || rBr_ != parser.peek())
				{
					Token tk = parser.peek();

					throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parse(): Missing bracket before %s\n",
						tk.pos.file, tk.pos.line, tk.pos.col, tk.token));
				}
				else parser.move();

				return toReturn;
			}

			if (layer == precedence_.size()) return callAfter_(parser);

			switch (std::get<1>(precedence_.at(layer)))
			{
				case OpType::UNARY_PREFIX:
				{
					return parseUnaryPrefix(parser, layer);
				}
				case OpType::UNARY_POSTFIX:
				{
					return parseUnaryPostfix(parser, layer);
				}
				case OpType::BINARY_INFIX:
				{
					return parseBinaryInfix(parser, layer);
				}
				case OpType::BINARY_INFIX_R:
				{
					return parseBinaryInfixR(parser, layer);
				}
				case OpType::BINARY_INFIX_L:
				{
					return parseBinaryInfixL(parser, layer);
				}
				default: break;
			}

			throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parse(): Unknown operator type on layer %zu", 
				layer), VAEXC_POS);
		}

		using NodeVector = std::vector<std::shared_ptr<Node>>;

		std::shared_ptr<Node> parseUnaryPrefix(TokenizerFileParser& parser, size_t layer)
		{
			const Token& tk = parser.peek();
			for (auto op : std::get<0>(precedence_.at(layer)))
			{
				if (op != tk) continue;

				auto toReturn = std::make_shared<OperationNode>(
					NodeVector({nullptr}),
					op.withOpType(OpType::UNARY_PREFIX),
					tk.pos
				);

				parser.move();

				toReturn->args.at(0) = parse(parser, layer + 1);

				return toReturn;
			}

			return parse(parser, layer + 1);
		}

		std::shared_ptr<Node> parseUnaryPostfix(TokenizerFileParser& parser, size_t layer)
		{
			auto operand = parse(parser, layer + 1);

			if (parser.finished()) return operand;

			const Token& tk = parser.peek();
			for (auto op : std::get<0>(precedence_.at(layer)))
			{
				if (op != tk) continue;

				parser.move();

				return std::make_shared<OperationNode>(
					NodeVector({operand}),
					op.withOpType(OpType::UNARY_POSTFIX),
					tk.pos
				);
			}

			return operand;
		}

		std::shared_ptr<Node> parseBinaryInfix(TokenizerFileParser& parser, size_t layer)
		{
			auto l = parse(parser, layer + 1);

			if (parser.finished()) return l;

			const Token& tk = parser.peek();
			for (auto op : std::get<0>(precedence_.at(layer)))
			{
				if (op != tk) continue;

				parser.move();

				auto toReturn = std::make_shared<OperationNode>(
					NodeVector({l, nullptr}),
					op.withOpType(OpType::BINARY_INFIX),
					tk.pos
				);

				if (parser.finished())
				{
					throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parseBinaryInfix(): Missing second operand to %s",
						tk.pos.file, tk.pos.line, tk.pos.col, tk.token));
				}

				auto r = parse(parser, layer + 1);

				toReturn->args.at(1) = r;

				return toReturn;
			}

			return l;
		}

		// 1 + 2 + 3 + 4 = 1 + (2 + (3 + 4))
		std::shared_ptr<Node> parseBinaryInfixR(TokenizerFileParser& parser, size_t layer)
		{
			auto toReturn = parse(parser, layer + 1);
			std::shared_ptr<OperationNode> toCorrect = nullptr;

			if (parser.finished()) return toReturn;

			// Creating tree for second argument

			const Token& tk = parser.peek();
			bool found = false;
			for (auto op : std::get<0>(precedence_.at(layer)))	
			{
				if (op != tk) continue;
				found = true;

				parser.move();

				toReturn = std::make_shared<OperationNode>(
					NodeVector({toReturn, nullptr}),
					op.withOpType(OpType::BINARY_INFIX_R),
					tk.pos
				);
				toCorrect = std::dynamic_pointer_cast<OperationNode>(toReturn);

				if (parser.finished())
				{
					throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parseBinaryInfixR(): Missing second operand to %s",
						tk.pos.file, tk.pos.line, tk.pos.col, tk.token));
				}

				toCorrect->args.at(1) = parse(parser, layer + 1);

				break;
			}

			if (!found) return toReturn;

			while (!parser.finished())
			{
				const Token& tk = parser.peek();
				found = false;
				for (auto op : std::get<0>(precedence_.at(layer)))
				{
					if (op != tk) continue;
					found = true;

					parser.move();

					auto rSubTree = toCorrect->args.at(1);
					toCorrect->args.at(1) = std::make_shared<OperationNode>(
						NodeVector({rSubTree, nullptr}),
						op.withOpType(OpType::BINARY_INFIX_R),
						tk.pos
					);

					toCorrect = std::dynamic_pointer_cast<OperationNode>(toCorrect->args.at(1));

					if (parser.finished())
					{
						throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parseBinaryInfixR(): Missing second operand to %s",
							tk.pos.file, tk.pos.line, tk.pos.col, tk.token));
					}

					toCorrect->args.at(1) = parse(parser, layer + 1);

					break;
				}

				if (!found) break;
			}

			return toReturn;
		}

		// 1 + 2 + 3 = (1 + 2) + 3
		std::shared_ptr<Node> parseBinaryInfixL(TokenizerFileParser& parser, size_t layer)
		{
			auto toReturn = parse(parser, layer + 1);

			while (!parser.finished())
			{
				const Token& tk = parser.peek();
				bool found = false;
				for (auto op : std::get<0>(precedence_.at(layer)))	
				{
					if (op != tk) continue;
					found = true;

					parser.move();

					if (parser.finished())
					{
						throw Exception(ArgMsg("[%s %04zu %03hu] OperatorParser::parseBinaryInfixL(): Missing second operand to %s",
							tk.pos.file, tk.pos.line, tk.pos.col, tk.token));
					}

					toReturn = std::make_shared<OperationNode>(
						NodeVector({toReturn, parse(parser, layer + 1)}),
						op.withOpType(OpType::BINARY_INFIX_L),
						tk.pos
					);

					break;
				}

				if (!found) break;
			}

			return toReturn;
		}
	};

}

#endif  // VL_MATH_PG_OPERATOR_CONSTRUCTS_PARSER