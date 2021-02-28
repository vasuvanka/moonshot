<img src="moonshot.svg" height="300px"/>

# Moonshot
An optionally-typed object-oriented language that extends Lua. This project allows you to use your favorite Lua libraries and frameworks with a better organized and error-safe syntax. This is a compiler, so it has no effect on the performance at runtime.

## Todo
- [x] Obtain a grammar for vanilla Lua code
- [x] Build a tokenizer for Lua source
- [x] Write parser structure to consume tokens and validate syntax
- [x] Test vanilla parser vs official Lua distribution
- [x] Write traversal algorithm to validate AST and output Lua code
- [x] Extend the Lua grammar for the new language
- [x] Add new language features to tokenizer, parser and traverser
- [ ] Implement a type checker
- [ ] Double check parser rules
- [ ] Split traversal into error checking and output phases
- [ ] Ensure that strings work in all scenarios, double check memory deallocation
- [ ] Integrate library into command line tool
- [ ] Double check the commenting
- [ ] Add a permissive license
- [ ] Package for deployment
- [ ] Design website
- [ ] Launch

## CLI options
- Organize output files after the input files or compress them into a single file
- Just compile into Lua or also run Lua after compilation
- Input from stdin or a file/directory
- Output code or just validate syntax

## Notes
- [Parts of a compiler](https://cs.lmu.edu/~ray/notes/compilerarchitecture/)
- Lua 5.3 [source code](https://www.lua.org/source/5.3/)
  - [parser](https://www.lua.org/source/5.3/lparser.c.html)

Lua grammar (reference)
----------

	block: { stat* }

	stat:
            `Do{ stat* }
          | `Set{ {lhs+} {expr+} }                    -- lhs1, lhs2... = e1, e2...
          | `While{ expr block }                      -- while e do b end
          | `Repeat{ block expr }                     -- repeat b until e
          | `If{ (expr block)+ block? }               -- if e1 then b1 [elseif e2 then b2] ... [else bn] end
          | `Fornum{ ident expr expr expr? block }    -- for ident = e, e[, e] do b end
          | `Forin{ {ident+} {expr+} block }          -- for i1, i2... in e1, e2... do b end
          | `Local{ {ident+} {expr+}? }               -- local i1, i2... = e1, e2...
          | `Localrec{ ident expr }                   -- only used for 'local function'
          | `Goto{ <string> }                         -- goto str
          | `Label{ <string> }                        -- ::str::
          | `Return{ <expr*> }                        -- return e1, e2...
          | `Break                                    -- break
          | apply

	expr:
            `Nil
          | `Dots
          | `Boolean{ <boolean> }
          | `Number{ <number> }
          | `String{ <string> }
          | `Function{ { `Id{ <string> }* `Dots? } block }
          | `Table{ ( `Pair{ expr expr } | expr )* }
          | `Op{ opid expr expr? }
          | `Paren{ expr }       -- significant to cut multiple values returns
          | apply
          | lhs

	apply:
             `Call{ expr expr* }
           | `Invoke{ expr `String{ <string> } expr* }

	lhs: `Id{ <string> } | `Index{ expr expr }

	opid:  -- includes additional operators from Lua 5.3 and all relational operators
            'add'  | 'sub' | 'mul'  | 'div'
          | 'idiv' | 'mod' | 'pow'  | 'concat'
          | 'band' | 'bor' | 'bxor' | 'shl' | 'shr'
          | 'eq'   | 'ne'  | 'lt'   | 'gt'  | 'le'   | 'ge'
          | 'and'  | 'or'  | 'unm'  | 'len' | 'bnot' | 'not'

Lua Grammar (Simplified)
----------
    stmt -> (function | if | set | call | while | repeat | local | goto | label | return | `break | do | fornum | forin)*
    expr -> lhs | `nil | `true | `false | number | string | function | table | operation | `paren expr `paren | call
    while -> `while expr `do stmt `end
    repeat -> `repeat stmt `until expr
    tuple -> expr (`comma expr)+
    number -> `int+ (`dot `int+)?
    set -> lhs `equal expr
    string -> `quote whatever `quote
    lhs -> `name | `name sub+ | `name (`comma `name)+
    sub -> (`square expr `square | `dot `name)
    operation ->
    function -> `function `name (`dot `name)* `paren expr* `paren stmt `do
    table -> `curly (`name `equal expr (`comma `name `equal expr)* )? `curly)
    if -> `if expr `then stmt (`elseif expr `then stmt)* (`else stmt)? `end
    call -> lhs `paren expr* `paren
    local -> `local lhs (`equal expr)?
    goto -> `goto `name
    label -> `dbcolon `name `dbcolon
    return -> `return expr?
    do -> `do stmt `end
    fornum -> `for `name `equal expr `comma expr (`comma expr)? `do stmt `end
    forin -> `for `name (`comma `name)+ `in expr (`comma expr)+ `do stmt `end

Moonshot Grammar
----------
    stmt -> (function | if | set | call | while | repeat | local | goto | label | return | `break | do | fornum | forin | typedef | define)*

    type -> basic_type | (`paren basic_type (`comma basic_type)* `paren)
    basic_type -> (`var | `name | (type (`paren (type (`comma type)* )? `paren)?)) (`!)?
    ltuple -> (type | `var)? `name (`comma (type | `var)? `name)*

    interface -> `interface `name (`extends `name)? `where interface_func+ `end
    interface_func -> (`function | type) `name `paren ltuple `paren
    class -> `class `name (`extends `name)? (`implements `name (`comma `name)* )? `where (function | define)* `end

    typedef -> `typedef `name type
    define -> `final? type `name (`equal expr)?
    local -> `local lhs (`equal expr)?
    call -> lhs `paren expr* `paren
    set -> lhs `equal expr

    goto -> `goto `name
    label -> `dbcolon `name `dbcolon
    return -> `return expr?

    lhs -> `name | `name sub+ | `name (`comma `name)+
    sub -> (`square expr `square | `dot `name)

    expr -> lhs | `nil | `true | `false | number | string | function | table | operation | `paren expr `paren | call | `new call
    function -> type? `function (`name (`dot `name)* )? `paren ltuple `paren stmt `do
    table -> `curly (`name `equal expr (`comma `name `equal expr)* )? `curly)
    operation -> * unary or binary operations *
    string -> `quote whatever `quote
    number -> `int+ (`dot `int+)?
    tuple -> expr (`comma expr)+

    fornum -> `for `name `equal expr `comma expr (`comma expr)? `do stmt `end
    forin -> `for `name (`comma `name)+ `in expr (`comma expr)+ `do stmt `end
    if -> `if expr `then stmt (`elseif expr `then stmt)* (`else stmt)? `end
    while -> `while expr `do stmt `end
    repeat -> `repeat stmt `until expr
    do -> `do stmt `end
