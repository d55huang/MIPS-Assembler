# MIPS Assembler

This is a C++ implementation of an assembler for the MIPS assembly language. The program translates all correct assembly language programs from standard input into MIPS machine language on standard output.

## MIPS Assembly Language

A MIPS Assembly program is a Unix text file consisting of a number of lines. Each line has the general format
```
labels instruction comment
```
Every line with an instruction specifies a corresponding machine language instruction word. Lines without an instruction are called null lines and do not specify an instruction word. That is, an assembly language program with n non-null lines specifies a machine language program with n words, in 1-1 ordered correspondence.

### Labels

The labels component lists zero or more labels, each followed by a colon (:). A label is a string of alphanumeric characters, the first of which must be a letter of the alphabet. For example, fred123x is a valid label but 123fred is not.
A label appearing in the labels component is said to be defined; a particular label may be defined at most once in an assembly language program. Labels are case-sensitive; that is, fred and Fred are distinct labels.

The location of a line in an assembly language program is 4n, where n is the number of non-null lines preceding it. The first line therefore has location 0. If the first line is non-null, the second line has location 4. On the other hand, if the first line is null, the location of the second line is also 0. Note that the location of any non-null line is exactly the same as the location of the machine language word that it specifies. And all null lines immediately preceding it have the same location.

The value of a label is defined to be the location of the line on which it is defined.

### Comments

A comment is any sequence of characters beginning with a semicolon (;) and ending with the end-of-line character that terminates the line. Comments have meaning only to the reader; they do not contribute to the specification of the equivalent machine language program.

### Instructions

An instruction takes the form of an opcode followed by one or more operands. The opcode may be add, sub, mult, multu, div, divu, mfhi, mflo, lis, lw, sw, slt, sltu, beq, bne, jr, jalr, or the pseudo-opcode .word. The number, allowed formats, and meaning of operands depend on the opcode. Operands are, unless otherwise specified, separated by commas (,). An operand may be
* a register denoted $0, $1, $2, ... $31,
* an unsigned or decimal integer denoted by a string of digits 0-9,
* a negative decimal integer denoted by a minus sign (-) followed by an unsigned decimal integer,
* a hexadecimal number denoted by 0x followed by a string of hexadecimal digits 0-9 or a-f or A-F,
* a label.

### Operand Format - add, sub, slt, sltu

These opcodes all take three register operands; for example
```
add $1, $2, $3
```

### Operand Format - mult, multu, div, divu

These opcodes take two register operands corresponding to $s and $t. For example
```
mult $4, $5
```

### Operand Format - mfhi, mflo, lis

These opcodes have a single register operand, $d.

### Operand Format - lw, sw

These opcodes have two register operands, $s and $t, and in addition an immediate operand, i. For example,
```
lw $4, 400($7)
```

### Operand Format - beq, bne

These operations take three operands: registers $s and $t, and an immediate operand i. i may be specified as an unsigned decimal integer, a negative decimal integer, a hexadecimal number, or a label. If i is a decimal or hexadecimal number, i is encoded as a 16-bit two's complement number; i must therefore be in the range -32768 through 32767 if i is decimal, and must not exceed 0xffff if i is hexadecimal.
If i is a label, the value (i-L-4)/4 is encoded where i is the value of the label and L is the location of the beq or bne instruction. (i-L-4)/4 must be in the range -32768 through 32767.

### Opearand Format - jr, jalr

These operations have one register operand, $s.

### Operand Format .word

.word is not a true opcode as it does not necessarily encode a MIPS instruction at all. .word has one operand i which is a 32-bit signed or unsigned number. If i is a decimal number, it must be in the range -2^31 through 2^32-1 (that is, the union of the ranges for signed and unsigned 32-bit integers). If i is hexadecimal, it must not exceed 0xffffffff. If a label is used for i, its value is encoded as a 32-bit integer.
