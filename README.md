# MIMA Assembler Tools

## Build and Run

This project is build with CMake. After letting CMake build all targets,
you will find your executables inside the `/bin` folder

## Project Structure

This project consists of three parts:
- miasm (**MI**MA **as**se**m**bler)
- midas (**MI**MA **d**is**as**sembler)
- runmima (MIMA bytecode interpreter*)

### MIMA Assembler (miasm)

Usage:
```
miasm <source.miasm> [output.mic]
```

The second argument is optional. If not specified, the file will be saved to 
the current directory. See [example.miasm](example.miasm) for a syntax explanation

### MIMA Disassembler (midas)

Usage:
```
midas <bytecode.mic> [begin] [end]
```
The disassembler requires a mima bytecode file as it's first argument.
The second and third argument are the bounds, in between which the memory
is dumped to stdout. By default, begin is set to 0 and end is set to the last
address explicitly set in the bytecode file. You can use numbers
(decimal, hex or octal literals in C-syntax) or the initial `IAR` value and `EOF`.

### MIMA Bytecode Interpreter* (runmima)

This program just takes the bytecode file as it's first argument.
Before execution, all input fields are prompted and stored in the memory.
After execution, all output fields are printed.

### MIMA Bytecode

| Bits | Description |
| --- | --- |
| 0-4 | contains the string "MIMA\0" |
| 5-7 | content of the IAR |
| 8 | number of input addresses |
| 9 | number of output addresses |
| 10+ | input addresses (each 3 bytes) |
| ... | output addresses (each 3 bytes) |
| until EOF | memory before execution |

### Example MIASM Program

```asm
; mima assembly language example

.data           ; anything written before .data is ignored

mem a1, a2, a3  ; a1, a2 and a3 are memory addresses in which the value 0 is stored
ask a1          ; mark a1 as an input field
out a3          ; mark a3 as an output field

.text           ; end of data segment, start of text segment

LDV a1          ; instruction using a memory address (is replaced by the preprocessor)
NOT             ; instruction without arguments
STV a2
LDC 0x1         ; instruction using a literal which will be appended to the opcode
ADD a2
STV a3

JMP end         ; jump to section ":end"
LDC 0x0         ; (skipped)
STV a3          ; (skipped)

HALT     :end   ; the assembler will not check for unreachable code, you will have to do that yourself
```

*: Based on the works of [Thorsten Rapp](mailto:Tutor@web.de) 
