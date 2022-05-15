# VM Translator

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
- local (`LCL` register in Hack assembly, which points to `RAM[1]`)
- argument (`ARG` register in Hack assembly, which points to `RAM[2]`)
- static
    Occupies `RAM[16..255]`, meaning there are 240 registers available for 
    storing static variables.
- constant
    Does not actually occupy memory.
- pointer (the segment that contains this and that, ie. occupies `RAM[3..4]`)
    - this (`THIS` register in Hack assembly, which points to `RAM[3]`)
    - that (`THAT` register in Hack assembly, which points to `RAM[4]`)
- temp (occupies 8 registers from `RAM[5..12]`)

We use `SP = RAM[0]` as the place to store the stack pointer contents. There's
no need to be aware of this though, as the VM translator.
