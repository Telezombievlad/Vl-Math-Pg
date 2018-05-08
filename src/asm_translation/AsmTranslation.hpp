// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_ASM_TRANSLATION
#define VL_MATH_PG_ASM_TRANSLATION

#include <list>
#include <tuple>
#include <random>

#include "../ast/AST.hpp"
#include "AsmCommandList.hpp"

namespace VlMathPG_AST
{
	enum VarType : unsigned char
	{
		VARIABLE,
		NEW_SCOPE
	};

	class AsmTranslator
	{
	private:
		using VarData = std::tuple<VarType, std::string, unsigned short, CodePos>;
		std::list<VarData> variables_;
		unsigned short nextAdress_;
		std::string curFunc_;
		
		unsigned short nextLabel_;
		std::string randomPrefix_;

	public:
		AsmTranslator() :
			variables_    ({}),
			nextAdress_   (0),
			curFunc_      (""),
			nextLabel_    (0),
			randomPrefix_ ("__")
		{
			static const char* alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

			for (size_t n = 0; n < 14; ++n)
			{
				randomPrefix_.push_back(alphanum[std::rand() % (sizeof(alphanum) - 1)]);
			}

			randomPrefix_.push_back('_');
			randomPrefix_.push_back('_');
		}

		// Functions:
		AsmTranslator& addVar(std::string var, CodePos varPos)
		{
			for (const auto& [type, name, address, pos] : variables_)
			{
				if (type == NEW_SCOPE) break;
				if (name == var)
				{
					throw Exception(ArgMsg("[%s %04zu %03hu] Conflicting declarations: %s",
						pos.file, pos.line, pos.col, name.c_str()));
				}
			}

			variables_.push_front(VarData(VARIABLE, var, nextAdress_, varPos));

			nextAdress_++;

			return *this;
		}

		AsmTranslator& newScope(CodePos pos)
		{
			variables_.push_front(VarData(NEW_SCOPE, "NEW_SCOPE", -1, pos));

			return *this;
		}

		AsmTranslator& clearScope()
		{
			while (variables_.size() != 0)
			{
				VarType type = std::get<0>(variables_.front());
				variables_.pop_front();

				if (type == NEW_SCOPE) break;
				nextAdress_--;
			}

			return *this;
		}

		unsigned short getAddress(std::string var, CodePos varPos) const
		{
			for (const auto& [type, name, address, pos] : variables_)
			{
				if (var == name) return address;
			}

			throw Exception(ArgMsg("[%s %04zu %03hu] Variable not found: %s",
				varPos.file, varPos.line, varPos.col, var.c_str()));
		}

		AsmTranslator& enterFunc(std::string name)
		{
			curFunc_ = name;
			return *this;
		}

		AsmTranslator& leaveFunc()
		{
			curFunc_ = "";
			return *this;
		}

		const std::string& getCurFunc() const
		{
			return curFunc_;
		}

		std::string generateLabel()
		{
			return randomPrefix_ + std::to_string(nextLabel_++);
		}
	};

	using namespace VlMathPG_AST;

	void OperationNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		for (auto& arg : args) arg->translate(stream, translator);

		stream << VlMathPG_Asm_Command_List::OPERATOR_TO_ASM.at(name.name) << std::endl;
	}

	void DataNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		stream << "push " << data << std::endl;
	}

	void VariableNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		unsigned short address = translator.getAddress(name, getPos());

		stream << "pushm " << address << std::endl;
	}

	void CallNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		stream << "pushr BP" << std::endl;
		stream << "pushr SP" << std::endl;
		stream << "popr BP" << std::endl;

		for (auto arg : args) arg->translate(stream, translator);

		stream << "call " << name << std::endl;
	}

	void AssignNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		val->translate(stream, translator);

		unsigned short address = translator.getAddress(name, getPos());

		stream << "popm " << address << std::endl << std::endl;
	}

	void DefVarNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		val->translate(stream, translator);

		translator.addVar(name, getPos());

		stream << "popm " << translator.getAddress(name, getPos()) << std::endl << std::endl;
	}

	void IfNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		if (cond != nullptr) cond->translate(stream, translator);
		else stream << "push -1" << std::endl;

		stream << "push 0" << std::endl;

		std::string   ifTag = translator.generateLabel();
		std::string elseTag = translator.generateLabel();
			
		stream << "ja " <<   ifTag << std::endl;
		stream << "jmp " << elseTag << std::endl << std::endl;

		translator.newScope(getPos());
		
		stream << ifTag << ":" << std::endl;
		if (ifTrue != nullptr) ifTrue->translate(stream, translator);
		stream << std::endl;
		
		translator.clearScope();

		translator.newScope(getPos());

		stream << elseTag << ":" << std::endl;
		if (ifFalse != nullptr) ifFalse->translate(stream, translator);
		stream << std::endl;

		translator.clearScope();
	}

	void WhileNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		std::string condTag = translator.generateLabel();
		std::string endBodyTag = translator.generateLabel();

		stream << condTag << ":" << std::endl;

		if (cond != nullptr) cond->translate(stream, translator);
		else stream << "push -1" << std::endl;

		stream << "push 0" << std::endl;
		stream << "jb " << endBodyTag << std::endl << std::endl;

		translator.newScope(getPos());
		if (body != nullptr) body->translate(stream, translator);
		translator.clearScope();

		stream << "jmp " << condTag << std::endl;

		stream << endBodyTag << ":" << std::endl << std::endl;
	}

	void PrintNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		toPrint->translate(stream, translator);

		stream << "print" << std::endl;
	}

	void ReturnNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		std::string cycleTag = translator.generateLabel();
		std::string leaveCycleTag  = translator.generateLabel();

		toReturn->translate(stream, translator);
		stream << "popr RT" << std::endl;

		if (translator.getCurFunc() == "main")
		{
			stream << "end" << std::endl;
			return;
		}

		stream << cycleTag << ":" << std::endl;
		stream << "pushr SP\npushr BP\njbe " << leaveCycleTag << std::endl;
		stream << "pop" << std::endl;
		stream << "jmp " << cycleTag << std::endl;
		stream << leaveCycleTag << ":" << std::endl;
		stream << "popr BP" << std::endl;

		stream << "pushr RT" << std::endl;
		stream << "ret" << std::endl;
	}

	void DefFuncNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		translator.newScope(getPos()).enterFunc(name);

		if (name == "main") stream << "beg" << std::endl;

		stream << name << ":" << std::endl;

		for (auto param : params) translator.addVar(param, getPos());
		stream << std::endl;

		if (body != nullptr) body->translate(stream, translator);

		translator.clearScope().leaveFunc();
		stream << std::endl << std::endl;
	}

	void StSeqNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		for (auto st : statements) st->translate(stream, translator);
	}

	void ProgramNode::translate(std::strstream& stream, AsmTranslator& translator) const
	{
		for (auto f : funcs) f->translate(stream, translator);
	}
}

#endif  // VL_MATH_PG_ASM_TRANSLATION