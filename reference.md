# Language Reference

## Table of contents:
- [Comments](#comments)
- [Primitive types](#primitive-types)
- [Arrays](#arrays)
- [Pointers](#pointers)
- [Type aliases](#type-aliases)
- [Structs](#structs)
- [Enums](#enums)
- [Variables](#variables)
- [Constants](#constants)
- [Routines](#routines)
- [Control flow](#control-flow)
  - [If-statements/expressions](#if-statementsexpressions)
  - [Match-statements](#match-statements)
  - [Loops](#loops)
- [Operators](#operators)

## Comments
Single-line comments begin with a `#`. There is currently no support for multiline
comments.

## Primitive types
Mute has the following primitive types
| Type    | Description                          |
|---------|--------------------------------------|
| `int`   | 64-bit signed 2's complement integer |
| `float` | 64-bit floating point                |
| `char`  | UTF-8 character                      |
| `bool`  | `true` or `false`                    |

## Arrays
Arrays are fixed-sized collections of values of the same type. They take the form
`[N]T` where `N` is a compile-time constant and `T` is a type.
```
let arr: [5]int = ...;
let arr = [5]int.[ ... ];
```

Some or all parts of `[N]int` can be omitted if they can be inferred.
```
# nothing is inferred
let arr = [5]int.[1, 2, 3, 4, 5];

# N is inferred
let arr = []int.[1, 2, 3, 4, 5];

# everything is inferred
let arr = .[1, 2, 3, 4, 5];
```

Unspecified array values are initialized to `0`. In the example below, the values of
`arr` are `[1, 2, 3, 4, 5, 0, 0, 0]`
```
let arr = [8]int.[1, 2, 3, 4, 5];
```

Accessing array members by index can be done using the `[]` operator.
```
arr[0] = 20;
```

## Strings
Mute does not have a dedicated string type. Like C, strings in Mute are achieved
with arrays of `char`s. String literals can be made by enclosing text with double
quoutes.
```
let my_string = "Hello!";
```

The type of `my_string` is `[6]char`.

## References
References can be used to alias existing variables. Any reassignment done to a
reference will be reflected in the variable being referenced.

References take the form `&T` where `T` is the type.

Use the `&` operator to make a reference to a variables
```
let num = 10;
let ref = &num;
# type of ref is &int
```

## Type aliases
Type aliases provide an alternative name to existing types. They can be defined using
the `type` keyword.
```
type MyInt = int;
```

> **NOTE:**
> Type aliases are NOT distinct from what they are aliasing i.e. `MyInt` is just
> another name for `int`, so they are treated as if they were the same type, because
> they are.

## Structs
Structs can be defined using the `struct` keyword.
```
struct {
    member1: T,
    member2: T,
    member3: T,
};
```

It is recommended to alias your struct type to give it a name.
```
type MyStruct = struct {
    member1: T,
    member2: T,
    member3: T,
};
```

Structs can be instantiated like so:
```
let mut my_struct = MyStruct.{ .member1 = ... };
```

Unspecified struct members will be initialized to 0.

Accessing struct members can be done using the `.` operator.
```
my_struct.member1 = ...;
```

## Enums
Enums in Mute are tagged unions. Each enum variant may or may not have a typed value
attached to it. Enums can be declared using the `enum` keyword.
```
enum {
    Variant1,
    Variant2(int),
    Variant3,
};
```

It is recommended to alias your enum type to give it a name.
```
type MyEnum = enum {
    Variant1,
    Variant2(int),
    Variant3,
};
```

Accessing enum variants can be done using the `.` operator.
```
let a = MyEnum.Variant2(10);
```

Testing for an enum's variant can be done using the `==` operator or with a `match`
statement. It is also through this match statement that enum variant values are
unpacked.
```
match a {
    .Variant1 -> ...,
    .Variant2(num) -> ...,
    .Variant3 -> ...,
}
```

## Variables

Variables are named containers of reusable and potentially mutable values.
```
let var = 20;
```

For any given scope, there must only be one variable with its name.
```
let x = 5;
let x = 10; # ERROR: redeclaration of `var`
```

The type of a variable can be explicit or implicit (if it can be inferred).
```
let num: int = 10; # explicit type
let num2 = 20;     # implicit type
let num3;          # ERROR: cannot infer type of `num3`
```

By default, variables are immutable. To declare a mutable variable, use the `mut`
qualifier.
```
let mut hello = 10;
```

To reassign a variable, use the `=` operator. Of course, only variables declared
`mut`able can be reassigned.
```
hello = 20;
```

Compound assignments are syntactic sugar that  combine a binary operation with
assignment. Mute supports the following compound assignment operations.
|       |      |      |       |       |
|:-----:|:----:|:----:|:-----:|:-----:|
| `+=`  | `-=` | `*=` | `/=`  | `&=`  |
| `\|=` | `~=` | `^=` | `<<=` | `>>=` |

## Constants

Constants are similar to immutable variables except they must be known at
compile-time.

To declare a constant, use the `const` keyword.
```
const PI = 3.14159265;
```

Constants and variables follow the same type inference rules.

## Routines
Routines are reusable code that perform certain operations or computations. In Mute,
there are two types of routines: `proc`edures and `func`tions.

Procedures are routines that may or may not return values, and may or may not cause
side-effects. They are no different from functions in other procedural languages

Functions are routines that **must** return a value, and **must not** cause any
side-effects. In other words, they are pure functions. Values can be returned using
the `return` keyword. Implicit returns, or the omission of the `return` keyword when
the value to be returned is at the end of the function, are allowed.

Routines can be declared using the `routine` keyword.
```
# a procedure
routine greet = proc(name: []char) {
    echo "Hello, ", name "!";
};

# a function
routine add = func(a: int, b: int) -> int {
    a + b
};
```

When a parameter of a routine is a type of array, the length can be specified to
signal that the length of the passed array **must** be the specified length, or
omitted to signal that the routine takes an array of any length.
```
# this routine takes an int array of length 10
routine f = proc(arr: [10]int) { }

# this routine takes an int array of any length
routine g = proc(arr: []int) { }
```

## Control flow

### If-statements/expressions
If-statements allow for conditional execution with an optional `else` branch.
```
if condition {
    ...
} else {
    ...
}
```

If-expressions are if-statements on the right-hand side of an assignment statement.
They are especially useful when the value of an immutable variable is conditional.
```
let immut = if condition { ... } else { ... };
```

> **NOTE:** If-expressions **must** have an `else` branch.

### Match-statements
Match-statements evaluate an expression and execute the matching branch based on the
result.
```
match expr {
    1 -> echo "one",
    2 -> echo "two",
    3 -> echo "three",
    else -> echo "more than three",
}
```

Unlike C switch-statements, match-statements in Mute do not fallthrough to subsequent
cases by default. Each branch is independent and has its own scope.

Match-statements can also be used as expressions.
```
let name = match num {
    1 -> "one",
    2 -> "two",
    else -> "other",
};
```

In both cases (statement and expression), all possible branches must be covered. This
can be done by handling each branch individually, or by having an `else` branch which
all unhandled cases default to.

### Loops
Mute supports regular while-loops and C-like for-loops.
```
while condition {
    ...
}

for let mut i = 0; i < N; i++ {
    ...
}
```

The `break` keyword can be used to break from a loop and the `continue` keyword can
be used skip the rest of the current iteration and proceed to the next one.

## Operators

### Arithmetic
| Operator | Description    | Types          |
|:--------:|----------------|----------------|
|   `+`    | Addition       | `int`, `float` |
|   `-`    | Subtraction    | `int`, `float` |
|   `*`    | Multiplication | `int`, `float` |
|   `/`    | Division       | `int`, `float` |
|   `%`    | Modulo         | `int`          |

### Comparison
| Operator | Description      | Types          |
|:--------:|------------------|----------------|
|   `==`   | Equal            | All            |
|   `!=`   | Not equal        | All            |
|   `>`    | Greater          | `int`, `float` |
|   `>=`   | Greater or equal | `int`, `float` |
|   `<`    | Less             | `int`, `float` |
|   `<=`   | Less or equal    | `int`, `float` |

### Logical
| Operator | Description | Types  |
|:--------:|-------------|--------|
|   `&&`   | Logical AND | `bool` |
|  `\|\|`  | Logical OR  | `bool` |
|   `!`    | Logical NOT | `bool` |

### Bitwise
| Operator | Description | Types |
|:--------:|-------------|-------|
|   `&`    | Bitwise AND | `int` |
|   `\|`   | Bitwise OR  | `int` |
|   `~`    | Bitwise NOT | `int` |
|   `^`    | Bitwise XOR | `int` |
|   `<<`   | Shift left  | `int` |
|   `>>`   | Shift right | `int` |

### Pointer
| Operator | Description  | Types |
|:--------:|--------------|-------|
|   `&`    | Reference to | All   |

### Array
| Operator | Description | Types |
|:--------:|-------------|-------|
|  `[N]`   | Index       | `[]T` |

Mixing of types is not allowed i.e. adding a `float` to an `int` will yield a
compile-time error. This is to ensure that all casts are intentional.
