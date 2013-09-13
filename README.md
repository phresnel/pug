# pug-vm

Pug is a tiny, small, minimalistic, evening studyable,tweakable, C++11 self-contained, embeddable and header-only Stack Virtual-Machine.

Use it for learning, fun, or as a backend to your own tiny programming language.

## Overview

The machine is a [stack-machine](https://en.wikipedia.org/wiki/Stack_machine), meaning that most instructions work on values in the stack.

# Examples

The following is an example of using PugVM directly using the raw C++ API. An assembler which makes for better redable code is being planned:

    // Recursive faculty program.
    int res = 5;
    std::vector<Instruction> program {
        {LoadInt, &res}
        {Call, 5},
        {PopReduce},
        {StoreInt, &res},
        {Exit},

        // fac(x)
            {LoadArg, 0},
            {PushInt, 1},
            {NotEqualsII},
            {JumpRelIfTrue, 3},
                {PushInt, 1},
                {ReturnTos},

            {LoadArg,0},
            {DecrementI},
            {Call, 5},
            {PopReduce},
            {LoadArg,0},
            {MulII},
            {ReturnTos}
    };

    PugVM::StackMachine<true> m(program);
    while (!m.halted()) {
        m.tick();
    }
    std::cout << "result: " << res << std::endl;

This code would output 120 as the factulty for 5. 

Loops are also possible. The following is the iterative faculty variation:

    // Iterative faculty program.
    std::vector<Instruction> program{
        {LoadInt, &res},  // [local 0]
        {Dup},            // [local 1] duplicate input to be used as counter

        // start of while loop
        {Dup},              // \
        {PushInt,1},        // | compare counter to 1
        {EqualsII},         // /      \
        {JumpRelIfTrue, 7}, //      quit if counter is 1

        {DecrementI},

        {LoadLocal, 0},
        {LoadLocal, 1},
        {MulII},
        {StoreLocal, 0},
        {Jump, 2},
        // loop end

        {Pop},
        {StoreInt, &res},
        {Exit}
    };


### Instructions

 * PushFloat
 * PushInt
 * PushBool

 * Pop
 * PopReduce

 * Dup

 * StoreFloat
 * StoreInt
 * StoreBool
 * StoreStAbs
 * StoreStRel
 * StoreLocal
 * StoreArg

 * LoadFloat
 * LoadInt
 * LoadBool
 * LoadStAbs
 * LoadStRel
 * LoadLocal
 * LoadArg

 * EqualsFF
 * EqualsII
 * EqualsBB
 * NotEqualsFF
 * NotEqualsII
 * NotEqualsBB

 * Jump
 * JumpIfTrue
 * JumpRel
 * JumpRelIfTrue

 * Call
 * Return
 * ReturnTos

 * AddFF
 * AddII

 * SubFF
 * SubII

 * MulFF
 * MulII

 * DecrementI
 * IncrementI

 * Dump

 * Exit

## Debugging

It offers some runtime introspection:

    [  0]         LoadInt : 
    [  1]            Call : 5 | 
    [  5]         LoadArg : 5 | 2 | 0 | 
    [  6]         PushInt : 5 | 2 | 0 | 5 | 
    [  7]     NotEqualsII : 5 | 2 | 0 | 5 | 1 | 
    [  8]   JumpRelIfTrue : 5 | 2 | 0 | 1 | 
    [ 11]         LoadArg : 5 | 2 | 0 | 

Here, the first number is the address of the instruction, the second is the operation mnemonic, and the information on the right hand side of ':' is the stack before the operation happens. You can read it e.g. like this: `Execute NotEquals upon the stack [5, 2, 0, 5, 1]`.

