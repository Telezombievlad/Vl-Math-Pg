// Copyright 2018 Aleinik Vladislav
#ifndef VL_MATH_PG_AST
#define VL_MATH_PG_AST

#include <vector>
#include <memory>
#include <cstring>
#include <strstream>

#include "../libs/VaException.hpp"
#include "../tokenization/TokenizerFileParser.hpp"

namespace VlMathPG_AST
{
	using namespace VaExc;
	using namespace TokenizeFParser;

	class AsmTranslator;

	struct Node
	{
	public:
		Node() :
			pos_ ()
		{}

		Node(CodePos pos) :
			pos_ (pos)
		{}

		virtual ~Node() = default;

		CodePos getPos() const
		{
			return pos_;
		}

		virtual void print(std::strstream&) const = 0;

		virtual void translate(std::strstream&, AsmTranslator&) const = 0;

	private:
		CodePos pos_;
	};

	enum class OpType : char
	{
		UNARY_PREFIX,
		UNARY_POSTFIX,
		BINARY_INFIX,
		BINARY_INFIX_R,
		BINARY_INFIX_L
	};

	struct Operator
	{
	public:
		char name[MAX_TOKEN_SIZE + 6];

		explicit Operator(const char* nm) :
			name ()
		{
			if (std::strlen(nm) > MAX_TOKEN_SIZE)
			{
				throw Exception("Operator::ctor(): Operator name length should be less then MAX_TOKEN_SIZE"_msg, VAEXC_POS);
			}

			std::strcpy(name, nm);
		}

		Operator withOpType(OpType type)
		{
			Operator toReturn{""};

			switch (type)
			{
				case OpType::UNARY_PREFIX: 
				{
					std::strcpy(toReturn.name, "unpr_");
					break;
				}
				case OpType::UNARY_POSTFIX: 
				{
					std::strcpy(toReturn.name, "unpt_");
					break;
				}
				case OpType::BINARY_INFIX:
				{
					std::strcpy(toReturn.name, "binf_");
					break;
				}
				case OpType::BINARY_INFIX_R:
				{
					std::strcpy(toReturn.name, "binr_");
					break;
				}
				case OpType::BINARY_INFIX_L:
				{
					std::strcpy(toReturn.name, "binl_");
					break;
				}
				default: break;
			}

			std::strcpy(toReturn.name + 5, name);

			return toReturn;
		}

		bool operator==(const Token& tk)    const { return std::strcmp(this->name, tk.token) == 0; }
		bool operator==(const Operator& op) const { return std::strcmp(this->name, op.name ) == 0; }
		bool operator!=(const Token& tk)    const { return !(*this == tk); }
		bool operator!=(const Operator& op) const { return !(*this == op); }

		bool operator> (const Operator& op) const { return std::strcmp(this->name, op.name)  > 0; }
		bool operator>=(const Operator& op) const { return std::strcmp(this->name, op.name) >= 0; }
		bool operator< (const Operator& op) const { return std::strcmp(this->name, op.name)  < 0; }
		bool operator<=(const Operator& op) const { return std::strcmp(this->name, op.name) <= 0; }
	};

	struct OperationNode : public Node
	{
	public:
		std::vector<std::shared_ptr<Node>> args;
		Operator name;

		OperationNode(std::vector<std::shared_ptr<Node>> newArgs, Operator newName, CodePos pos) :
			Node(pos),
			args (newArgs),
			name (newName)
		{}

		virtual ~OperationNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct DataNode : public Node
	{
	public:
		double data;

		DataNode(double newData, CodePos pos) : 
			Node(pos),
			data (newData)
		{}

		virtual ~DataNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct VariableNode : public Node
	{
	public:
		std::string name;

		VariableNode(std::string newName) :
			Node(),
			name (newName)
		{}

		VariableNode(const Token& tk) :
			Node(tk.pos),
			name (std::string(tk.token))
		{}

		virtual ~VariableNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct CallNode : public Node
	{
	public:
		std::string name;
		std::vector<std::shared_ptr<Node>> args;

		CallNode(std::string funcName, std::vector<std::shared_ptr<Node>> newArgs, CodePos pos) :
			Node(pos),
			name (funcName),
			args (newArgs)
		{}

		virtual ~CallNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct AssignNode : public Node
	{
	public:
		std::string name;
		std::shared_ptr<Node> val;

		AssignNode(std::string varName, std::shared_ptr<Node> varVal, CodePos pos) :
			Node(pos),
			name (varName),
			val (varVal)
		{}

		virtual ~AssignNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct DefVarNode : public Node
	{
	public:
		std::string name;
		std::shared_ptr<Node> val;

		DefVarNode(std::string varName, std::shared_ptr<Node> varVal, CodePos pos) :
			Node(pos),
			name (varName),
			val (varVal)
		{}

		virtual ~DefVarNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct IfNode : public Node
	{
	public:
		std::shared_ptr<Node> cond;
		std::shared_ptr<Node> ifTrue;
		std::shared_ptr<Node> ifFalse;

		IfNode(std::shared_ptr<Node> condition, std::shared_ptr<Node> ifT, std::shared_ptr<Node> ifF, CodePos pos) :
			Node(pos),
			cond (condition),
			ifTrue (ifT),
			ifFalse (ifF)
		{}

		virtual ~IfNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct WhileNode : public Node
	{
	public:
		std::shared_ptr<Node> cond;
		std::shared_ptr<Node> body;

		WhileNode(std::shared_ptr<Node> condition, std::shared_ptr<Node> cycleBody, CodePos pos) :
			Node(pos),
			cond (condition),
			body (cycleBody)
		{}

		virtual ~WhileNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct PrintNode : public Node
	{
	public:
		std::shared_ptr<Node> toPrint;

		PrintNode(std::shared_ptr<Node> printed, CodePos pos) :
			Node(pos),
			toPrint (printed)
		{}

		virtual ~PrintNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct ReturnNode : public Node
	{
	public:
		std::shared_ptr<Node> toReturn;

		ReturnNode(std::shared_ptr<Node> returnVal, CodePos pos) :
			Node(pos),
			toReturn (returnVal)
		{}

		virtual ~ReturnNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct DefFuncNode : public Node
	{
	public:
		std::string name;
		std::vector<std::string> params;
		std::shared_ptr<Node> body;

		DefFuncNode(std::string fName, std::vector<std::string> fParams, std::shared_ptr<Node> fBody, CodePos pos) :
			Node(pos),
			name (fName),
			params (fParams),
			body (fBody)
		{}

		virtual ~DefFuncNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct StSeqNode : public Node
	{
	public:
		std::vector<std::shared_ptr<Node>> statements;

		StSeqNode(std::vector<std::shared_ptr<Node>> statementSeq, CodePos pos) :
			Node(pos),
			statements (statementSeq)
		{}

		virtual ~StSeqNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};

	struct ProgramNode : public Node
	{
	public:
		std::vector<std::shared_ptr<Node>> funcs;

		ProgramNode(std::vector<std::shared_ptr<Node>> funcs) :
			Node(),
			funcs (funcs)
		{}

		virtual ~ProgramNode() = default;

		virtual void print(std::strstream& stream) const;
		virtual void translate(std::strstream&, AsmTranslator&) const;
	};
}

#endif  // VL_MATH_PG_AST
