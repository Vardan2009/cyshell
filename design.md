# cyShell syntax

## Brackets

```
{   } => command grouping (doesnt eval to anything, execute commands sequentially, also creates lexical scope)
{  }? => command grouping + evals to exit code
[   ] => subshell (doesnt eval to anything)
[  ]? => subshell + evals exit code
$(  ) => command substitution (evals to stdout)
(   ) => expression substitution (evals to result of expression inside)
! ... => expression command (doesnt eval to anything, command type)
& ... => background command (doesnt eval to anything, command type)
```

## Special "directives"

```sh
if <expr> [<cmd> or <cmdgroup>] [else [<cmd> or <cmdgroup>]]
for <var> in <expr> -> <expr> [<cmd> or <cmdgroup>]
```

## Default variables

```sh
const FSTDIN  # stdin  fd (0)
const FSTDOUT # stdout fd (1)
const FSTDERR # stderr fd (2)
const nil     # nil
```

## Default non-standard commands

```sh
let x $nil
dup $STDERR > $STDOUT
```

## Examples

```sh
if [init]? != 0 {
    echo Initialization failed!;
    exit 1;
}
```

```sh
let name $1;

if name == $nil {
    echo Usage: $0 <name>;
    exit 1
}

let greeting "Hello";
echo ($greeting + ", " + $name + "!")
```

```sh
let file $1;

if !{exists $file}? {
    echo File does not exist
    exit 1
}

if {readable $file}?
    echo File is readable;
else File is not readable
```

```sh
fn even {
    if $1 % 2 == 0
        return 0;
    else
        return 1

    # or directly return ($1 % 2)
}

let num 10;

if {even $num}?
    echo $num is even;
else
    echo $num is odd;
```

```sh
let count $(ls | wc -l);
echo There are $count files in this directory
```

```sh
& sleep 2;
& sleep 3;

wait;
echo All jobs finished
```

```sh
let sum;

for i in 0 -> 5 !$sum += $i;

echo sum is: $sum
```
