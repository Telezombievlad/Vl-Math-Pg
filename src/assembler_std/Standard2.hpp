// Copyright 2018 Aleinik Vladislav
#ifndef HEADER_GUARD_MY_COMPILER_STANDARD1_HPP_INCLUDED
#define HEADER_GUARD_MY_COMPILER_STANDARD1_HPP_INCLUDED

// Includes:

#include <string>
#include <array>
#include <vector>

#include <memory>
#include <functional>
#include <limits>
#include <cmath>

#include <cstdio>

#include "../libs/MyException.hpp" 
#include "../libs/Stack.hpp"

// Defines:

// Passed to Exception
#define NOTHING "", "", 0

namespace MyStd1
{
	using namespace MyExceptionCharStringRepresentation;
	using namespace MyStackStaticArrayRepresentation;

	using RegAdr_t = unsigned char;
	using Val_t    = double; // Change PRINT_FORMAT
	using Cmd_t    = unsigned char;
	using CmdNum_t = unsigned short;
	using MemAdr_t = unsigned short;

	using MagicNum_t = unsigned char;
	using   StdNum_t = unsigned char;

	MagicNum_t MAGIC_NUM = 0xBA;
	StdNum_t STD_NUM = 2;

	const char*  INPUT_FORMAT = "%lf";
	const char* OUTPUT_FORMAT = "%.03lf\n";	

	const char* SINGLE_LINE_COMMENT = "//";

	//-----------------------------------------------------------------------------

	namespace _registers
	{
		const size_t REGISTER_COUNT = 7;

		const size_t RT_REGISTER_I = 4;
		const size_t BP_REGISTER_I = 5;
		const size_t SP_REGISTER_I = 6;

		const char* REGISTERS[REGISTER_COUNT] = 
		{
			"AX",
			"BX",
			"CX",
			"DX",
			"RT",
			"BP",
			"SP"
		};

	} // namespace _address

	//-----------------------------------------------------------------------------

	namespace _command
	{
		const size_t VALUE_STACK_SIZE = 1024;
		const size_t  CALL_STACK_SIZE = 1024;

		struct Command;

		struct CPU
		{
		public:
			// Variables:
				std::vector<Command*> cmdArr;
				CmdNum_t curCmd;
				Stack<CmdNum_t,  CALL_STACK_SIZE> callSt;
				Stack<   Val_t, VALUE_STACK_SIZE>  valSt; 
				std::array<Val_t, _registers::REGISTER_COUNT> regs;

			// Ctor:

				CPU() :
					cmdArr(),
					curCmd(0),
					callSt(),
					valSt (),
					regs  ()
				{
					regs.fill(0);
				}

				void updateSp()
				{
					regs[_registers::SP_REGISTER_I] = valSt.filledSize();
				}
		};
	
		struct Command
		{
		public:
			// Dtor:
				virtual ~Command() = default; // pure
			
			// Functions:
				virtual void execute(CPU& cpu) = 0; // pure
		};

		// Defines:
		#define THROW_IF_VAL_ST_FULL(cmd_name) \
			if (cpu.valSt.full()) throw Exception("Value stack is full!", "", cmd_name, 0)

		#define THROW_IF_VAL_ST_EMPTY(cmd_name) \
			if (cpu.valSt.empty()) throw Exception("Value stack is empty!", "", cmd_name, 0)

		#define THROW_IF_INVALID_REG(cmd_name) \
			if (regAdr_ >= _registers::REGISTER_COUNT) throw Exception("Unable to find the register!", "", cmd_name, 0)

		#define DEF_BINARY_OPERATOR(class_name, cmd_name, piece_of_code)           \
			struct class_name : Command                                            \
			{                                                                      \
				class_name() = default;                                            \
				virtual ~class_name() = default;                                   \
				void execute(CPU& cpu)                                             \
				{                                                                  \
					if (cpu.valSt.empty())                                         \
						throw Exception("Value stack is empty!", "", cmd_name, 0); \
					Val_t r = cpu.valSt.pop();                                     \
                                                                                   \
					if (cpu.valSt.empty())                                         \
						throw Exception("Value stack is empty!", "", cmd_name, 0); \
					Val_t l = cpu.valSt.pop();                                     \
                                                                                   \
					cpu.valSt.push(piece_of_code);                                 \
                    cpu.updateSp();                                                \
				}                                                                  \
			};

		#define DEF_COND_JUMP(class_name, cmd_name, jump_condition)                \
			struct class_name : public CmdJmp                                      \
			{                                                                      \
				explicit class_name(CmdNum_t toJump) :                             \
					CmdJmp(toJump) {};                                             \
				virtual ~class_name() = default;                                   \
				void execute(CPU& cpu)                                             \
				{                                                                  \
					if (cpu.valSt.empty())                                         \
						throw Exception("Value stack is empty!", "", cmd_name, 0); \
					Val_t r = cpu.valSt.pop();                                     \
					                                                               \
					if (cpu.valSt.empty())                                         \
						throw Exception("Value stack is empty!", "", cmd_name, 0); \
					Val_t l = cpu.valSt.pop();                                     \
                                                                                   \
					if (jump_condition) CmdJmp::execute(cpu);                      \
					cpu.updateSp();                                                \
				}                                                                  \
			};                                                                     \

		// Commands:

			// Entry and end points of the program:

				struct CmdBeg : public Command
				{
					// Functions:
						CmdBeg() = default;
						virtual ~CmdBeg() = default;
						virtual void execute(CPU& cpu) override {}
				};

				struct CmdEnd : public Command
				{
					// Functions:
						CmdEnd() = default;
						virtual ~CmdEnd() = default;
						virtual void execute(CPU& cpu) override
						{
							cpu.curCmd = cpu.cmdArr.size();

							std::printf("\nExecution finished succesefully\nProcess returned: %f\n",
								cpu.regs.at(_registers::RT_REGISTER_I));
						}
				};

			// Stack operations:

				struct CmdPush : public Command
				{
					// Variables:
						Val_t toPush_;

					// Functions:
						explicit CmdPush(Val_t toPush) : toPush_ (toPush) {}
						virtual ~CmdPush() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_FULL("PUSH");

							cpu.valSt.push(toPush_);

							cpu.updateSp();
						}
				};

				struct CmdPushR : public Command
				{
					// Variables:
						RegAdr_t regAdr_;

					// Functions:
						explicit CmdPushR(RegAdr_t regAdr) : regAdr_ (regAdr) {};
						virtual ~CmdPushR() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_FULL("PUSHR");

							THROW_IF_INVALID_REG("PUSHR");

							cpu.valSt.push(cpu.regs.at(regAdr_));

							cpu.updateSp();
						}
				};

				struct CmdPop : public Command
				{
					// Functions:
						CmdPop() = default;
						virtual ~CmdPop() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("POP");

							cpu.valSt.pop();

							cpu.updateSp();
						}
				};

				struct CmdPopR : public Command
				{
					// Variables:
						RegAdr_t regAdr_;

					// Functions:
						explicit CmdPopR(RegAdr_t regAdr) : regAdr_ (regAdr) {};
						virtual ~CmdPopR() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("POPR");

							THROW_IF_INVALID_REG("POPR");

							cpu.regs.at(regAdr_) = cpu.valSt.pop();

							cpu.updateSp();
						}
				};

			// Arithmetics:

				DEF_BINARY_OPERATOR(CmdAdd, "ADD", l + r);
				DEF_BINARY_OPERATOR(CmdSub, "SUB", l - r);
				DEF_BINARY_OPERATOR(CmdMul, "MUL", l * r);

				struct CmdDiv : public Command
				{
					// Functions:
						CmdDiv() = default;
						virtual ~CmdDiv() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("DIV");

							Val_t divisor = cpu.valSt.pop();

							if (std::abs(divisor) <= std::numeric_limits<Val_t>::epsilon() * 5)
							{
								throw Exception("Unable to divide by 0!", "", "DIV", 0);
							}

							THROW_IF_VAL_ST_EMPTY("DIV");

							cpu.valSt.push(cpu.valSt.pop() / divisor);

							cpu.updateSp();
						}
				};

				struct CmdSqrt : public Command
				{
					// Functions:
						CmdSqrt() = default;
						virtual ~CmdSqrt() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("SQRT");

							Val_t popped = cpu.valSt.pop();

							if (popped < 0)
							{
								throw Exception("Root of negative number doesn't exist", "", "SQRT", 0);
							}

							cpu.valSt.push(std::sqrt(popped));

							cpu.updateSp();
						}
				};

			// IO:

				struct CmdOut : public Command
				{
					// Functions:
						CmdOut() = default;
						virtual ~CmdOut() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("OUT");

							std::printf(OUTPUT_FORMAT, cpu.valSt.pop());

							cpu.updateSp();
						}
				};

				struct CmdIn : public Command
				{
					// Functions:
						CmdIn() = default;
						virtual ~CmdIn() = default;
						virtual void execute(CPU& cpu) override
						{
							Val_t toRead = 0;

							std::scanf(INPUT_FORMAT, &toRead);

							THROW_IF_VAL_ST_FULL("IN");

							cpu.valSt.push(toRead);

							cpu.updateSp();
						}
				};

			// Jumps:

				struct CmdJmp : public Command
				{
					// Variables:
						CmdNum_t toJump_;
					// Functions:
						explicit CmdJmp(CmdNum_t toJump) : toJump_ (toJump) {};
						virtual ~CmdJmp() = default;
						virtual void execute(CPU& cpu) override
						{
							cpu.curCmd = toJump_ - 1; // That -1 is because ++cpu.curCmd is called when the cycle step ends
						}
				};

				DEF_COND_JUMP(CmdJe,  "JE",  std::abs(l-r) <= 5 * std::abs(l+r) * std::numeric_limits<Val_t>::epsilon())
				DEF_COND_JUMP(CmdJne, "JNE", std::abs(l-r) >  5 * std::abs(l+r) * std::numeric_limits<Val_t>::epsilon())

				DEF_COND_JUMP(CmdJa,  "JA",  l >  r)
				DEF_COND_JUMP(CmdJae, "JAE", l >= r)
				DEF_COND_JUMP(CmdJb,  "JB",  l <  r)
				
				struct CmdJbe : public CmdJmp                                      
				{                                                                      
					explicit CmdJbe(CmdNum_t toJump) :                             
						CmdJmp(toJump) {};                                             
					virtual ~CmdJbe() = default;                                   
					void execute(CPU& cpu)                                             
					{   
						THROW_IF_VAL_ST_EMPTY("JBE");
						Val_t r = cpu.valSt.pop();                                     
						                                                               
						THROW_IF_VAL_ST_EMPTY("JBE");                                                              
						Val_t l = cpu.valSt.pop();                                     

						if (l <= r) CmdJmp::execute(cpu);                      
						
						cpu.updateSp();                                                
					}                                                                  
				}; 

			// Function support:

				struct CmdCall : public CmdJmp
				{
					// Functions:
						explicit CmdCall(CmdNum_t toJump) : CmdJmp(toJump) {};
						virtual ~CmdCall() = default;
						virtual void execute(CPU& cpu) override
						{
							if (cpu.callSt.full())
							{
								throw Exception("CALL: Stack overflow", NOTHING);
							}

							cpu.callSt.push(cpu.curCmd);
							CmdJmp::execute(cpu);

							cpu.updateSp();
						}
				};

				struct CmdRet : public Command
				{
					// Functions:
						CmdRet() = default;
						virtual ~CmdRet() = default;
						virtual void execute(CPU& cpu) override
						{
							if (cpu.callSt.empty())
							{
								throw Exception("RET: Call stack is empty", NOTHING);
							}

							cpu.curCmd = cpu.callSt.pop();

							cpu.updateSp();
						}
				};

			// Dump:

				struct CmdDump : public Command
				{
					// Functions:
						CmdDump() = default;
						virtual ~CmdDump() = default;
						virtual void execute(CPU& cpu) override
						{
							std::printf("--------------STACK----------------\n");
							for (size_t i = cpu.valSt.filledSize() - 1; i < cpu.valSt.filledSize(); --i)
							{
								std::printf(OUTPUT_FORMAT, cpu.valSt.at(i));
							}
							std::printf("------------CALL-STACK-------------\n");
							for (size_t i = cpu.callSt.filledSize() - 1; i < cpu.callSt.filledSize(); --i)
							{
								std::printf(OUTPUT_FORMAT, cpu.callSt.at(i));
							}
							std::printf("------------REGISTERS-------------\n");
							for (size_t i = 0; i < cpu.regs.size(); ++i)
							{
								std::printf("%s: %lf\n", _registers::REGISTERS[i], cpu.regs.at(i));
							}
							std::printf("----------------------------------\n");
						}
				};

				struct CmdPrint : public Command
				{
					// Funcyions:
						CmdPrint() = default;
						virtual ~CmdPrint() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("PRINT");

							std::printf(OUTPUT_FORMAT, cpu.valSt.pop());

							cpu.updateSp();
						}
				};

			// Comparison:

				DEF_BINARY_OPERATOR(CmdIsL,  "IS_L",  (l <  r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdIsLE, "IS_LE", (l <= r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdIsM,  "IS_M",  (l >  r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdIsME, "IS_ME", (l >= r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdIsE,  "IS_E",  (l == r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdIsNE, "IS_NE", (l != r)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdAnd,  "AND",   (l > 0 && r > 0)? 1 : -1)
				DEF_BINARY_OPERATOR(CmdOr,   "OR",    (l > 0 || r > 0)? 1 : -1)

			// Memory operations:

				struct CmdPushMem : public Command
				{
					// Variables:
						MemAdr_t memAdr_;
					// Functions:
						CmdPushMem(MemAdr_t memAdr) : 
							memAdr_(memAdr)
						{}

						virtual ~CmdPushMem() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_FULL("PUSHM");

							MemAdr_t bp = cpu.regs.at(_registers::BP_REGISTER_I);

							if (bp + memAdr_ >= cpu.valSt.filledSize())
							{
								throw Exception("Access out of stack", "", "PUSHM", 0);
							}

							cpu.valSt.push(cpu.valSt.at(bp + memAdr_));

							cpu.updateSp();
						}
				};

				struct CmdPopMem : public Command
				{
					// Variables:
						MemAdr_t memAdr_;
					// Functions:
						CmdPopMem(MemAdr_t memAdr) : 
							memAdr_(memAdr)
						{}
						virtual ~CmdPopMem() = default;
						virtual void execute(CPU& cpu) override
						{
							THROW_IF_VAL_ST_EMPTY("POPM");

							MemAdr_t bp = cpu.regs.at(_registers::BP_REGISTER_I);

							if (bp + memAdr_ >= cpu.valSt.filledSize())
								throw Exception("Access out of stack", "", "POPM", 0);

							if (bp + memAdr_ < cpu.valSt.filledSize() - 1)
							{
								cpu.valSt.at(bp + memAdr_) = cpu.valSt.pop();
							}

							cpu.updateSp();
						}
				};

		//-----------------------------------------------------------------------------

		// Now info for assembler and disassembler:
		
		enum class ArgType
		{
			REGISTER_ADDRESS,
			VALUE,
			NAMETAG,
			ADDRESS_SHIFT,
			MEMORY_ADDRESS
		};

		using Word = FileWork::Word;

		struct CommandType
		{
		public:
			// Variables:
				FileWork::Word name;
				const std::vector<ArgType> argTypes;			
		};

		const CommandType COMMANDS[] =
		{
			{Word(   "BEG"), {}}, //0
			{Word(   "END"), {}}, //1
			{Word(  "PUSH"), {ArgType::VALUE}}, //2
			{Word( "PUSHR"), {ArgType::REGISTER_ADDRESS}}, //3
			{Word(   "POP"), {}}, //4
			{Word(  "POPR"), {ArgType::REGISTER_ADDRESS}}, //5
			{Word(   "ADD"), {}}, //6
			{Word(   "SUB"), {}}, //7
			{Word(   "MUL"), {}}, //8
			{Word(   "DIV"), {}}, //9 
			{Word(  "SQRT"), {}}, //10
			{Word(   "OUT"), {}}, //11
			{Word(    "IN"), {}}, //12
			{Word(   "JMP"), {ArgType::NAMETAG}}, //13
			{Word(    "JE"), {ArgType::NAMETAG}}, //14
			{Word(   "JNE"), {ArgType::NAMETAG}}, //15
			{Word(    "JA"), {ArgType::NAMETAG}}, //16
			{Word(   "JAE"), {ArgType::NAMETAG}}, //17
			{Word(    "JB"), {ArgType::NAMETAG}}, //18
			{Word(   "JBE"), {ArgType::NAMETAG}}, //19
			{Word(  "CALL"), {ArgType::NAMETAG}}, //20
			{Word(   "RET"), {}}, //21
			{Word(     "@"), {}}, //22
			{Word( "PRINT"), {}}, //23
			{Word(  "IS_L"), {}}, //24
			{Word( "IS_LE"), {}}, //25
			{Word(  "IS_M"), {}}, //26
			{Word( "IS_ME"), {}}, //27
			{Word(  "IS_E"), {}}, //28
			{Word( "IS_NE"), {}}, //29
			{Word(   "AND"), {}}, //30
			{Word(    "OR"), {}}, //31
			{Word("PUSHM"), {ArgType::MEMORY_ADDRESS}}, // 32
			{Word( "POPM"), {ArgType::MEMORY_ADDRESS}}  // 33
		};

		const Cmd_t COMMAND_COUNT = sizeof(COMMANDS) / sizeof(*COMMANDS);

	} // namespace _command

} // namespace MyCompilerStandard0
#undef NOTHING

#endif /*HEADER_GUARD_MY_COMPILER_STANDARD0_HPP_INCLUDED*/