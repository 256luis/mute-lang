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

## Feature checklist

### Parsing

#### L/RValues
| Type                | Status      |
|---------------------|-------------|
| Literals            | Done        |
| Identifiers         | Done        |
| Binary expressions  | Not started |
| Unary expressions   | Not started |
| Routine calls       | Not started |
| Routine definitions | Not started |
| Array initializer   | Not started |
| Array index         | Not started |
| Struct initializer  | Not started |
| Field access        | Not started |
| If-expressions      | Not started |
| Match-expressions   | Not started |
| Struct definitions  | Not started |
| Enum definitions    | Not started |

#### Statements
| Type                  | Status      |
|-----------------------|-------------|
| Variable declarations | Not started |
| Variable reassignment | Not started |
| Constant declarations | Not started |
| Routine declarations  | Not started |
| Type aliases          | Not started |
| If-statements         | Not started |
| Match-statements      | Not started |
| While-loops           | Not started |
| For-loops             | Not started |
