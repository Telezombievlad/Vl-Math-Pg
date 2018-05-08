How to set it all working:
1) Download the project folder somewhere onto your computer. Call it Vl-Math-PG
2) Create bin folder in Vl-Math-PG
3) Compile all three .cpp files with c++17 compiler (or newer). Put the output files into the bin folder
4) Open both vl_math_pg_compile.sh and vl_math_pg_execute.sh. Set BIN_FOLDER to the path of newly-created bin folder in both files. Also check the executable files format (Change .out to .exe in TRANSLATE_SCRIPT, ASSEMBLE_SCRIPT, EXECUTE_SCRIPT definitions)

5) TO COMPILE, open terminal and call (from Vl-Math-PG folder):
./vl_math_pg_compile.sh <path/to/source/file.vmpg> <res_folder>

It will generate .valang and .vacode files in the <res_folder> folder. The first one (.valang) is the assembly language file. It is kinda readable, if you are interested. The second one (.vacode) contains command codes for emulated processor

6) TO EXECUTE, open terminal and call (from Vl-Math-PG folder):
./vl_math_pg_execute <path/to/command/file.vacode>

This one will emulate the REAL VIRTUAL PROCESSOR (holy shift?! holy shift!!! OMFG!!!) and execute the code (F yeah)

*************************************************************
* To code in Vl-Math-Pg language watch src/LangStandard.txt *
*************************************************************