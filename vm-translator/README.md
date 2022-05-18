# VM Translator (Part I)

Translates .vm files containing Jack VM instructions into a .asm file containing
Hack assembly instructions.

A VM program is just a sequence of the following command types:
- Push/pop
    - Format: `push segment index` which pulls `segment[index]` from RAM and
    inserts it into the stack. Effectively, it's `RAM[sp++] = segment[index]`.
    - Format: `pop segment index` which pops off the top of the stack and writes
    the value into `segment[index]`. Effectively, it's `segment[index] = RAM[--sp]`.
- Arithmetic/logic
    All of these commands are either binary (uses 2 operands) or unary (uses 1 operand). In either case, they simply pop the last 2 or 1 values from the stack and use those as operands.
    - Arithmetic: `add`, `sub`, `neg`
    - Logical: `and`, `or`, `not`
    - Comparison: `eq`, `gt`, `lt`
- Branching
    To be done later.
- Function
    To be done later.

Segment types include:
- local (`LCL` register in Hack assembly, which is at `RAM[1]`)
- argument (`ARG` register in Hack assembly, which is at `RAM[2]`)
- static
    Occupies `RAM[16..255]`, meaning there are 240 registers available for 
    storing static variables.
- constant
    Does not actually occupy memory.
- pointer (the segment that contains this and that, ie. occupies `RAM[3..4]`)
    - this (`THIS` register in Hack assembly, which is at `RAM[3]`)
    - that (`THAT` register in Hack assembly, which is at `RAM[4]`)
- temp (occupies 8 registers from `RAM[5..12]`)

We use `SP = RAM[0]` as the place to store the stack pointer contents. There's
no need to be aware of this though, as the VM translator.

# VM Translator (Part II)

Part 2 is about introducing branching capabilities to the Jack VM instruction
language. 

- `label <label>` - creates a label symbol with associated address.
- `goto <label>` - unconditionally jumps to the address associated with the given label.
- `if-goto <label>` - conditionally jumps. The stack's top value must be non-zero to jump, else this line effectively does nothing.
- `function <functionName> <num_params>` - declares a function and associates an address with the symbol.
    - The VM Translator generates assembly code responsible for initialising local variables of the callee.
- `call <functionName> <num_args>` - invokes function, passing in the list of args.
    - The VM Translator generates assembly code responsible for saving the current function's stack frame and jumps to the callee.
- `return` - transfers execution back to the caller.
    - The VM Translator generates assembly code responsible for copying the return value to the top of the caller's working stack and restores state that is expected to be unchanged, and jumps back to the return address.

`VMTranslator` should now take in either a filename or a directory name containing .vm files (and no subdirectories).

