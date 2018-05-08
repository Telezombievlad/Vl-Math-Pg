// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_AST_PRINT
#define VL_MATH_PG_AST_PRINT

#include "AST.hpp"

namespace VlMathPG_AST
{
	void OperationNode::print(std::strstream& stream) const
	{
		static char toPrint[MAX_TOKEN_SIZE + 1];

		if (args.size() > 1) stream << "(";

		std::strcpy(toPrint, name.name + 5);

		bool firstCycle = true;
		for (auto node : args)
		{
			if (!firstCycle) stream << " " << toPrint << " ";
			firstCycle = false; 
			node->print(stream); 
		}

		if (args.size() > 1) stream << ")";
	}

	void DataNode::print(std::strstream& stream) const
	{
		stream << data;
	}

	void VariableNode::print(std::strstream& stream) const
	{
		stream << name;
	}

	void CallNode::print(std::strstream& stream) const
	{
		stream << name << "(";

		bool firstCycle = true;
		for (auto node : args)
		{
			if (!firstCycle) stream << ", ";
			firstCycle = false; 
			node->print(stream); 
		}

		stream << ")";
	}

	void AssignNode::print(std::strstream& stream) const
	{
		stream << name << " = ";
		val->print(stream);
		stream << ";\n";
	}

	void DefVarNode::print(std::strstream& stream) const
	{
		stream << "var " << name << " = ";
		val->print(stream);
		stream << ";\n";
	}

	void IfNode::print(std::strstream& stream) const
	{
		stream << "if (";
		cond->print(stream);
		stream << ") ";
		ifTrue->print(stream);
		if (ifFalse != nullptr)
		{
			stream << "else ";
			ifFalse->print(stream);
		}
	}

	void WhileNode::print(std::strstream& stream) const
	{
		stream << "while (";
		cond->print(stream);
		stream << ") ";
		body->print(stream);
	}

	void PrintNode::print(std::strstream& stream) const
	{
		stream << "print(";
		toPrint->print(stream);
		stream << ");\n";
	}

	void ReturnNode::print(std::strstream& stream) const
	{
		stream << "return ";
		toReturn->print(stream);
		stream << ";\n";
	}

	void DefFuncNode::print(std::strstream& stream) const
	{
		stream << "def " << name << "(";

		bool firstCycle = true;
		for (auto param : params)
		{
			if (!firstCycle) stream << ", ";
			firstCycle = false; 
			stream << param; 
		}

		stream << ") ";
		body->print(stream);
	}

	void StSeqNode::print(std::strstream& stream) const
	{
		if (statements.size() == 0) stream << ";";
		else if (statements.size() == 1) statements.at(0)->print(stream);
		else
		{
			stream << "\n{\n";
			for (auto st : statements) st->print(stream);
			stream << "}\n";
		}
	}

	void ProgramNode::print(std::strstream& stream) const
	{
		for (auto func : funcs)
		{
			func->print(stream);
			stream << "\n";
		}
	}
}

#endif  // VL_MATH_PG_AST_PRINT