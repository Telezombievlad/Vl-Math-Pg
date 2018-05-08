// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_ASM_COMMAND_LIST
#define VL_MATH_PG_ASM_COMMAND_LIST

#include <map>
#include <utility>

namespace VlMathPG_Asm_Command_List
{
	using OperImplPair = std::pair<std::string, std::string>;

	std::map<std::string, std::string> OPERATOR_TO_ASM
	{
		OperImplPair("unpr_+", ""),
		OperImplPair("unpr_-", "push -1\nmul"),
		OperImplPair("binl_*", "mul"),
		OperImplPair("binl_/", "div"),
		OperImplPair("binl_+", "add"),
		OperImplPair("binl_-", "sub"),
		OperImplPair("binf_<",  "is_l"),
		OperImplPair("binf_<=", "is_le"),
		OperImplPair("binf_>",  "is_m"),
		OperImplPair("binf_>=", "is_me"),
		OperImplPair("binf_==", "is_e"),
		OperImplPair("binf_!=", "is_ne"),
		OperImplPair("binl_&&", "and"),
		OperImplPair("binl_||", "or")
	};
}

#endif  // VL_MATH_PG_ASM_COMMAND_LIST
