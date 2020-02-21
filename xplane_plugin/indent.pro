// .indent.pro http://www.gnu.org/software/indent/manual/indent.html
//
//
// don't force a blank line afer every block of declarations
// -nbad
--no-blank-lines-after-declarations
// force a blank line after procedure bodies
// -bap
--blank-lines-after-procedures
// prefer to break long lines before && or || operators
// -bbo
--break-before-boolean-operator
// Don't put newlines after commas in multiple declarations
// int foo, bar, baz;
// -nbc
--no-blank-lines-after-commas
// Dont't Break the line before all arguments in a declaration.
// -nbfda
--dont-break-function-decl-args
// K&R style braces
// if (foo) {
//     bar;
// }
// -br
--braces-on-if-line
// Put braces on struct declaration line.
//-brs
--braces-on-struct-decl-line
// cuddle 'else' to preceding }
// if (foo) {
//     bar;
// } else {
//     baz;
// }
// -ce
--cuddle-else
// cuddle 'while' to preceding '}'
// do {
//    foo;
// } while (bar);
// -cdw
--cuddle-do-while
// K&R style braces in struct declarations
// struct foo {
//    ...
// };
// -brs
--braces-on-struct-decl-line
// BUT the opening brace of a function is at the
// start of it's own line.  Consistent?  Nope.
// int foo(int bar)
// {
//     ...
// }
// -blf
--braces-on-func-def-line
// Start comments which follow code on a line, at column 33 (the
// default)
// -c33
--comment-indentation33
// Ditto for comments following declarations
// -cd33
--declaration-comment-column33
// Ditto for comments following cpp directives
// -cp33
--else-endif-column33
// Don't force comment delimiters onto their own lines
// -ncdb
--no-comment-delimiters-on-blank-lines
// 'case' statements are not indented relative to the switch
// switch (foo) {
// case BAR:
// }
// -cli0
--case-indentation0
// Don't put a space after a cast operator
// -ncs
--no-space-after-casts
// Indent block comments to their surrounding code
// -d0
--line-comments-indentation0
// put identifiers in declarations immediately after type
// int foo;
// -di1
--declaration-indentation1
// Don't format comments starting in column 1
-nfc1
// Don't format comments starting after column 1
// -nfca
--dont-format-comments
// try to break long lines where the original code did
// -hnl
--honour-newlines
// indentation is 3 characters (2 is too little, 4 is too much :-)
// -i3
--indent-level3
// tabs are 8 characters (the default)
// -ts8
--tab-size8
// Don't add extra indentation for multiple opening parens
// or K&R function declarations (not that we should have any)
// -ip0
--parameter-indentation0
// Maximum line length of long code lines
// -l80
--line-length120
// broken 2nd line of function arguments is indented
// to align with open parenthesis e.g.
// long_function_name(first_argument, second_argument,
//                    indented_third_argument)
// -lp
--continue-at-parentheses
// indent nested cpp directives
// #if X
// #    if Y
// #        define Z 1
// -ppi3
--preprocessor-indentation3
// goto labels start in column 0
// -il0
--indent-label0
// No space between a called function name and its args
// function_call(arg1, arg2)
// -npcs
--no-space-after-function-call-names
// No space between 'sizeof' and its arg
// sizeof(struct foo);
-nbs
// No space inside parentheses
// -nprs
--no-space-after-parentheses
// Put the type of a procedure at the start of the same
// line it's definition
// int foo(int bar)
// {
// -npsl
--dont-break-procedure-type
// Force a space between 'for' and the following parenthesis
// for (i=0 ; i<5 ; i++) {
// -saf
--space-after-for
// Force a space between 'if' and the following parenthesis
// if (foo != bar) {
// -sai
--space-after-if
// Force a space between 'while' and the following parenthesis
// while (foo) {
// -saw
--space-after-while
// Don't force a * at the beginning of each line of box comments
// -nsc
--dont-star-comments
// Don't swallow "optional" blank lines in the original code
// -nsob
--leave-optional-blank-lines
// Don't force a space before the ';' when it's the entire
// body of a 'for' or 'while'.
// -nss
--dont-space-special-semicolon
// Use spaces instead of tabs.
// -nut
--no-tabs

--ignore-newlines
