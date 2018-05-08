// Copyright 2018 Aleinik Vladislav
// Den's password (totally in denglish) - Awsedcrfg

// DONE: DSL
// ToDo: THROW_IF_INCORRECT_TOKEN fix
#ifndef VL_MATH_PG_RECURSIVE_DESCENT_PARSERS
#define VL_MATH_PG_RECURSIVE_DESCENT_PARSERS

#include <cstdlib>

#include "../libs/VaException.hpp"
#include "OperatorConstructs.hpp"
#include "OperatorPrecedenceList.hpp"

namespace VlMathPG_RecursiveDescentParsers
{
	using namespace VlMathPG_AST;
	using namespace VlMathPG_OperatorConstructsParser;
	using namespace VlMathPG_OperatorPrecedenceList;
	using namespace TokenizeFParser;

	//-------------------------------------------------------------------------
	// Helper fuctions:
	//-------------------------------------------------------------------------

	#define THROW_IF_FINISHED() if (parser.finished()) \
		throw Exception(ArgMsg("%s(): Parsing process is finished", __func__))

	// Set  tk_type  tp false to skip the check
	// Set corr_name to false in order to skip the second check 
	#define THROW_IF_INCORRECT_TOKEN(tk_type, corr_name, msg)             \
		if (tk_type && !tk.is(tk_type) || corr_name && !tk.is(corr_name)) \
			throw Exception(ArgMsg("[%s %04zu %03hu] %s(): %s", tk.pos.file, tk.pos.line, tk.pos.col, __func__, msg))

	#define EAT_TOKEN(exp_type, exp_name, msg) \
		THROW_IF_FINISHED();                   \
		tk = parser.move();                    \
		THROW_IF_INCORRECT_TOKEN(exp_type, exp_name, msg)

	//-------------------------------------------------------------------------
	// Expression:
	//-------------------------------------------------------------------------

	std::shared_ptr<Node> parseE(TokenizerFileParser& parser);

	std::shared_ptr<Node> parseNum(TokenizerFileParser& parser)
	{
		Token tk;
		EAT_TOKEN(NUMBER, false, "Expected number");

		auto toReturn = std::make_shared<DataNode>(std::strtod(tk.token, nullptr), tk.pos);
		return toReturn;
	}

	std::shared_ptr<Node> parseId(TokenizerFileParser& parser)
	{
		Token tk;
		EAT_TOKEN(VARIABLE, false, "Expected variable or function name");

		auto toReturn = std::make_shared<VariableNode>(tk);
		return toReturn;
	}

	std::shared_ptr<Node> parseCallAndVar(TokenizerFileParser& parser)
	{
		Token tk;
		EAT_TOKEN(VARIABLE, false, "Expected function name");
		CodePos pos = tk.pos;
		std::string name{tk.token};

		THROW_IF_FINISHED();
		if (!parser.peek().is("(")) return std::make_shared<VariableNode>(tk);
		parser.move();

		// Parsing arguments:
		std::vector<std::shared_ptr<Node>> args{};
		bool firstCycle = true;
		while (!parser.finished())
		{
			tk = parser.peek();
			if (tk.is(")"))
			{
				parser.move();	
				return std::make_shared<CallNode>(name, args, pos);
			}

			if (!firstCycle)
			{
				EAT_TOKEN(COMMA, false, "Function arguments should be seperated with commies");
			}
			firstCycle = false;

			args.push_back(parseE(parser));
		}

		throw Exception(ArgMsg("[%s %04zu %03hu] parseCallAndVar(): Unclosed argument list",
		    tk.pos.file, tk.pos.line, tk.pos.col));
	}

	std::shared_ptr<Node> parseSimpleExpr(TokenizerFileParser& parser)
	{
		THROW_IF_FINISHED();

		const Token& tk = parser.peek();
		
		switch (tk.type)
		{
			case NUMBER: return parseNum(parser);
			case VARIABLE: return parseCallAndVar(parser);
			default: break;
		}

		throw Exception(ArgMsg(
			"[%s %04zu %03hu] parseVarN(): Expected number, variable or function call",
		    tk.pos.file, tk.pos.line, tk.pos.col));
	}

	using LowerParsingFunc = std::function<std::shared_ptr<Node>(TokenizerFileParser&)>;
	std::shared_ptr<Node> parseE(TokenizerFileParser& parser)
	{
		static OperatorParser exprParser = OperatorParser
		(
			OPERATOR_PRECEDENCE_LIST,
			LowerParsingFunc(parseSimpleExpr),
			true,
			Operator("("), Operator(")")
		);

		THROW_IF_FINISHED();

		return exprParser.parse(parser);
	}

	//-------------------------------------------------------------------------
	// Statements
	//-------------------------------------------------------------------------

	std::shared_ptr<Node> parseCd(TokenizerFileParser& parser);

	std::shared_ptr<Node> parseAssign(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		EAT_TOKEN(VARIABLE, false, "Expected variable name");
		CodePos pos = tk.pos;
		std::string name{tk.token};

		EAT_TOKEN(OPERATOR, "=", "Expected assign operator");

		auto val = parseE(parser);

		EAT_TOKEN(SEMICOLON, false, "Expected ; after assign statement");

		return std::make_shared<AssignNode>(name, val, pos);
	}

	std::shared_ptr<Node> parseDefVar(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		EAT_TOKEN(VARIABLE, "var", "Expected keyword: var");

		EAT_TOKEN(VARIABLE, false, "Expected variable name");
		CodePos pos = tk.pos;
		std::string name{tk.token};

		EAT_TOKEN(OPERATOR, "=", "Expected assign operator");

		auto val = parseE(parser);

		EAT_TOKEN(SEMICOLON, false, "Expected ; after assign statement");

		return std::make_shared<DefVarNode>(name, val, pos);
	}

	std::shared_ptr<Node> parseIf(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		CodePos pos = tk.pos;
		EAT_TOKEN(VARIABLE, "if", "Expected 'if' keyword");

		EAT_TOKEN(BRACKET, "(", "Missing left bracket");

		auto cond = parseE(parser);

		EAT_TOKEN(BRACKET, ")", "Missing right bracket");

		auto thenBranch = parseCd(parser);

		if (parser.finished() || !parser.peek().is(VARIABLE) || !parser.peek().is("else"))
			return std::make_shared<IfNode>(cond, thenBranch, nullptr, pos);

		parser.move();

		auto elseBranch = parseCd(parser);

		return std::make_shared<IfNode>(cond, thenBranch, elseBranch, pos);
	}

	std::shared_ptr<Node> parseWhile(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		CodePos pos = tk.pos;
		EAT_TOKEN(VARIABLE, "while", "Expected 'while' keyword");

		EAT_TOKEN(BRACKET, "(", "Missing left bracket");

		auto cond = parseE(parser);

		EAT_TOKEN(BRACKET, ")", "Missing left bracket");

		auto loopBody = parseCd(parser);

		return std::make_shared<WhileNode>(cond, loopBody, pos);
	}

	std::shared_ptr<Node> parsePrint(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		CodePos pos = tk.pos;
		EAT_TOKEN(VARIABLE, "print", "Expected 'print' keyword");

		EAT_TOKEN(BRACKET, "(", "Missing left bracket");

		auto toPrint = parseE(parser);

		EAT_TOKEN(BRACKET, ")", "Missing left bracket");

		EAT_TOKEN(SEMICOLON, false, "Expected ; after print statement");

		return std::make_shared<PrintNode>(toPrint, pos);
	}

	std::shared_ptr<Node> parseReturn(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		CodePos pos = tk.pos;
		EAT_TOKEN(VARIABLE, "return", "Expected 'return' keyword");

		auto returnVal = parseE(parser);

		EAT_TOKEN(SEMICOLON, false, "Expected ; after return statement");

		return std::make_shared<ReturnNode>(returnVal, pos);
	}

	std::shared_ptr<Node> parseDefFunc(TokenizerFileParser& parser)
	{
		Token tk = parser.peek();
		
		CodePos pos = tk.pos;
		EAT_TOKEN(VARIABLE, "def", "Expected 'def' keyword");

		EAT_TOKEN(VARIABLE, false, "Expected function name");
		std::string name{tk.token};

		EAT_TOKEN(BRACKET, "(", "Expected function name");

		// Parsing parameters:

		std::vector<std::string> params;

		bool firstCycle = true;
		while (!parser.finished())
		{
			if (parser.peek().is(")"))
			{
				parser.move();
				tk = parser.peek();
				break;
			}

			if (!firstCycle)
			{
				EAT_TOKEN(COMMA, false, "Function parameters should be seperated with commies");
			}
			firstCycle = false;

			EAT_TOKEN(VARIABLE, false, "Function parameter should be a variable");
			params.push_back(std::string(tk.token));
		}

		THROW_IF_FINISHED();

		auto body = parseCd(parser);

		return std::make_shared<DefFuncNode>(name, params, body, pos);
	}

	std::shared_ptr<Node> parseSt(TokenizerFileParser& parser)
	{
		THROW_IF_FINISHED();

		Token tk = parser.peek();
		THROW_IF_INCORRECT_TOKEN(VARIABLE, false, "All statements start with an ID or keyword");

		if (tk.is("var"))    return parseDefVar(parser);
		if (tk.is("if"))     return parseIf(parser);
		if (tk.is("while"))  return parseWhile(parser);
		if (tk.is("print"))  return parsePrint(parser);
		if (tk.is("return")) return parseReturn(parser);
		if (tk.is("def"))    return parseDefFunc(parser);

		return parseAssign(parser);;
	}

	std::shared_ptr<Node> parseCd(TokenizerFileParser& parser)
	{
		THROW_IF_FINISHED();

		Token tk = parser.peek();
		CodePos pos = tk.pos;
		if (tk.is(BRACKET) && tk.is("{"))
		{
			parser.move();

			std::vector<std::shared_ptr<Node>> statements;

			tk = parser.peek();
			for (; !parser.finished() && !tk.is(BRACKET) && !tk.is("}"); tk = parser.peek())
				statements.push_back(parseSt(parser));

			EAT_TOKEN(BRACKET, "}", "Expected } after code block");

			return std::make_shared<StSeqNode>(statements, pos);
		}

		return std::make_shared<StSeqNode>(std::vector<std::shared_ptr<Node>>({parseSt(parser)}), pos);
	}
}

namespace VMPG
{
	using namespace VlMathPG_RecursiveDescentParsers;

	std::shared_ptr<Node> parsePg(TokenizerFileParser& parser)
	{
		std::vector<std::shared_ptr<Node>> funcs;

		while (!parser.finished()) funcs.push_back(parseDefFunc(parser));

		return std::make_shared<ProgramNode>(funcs);
	}
}

#undef THROW_IF_FINISHED
#undef THROW_IF_INCORRECT_TOKEN
#undef EAT_TOKEN

#endif  // VL_MATH_PG_RECURSIVE_DESCENT_PARSERS