# HomeworkScript

Schoold project for *AUtomata and Grammars*.

The main idea was to create an interpreter for a simple programming language. To learn more, I decided to attempt establishing my own grammar instead of copying already existing one.

The source code requires a refactor. The grammar also could be improved: The requirement to pass variable names instead of expressions to functions is cumbersome.

Nevertheless the project is archived and will not be updated. I can't wait to create a new language!

## Language

### Variables

The language supports two types of variables: `Number` and `Logic`, expressing integer numbers and boolean values.

```
let a = 1;
let b = true;
```

> Unfortunatelly, there wasn't enough time to implement `let a : Number = 1;` statement.

Values can be reassigned. Every variable has a type assigned at its initialization. So an attempt to change type will crash the program.

```
a = 2;
b = false;
a = true; /* ILLEGAL! */
```

The language organizes statements with semicolons, so they need to follow both types of statements. It also applies to the printing debugging statement:

```
let a = 2;
print a;
```

Which will print name, type and value of the variable.


### Operators

The language supports basic arithmetic and logic operators to be used with `Number` and `Logic` values respectively.

```
let a = 5; let b = 3;
let n1 = a + b; /* 8 */
let n2 = a - b; /* 2 */
let n3 = a * b; /* 15 */
let n4 = a / b; /* 1 */
let n5 = a % b; /* 2 */
let n6 = -a; /* -5 */

let c = true; let d = false;
let l1 = c && d;
let l2 = c || d;
let l3 = c ^ d;
let l4 = !c;
```

Also comparison operators can be used.

```
let a = 5; let b = 3;
let l1 = a == b; /* false */
let l2 = a != b; /* true */
let l3 = a > b; /* true */
let l4 = a >= b; /* true */
/* ... */
```

With `==` and `!=` applicable for both `Number` and `Logic` types.

### Flow Control

The language supports two flow control keywords: `if` and `while`. Of course, they require `true` to be executed. The biggest difference is that they require semicolon.

```
let a = true;
if a {
  /**/
};
while a {
  /**/
}; 
```

Passing `Number` value will crash the program.

```
let a = 2;
if a { /**/ }; /* ILLEGAL! (a does not evaluate to bool) */
```

### Functions

Declaring a function takes place with a `func` keyword followed by argument list. Arguments have no type checking.

```
let dummy = 0;
func doSomething(a) { /**/ }
```

Variables are **passed by name** but **can be overloaded**. This means that when a variable is passed to a method, inside it uses a copy of original variable.

```
let a = 1;
let b = 1;

func changeValues(b) {
  a = 2;
  b = 2;
  let c = 2;
}

changeValues(b);

print a; // Prints 2
print b; // Prints 1
print c; // ILLEGAL! (c is not present after exiting the function)
```

Stopping the method and returning a value takes place with `return` keyword. Unfortunatelly, it **always** requires a **variable name** to be returned.

```
func addOne(a) {
  return a + 1; // ILLEGAL! (Must bind to a variable)
}

func addOne(a) {
  let result = a + 1;
  return result; // Works!
}
```

Functions can be declared in functions. As mentioned, variables are passed by name so they are autmatically captured, unless overloaded.

```
let a = 0;
func doStuff() {
  for doMoreStuff() {
    a = 1;
  }
  doMoreStuff();
}
doStuff();
print a; // Prints 1
```

### Comments

Ultimately, the language supports comments opened with `/*` and closed with `*/`. Comments can be nested, so `/* /* */ */` is a legal comment.
