// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only needs to handle arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.

// Put your code here.

// Treats multiplication as repeated addition. Based on `Mult.cc`.

// int i = 0, product = 0;
    @i
    M = 0
    @R2
    M = 0
// int a = RAM[0], b = RAM[1];
    @R0
    D = M
    @a
    M = D
    @R1
    D = M
    @b
    M = D
// while (i < a) { product += b; ++i }
(LOOP)
    // if (i - a >= 0) store result in RAM[2] and jump to end
    @i
    D = M
    @a
    D = D - M

    @END
    D;JGE

    // product += b;
    @b
    D = M
    @R2
    M = M + D
    // ++i;
    @i
    M = M + 1

    @LOOP
    0;JMP

// Loop indefinitely.
(END)
    @END
    0;JMP