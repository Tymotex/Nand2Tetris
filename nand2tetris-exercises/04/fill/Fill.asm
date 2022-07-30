// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

(FILL)
    // int i = (256 * 512 / 16);
    @8191
    D = A
    @i
    M = D  // This is how many 16-pixel groups there are in the screen.

    // For each 16-bit register, write a -1 to it.
    (FILL_LOOP)
        // if i <= 0, then jump back to the polling loop.
        @i
        D = M
        @LOOP
        D;JLE

        @SCREEN
        D = A
        @i
        D = D + M
        A = D
        M = -1

        // --i;
        @i
        M = M - 1

        @FILL_LOOP
        0;JMP

(CLEAR)
    // int i = (256 * 512 / 16);
    @8191
    D = A
    @i
    M = D  // This is how many 16-pixel groups there are in the screen.

    // For each 16-bit register, write a -1 to it.
    (CLEAR_LOOP)
        // if i <= 0, then jump back to the polling loop.
        @i
        D = M
        @LOOP
        D;JLE

        @SCREEN
        D = A
        @i
        D = D + M
        A = D
        M = 0

        // --i;
        @i
        M = M - 1

        @CLEAR_LOOP
        0;JMP

(LOOP)
    // if KBD's value is non-zero, then that means a key is being pressed
    @KBD
    D = M
    @FILL 
    D;JNE

    @CLEAR
    0;JMP
