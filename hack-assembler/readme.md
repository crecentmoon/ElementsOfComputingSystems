# Hack Assembly
This is a compiler for the Hack assembly code, compiles the assembly code to the binary machine code.
Written in C, and it is a project for the [Nand2Tetris](https://www.nand2tetris.org/).

# Steps to create the Hack Assembly
- Create the function to read the assembly code line by line.
- Create the parser, which parses the assembly code and generates the binary machine code.
    - if command type is A, then extract the symbol or decimal.
    - if command type is C, then extract the dest, comp and jump.
- Create the symbol table, which maps the label to the address.
- Create the main function, which calls the parser, code and symbol table to generate the binary machine code.

Hope this instruction is helpful for the one who is interested in this project.

定数　非負数で10進数
ユーザー定義シンボル　アルファベット　数字　アンダースコア　ドット　ドル記号　コロン　数字からは始まらない

コメント　//　行末まで

空白　空白文字と空行は無視

大文字　ニーモニックは大文字　ラベルは大文字　変数は小文字
小文字　
