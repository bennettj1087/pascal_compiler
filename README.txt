Justin Bennett and Max Edmands
Compiler Project - CS445 - Spring 2009

Project goal:

	Implement a Pascal compiler in C using lex and yacc.

----------------------------------------------------------------------------------------------------------

Source listing:

       arg_node.{c,h}	A data structure for holding type information for function arguments.
       code.{c,h}	Functions used in the generation of assembly code (Intel 32-bit).
       hash.{c,h}	A linkable hash table data structure used to keep track of scopes.
       makefile		A makefile that will compile all of the code using gcc, yacc and lex.
       node.{c,h}	A linked list data structure for use in the "buckets" created in
       			the hash table.
       pc.l		Lex source file containing the tokens of the Pascal grammar used.
       pc.y		Yacc source file containing all the grammar rules for Pascal.  This
       			file also contains calls to functions for semantic checkin and 
			code generation.
       pointerlist.{c,h}     A data structure used to build lists of variables and associate
       			     type information with them.
       stack.{c,h}	A stack data structure used to keep track of the registers used by the
       			gencode algorithm.
       tree.{c,h}	A tree data structure that will contain a representation of the entire
       			program after being processed by yacc.
       types.{c,h}	Contains one function that returns a string based on the type passed
       			to it.

----------------------------------------------------------------------------------------------------------

Design:

  This compiler uses lex and yacc to scan and parse a Pascal source file according to the grammar 
found in the textbook. The source is then checked for semantic errors using function calls integrated 
into the yacc file.  If everything checks out, calls are then made to code generation functions
which write assembly language to the output file. Once complete this assembly language can (hopefully) 
be compiled using gcc.

  Rather than performing each of the compilation steps individually (scan -> parse -> semantic checks 
-> code generation), this compiler peforms many tasks as the source file is parsed. Looking through 
the yacc file, you can see that calls are made to functions for semantic checking and for code 
generation. 

Our stack looks like this:

              |  Stack starts here  |
              |---------------------|
              |                     |
              |    Dynamic space    |
              |     for locals      |
-12(%ebp)     |                     |
              |---------------------|
-8(%ebp)      |  Meta fn base ptr   |
              |---------------------|
-4(%ebp)      |  Temp for division  |
              |---------------------|
%ebp          |  Prev base pointer  |
              |---------------------|
+4(%ebp)      |  Assembly call info |
              |---------------------|
+8(%ebp)      |                     |
              |      Space for      |
              | function arguments  |
              |                     |
              |---------------------|
              |   Stack ends here   |


----------------------------------------------------------------------------------------------------------

User manual:

Use of this pascal compiler is relatively straightforward.  Below, we demonstrate using our source file, 
"source.p".  The output is written to the same filename with a .s extension ("source.s" in this case).
This is then compiled with gcc and then run.
	    mypc source.p
	    gcc source.s
	    ./a.out

----------------------------------------------------------------------------------------------------------

Testing report:

Testing is successful for every one of the semantic analysis test files provided.  

For code generation, the compiler is capable of passing all tests up to t10.p, excluding t8.p.  This is 
exactly as expected since the other test files use features that have not been implemented.

----------------------------------------------------------------------------------------------------------

Status report:

This compiler implements the following extra features:
   - Two styles of comments: {...} and (*...*)
   - Basic optimization (performing simple arithmetic operations at compile time rather than at run time).
   - Fully supports the for statement
   - Error messages that include line number and token that the code failed on

Features that are coded but don't work:
   referencing non-local names

At this time, the following features have not been implemented:
   array referencing
   register spilling (use of temporaries in assembly)
   
The following known bugs exist:
   None that we know of (yet...)
   
----------------------------------------------------------------------------------------------------------

Haiku:

we didn't know how
to write assembly language.
pipe to gpc.

----------------------------------------------------------------------------------------------------------

