# Project Documentation

## Table of contents

## Program flow
Compilers are generally split up into multiple stages. The typical one being:
```
    Input                                                                                          Output
+-------------+   Tokenization   +--------+   Parsing   +-----+   Codegen   +-----+   Assembly   +--------+
| Source code |----------------->| Tokens |------------>| AST |------------>| ASM |------------->| Binary |
+-------------+                  +--------+             +-----+             +-----+              +--------+
```

> **NOTE:** For the sake of brevity, some stages (notably IR, and optimization stages)
> may not have been included.

Since Mute transpiles to C, we first need to transform our Mute AST into C AST before
generating C code. The generated C code is then run through a C compiler to produce
the final binary.
```
    Input                                                                                                              Output
+-------------+   Tokenization   +--------+   Parsing   +----------+   Transform to C AST   +-------+   C Compiler   +--------+
| Source code |----------------->| Tokens |------------>| Mute AST |----------------------->| C AST |--------------->| Binary |
+-------------+                  +--------+             +----------+                        +-------+                +--------+
```

Compiling to C allows us to leverage optimizations present in C compilers.

## Memory management strategy
Memory will not be freed. This is a short-lived program. Let the OS free the memory
once the program ends.

## Feature checklist
> Legend: ✅ Done - ⏳ In progress - ⬜ Not started

### Parsing

#### L/RValues
| Type                | Status | Remarks                                        |
|---------------------|:------:|------------------------------------------------|
| Literals            |   ✅   |                                                |
| Identifiers         |   ✅   |                                                |
| Binary expressions  |   ⏳   | Operator precedence is currently unimplemented |
| Unary expressions   |   ✅   |                                                |
| Routine calls       |   ✅   |                                                |
| Routine definitions |   ⬜   |                                                |
| Array initializer   |   ✅   |                                                |
| Array types         |   ✅   |                                                |
| Array index         |   ✅   |                                                |
| Struct initializer  |   ✅   |                                                |
| If-expressions      |   ✅   |                                                |
| Match-expressions   |   ⬜   |                                                |
| Struct definitions  |   ⬜   |                                                |
| Enum definitions    |   ⬜   |                                                |
| Field access        |   ✅   |                                                |
| Compound            |   ✅   |                                                |

#### Statements
| Type                  | Status | Remarks |
|-----------------------|:------:|---------|
| Variable declarations |   ⬜   |         |
| Variable reassignment |   ✅   |         |
| Constant declarations |   ⬜   |         |
| Routine declarations  |   ⬜   |         |
| Type aliases          |   ⬜   |         |
| If-statements         |   ✅   |         |
| Match-statements      |   ⬜   |         |
| While-loops           |   ⬜   |         |
| For-loops             |   ⬜   |         |
| Compound              |   ✅   |         |
