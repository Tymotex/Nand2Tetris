# Implementation Plan [TODO: remove me]

- [ ] Write the pseudocode necessary to implement each of the functions.
- [ ] Translate to Jack.
TODO: remove Array.cc

# Jack Operating System

The purpose of the OS is to provide a software abstraction over hardware
resources.

### Math
The below are implemented as O(N) algorithms, where N is the bit width of the
input/s. 

- `multiply`
    ```
        multiply(x, y):
            sum = 0
            shiftedx = x
            for (int i = 0; i < n; ++i) {
                if (i-th bit of y == 1){  // TODO: turn this into a helper function, `bit(x, i)` that returns true if the i-th bit is 1.
                    sum = sum + shiftedx
                }
                shiftedx = 2 * shiftedx
            }
            return sum
    ```
    Need to define an array of length 16 where each element is 2^i for i = 0..15.
    This is used for 'shifting' which is not supported natively.
- `divide`
    ```
        divide(x, y):
            if (y > x) return 0
            q = divide(x, 2 * y)
            if ((x- 2 * q * y) < y) {
                return 2 * q
            } else {
                return 2 * q + 1
            }
    ```
    Works with positive x, y. If exactly one of them is negative, then flip the result's sign.
    Beware of overflow of y. Check if y is negative to detect overflow.
- `sqrt`
    ```
        sqrt(x):
            y = 0
            for (int j = (n / 2 - 1); j >=0; --j) {
                if (y + 2^j)^2 <= x then y = y + 2^j
            }
            return y;
    ```
    (y + 2^j)^2 can overflow.
- `min`
    ```
    ```

### String

All instances of String are just objects containing a field for the length of the
string, a char array and a max length property. The length will equal the max
length until characters get erased from the string.

The string can be resized if length would exceed max length.

- `intValue`
    ```python
        def int_to_string(val):
            lastDigit = val % 10
            c = character representing lastDigit
            if (val < 10)
                return c (as string)
            else 
                return int_to_string(val / 10).appendChar(c)
    ```
- `setInt`
    ```python
        def string_to_int(str):
            val = 0
            for (int i = 0...str.length()):
                d = integer value of str.charAt(i)
                val = val * 10 + d
            return val
    ```
- `newLine`
- `backSpace`
- `doubleQuote`

### Array

- `new` - this is not a constructor. It needs to directly call `Memory.alloc`.
    We do this to enable a trick required to implement `Memory.peek` and
    `Memory.poke`.
- `dispose` - calls `Memory.deAlloc`

### Memory

When the OS starts, a heap pointer is initialised to the base address of the
heap (address 2048 in RAM).

**malloc algorithm**

A linked list tracks free memory segments. Initially, this list will consist of
1 node whose size is the entire heap.
Each node begins with 2 housekeeping fields: `length` and `next`, the address of
the next free memory segment.

1. When a memory block is to be allocated, the list is searched for the best-fit or
first-fit. A free memory segment must have a size greater than or equal to the
desired size plus 2 (because each linked list node contains 2 book-keeping
fields: `size`, `next`).

2. If no segment is available, then try to defragment the heap. If that also fails,
then we're truly out of space on the heap so we have to produce an error.

3. Update the linked list node representing the memory segment we are occupying
with the remaining space in that segment. If there is no space left, then delete
the node entirely.

**free algorithm**

Append a new node to the end of the linked list tracking free memory segments.
In subsequent mallocs, that free memory segment may then be occupied.

- `peek` - we can directly access RAM addresses like this:
    ```
        var Array memory;
        let memory = 0;      // Align base address to 0.
        let x = memory[i];   // RAM[i]
        let memory[i] = 17;  // RAM[i] = 17
    ```
- `poke`
- `alloc` - start the heap at address 2048. Use the linked list data structure.
- `deAlloc` - use first-fit heuristics. 

### Screen

- `drawPixel` - use `Memory.peek` and `Memory.poke`. (row, col) is at 16384 + row * 32 + col / 16.
- `drawLine`
    ```
        drawLine(x1, y1, x2, y2):
            x = x1
            y = y1
            dx = ...
            dy = ...
            a = 0
            b = 0
            while (a <= dx and b <= dy) {
                drawPixel(x + a, y+ b0)
                if (b / a > dy/dx) a++;
                else b++;
            }
    ```
    ```
        drawLineImproved(x1, y1, x2, y2):
            x = x1
            y = y1
            dx = ...
            dy = ...
            a = 0
            b = 0
            diff = 0
            while (a <= dx and b <= dy) {
                drawPixel(x + a, y+ b0)
                if (b / a > dy/dx) { a++; diff += dy } 
                else { b++; diff -= dx }
            }
    ```
- `drawCircle` - beware `drawLine` overflow - set the max radius to be 181.
    ```
        drawCircle(x, y, r):
            for each dy = -r to r:
                drawLine(...)
    ```

The target screen has dimensions 256x512 pixels. We treat characters as
monospace blocks of dimension 11x8 pixels containing bitmapped images. This
means each line on the screen can fit 64 characters.

### Output

A global cursor is maintained for character printing. It is progressed forward
by 1 character column each time (8 pixels) and progressed forward 1 row when
a column is exhausted.

- `printChar` - write the character's bitmap onto the current 11x8 pixel box.
- `printString` - repeated calls to `printChar`
- `printInt` - convert int to string, then print.

### Keyboard

The keyboard is responsible for continuously writing the currently pressed key
into a dedicated register in RAM that the OS will peek at.

- `keyPressed` - use `Memory.peek`
- `readChar`, `readString` - see fig 12.10
- `readInt` - use `readString` and then convert to int.

### Sys

- `wait` - busy-wait in a loop that empirically is `duration` milliseconds long.
- `halt` - infinite loop.
- `init` (not documented in the OS API).
    Call the `init` functions of the other OS classes, then Call `Main.main`. 
