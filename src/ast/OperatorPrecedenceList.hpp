// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_OPERATOR_PRECEDENCE_LIST
#define VL_MATH_PG_OPERATOR_PRECEDENCE_LIST

#include <vector>
#include <list>
#include <utility>

#include "AST.hpp"
#include "OperatorConstructs.hpp"

namespace VlMathPG_OperatorPrecedenceList
{
	using namespace VlMathPG_AST;
	using namespace VlMathPG_OperatorConstructsParser;

	using OperList = std::list<Operator>;
	using OperPair = std::pair<OperList, OpType>;

	std::vector<std::pair<std::list<Operator>, OpType>> OPERATOR_PRECEDENCE_LIST = 
	{
		OperPair(OperList({Operator("||")}), OpType::BINARY_INFIX_L),
		OperPair(OperList({Operator("&&")}), OpType::BINARY_INFIX_L),
		OperPair(OperList({Operator("=="), Operator("!=")}), OpType::BINARY_INFIX),
		OperPair(OperList({Operator("<"), Operator(">"), Operator("<="), Operator(">=")}), OpType::BINARY_INFIX),
		OperPair(OperList({Operator("+"), Operator("-")}), OpType::BINARY_INFIX_L),
		OperPair(OperList({Operator("*"), Operator("/")}), OpType::BINARY_INFIX_L),
		OperPair(OperList({Operator("+"), Operator("-")}), OpType::UNARY_PREFIX)
	};
}

#endif  // VL_MATH_PG_OPERATOR_PRECEDENCE_LIST