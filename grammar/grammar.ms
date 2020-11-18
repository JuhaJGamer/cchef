.TL
Formalized grammar specification for the 
.BI chef
programming 
language
.AU
kamray23
.ND
.SH
Token streams
.LP
A chef program consists of a number of tokens arranged in a token stream.
The following figures will describe each token stream and available token.
.KS
.PP 
The program itself consists of zero or more recipes, 
analogous to functions in other languages.
The first of these is treated as the "main" recipe, 
as 
.CW main()
is in C.
.PS
ellipse "Program ="
line 0.2
arc cw
line 0.25
arc ->
box "recipe" from last arc
arc
CONT: arc
line 0.75
arc
move to start of CONT; line up 0.25; arc cw
line to start of 1st arc
line -> right 0.2 from end of last arc
circle rad 0.1"x"
.PE
.KE
.KS
.LP
The recipe might be one of the more complex parts,
being made up of several tokens separated by separators.
Of particular interest are the 
.CW method ,
which is the instructions, the
.CW ingredients ,
which are the variables,
and the 
.CW name ,
which is analogous to the function name in other languages.
.PS
ellipse "recipe ="
move down .5; move right .2;
left; SECNDLINE: arc; 
move to .e of 1st ellipse; right
line -> 0.2; box "name"
line -> 0.2; box width 0.3 "."
line -> 0.2; box "comment";
line -> 0.2; box width 1 "\"\\nIngredients\""
line -> 0.2; box width 0.3 "."
line -> 0.2; box "Ingredients"
line 0.2; arc cw; arc cw; line to start of SECNDLINE
move to end of SECNDLINE; down; 
arc ->; box "\"Method\""
line -> 0.2; box width 0.3 "."
line -> 0.2; box "method"
line -> 0.2; circle rad 0.1 "x"
.PE
.KE
.KS
.LP
The name is any string of characters until the next token, here the separator (full stop).
.PS
ellipse "name ="
line 0.2
arc cw
line 0.25
arc ->; box width 1 "any not '.'"
CONT: arc; arc; line 1; arc;
move to end of CONT; up; line 0.25;
arc cw
line -> 0.2; circle rad 0.1 "x" 
line to .e of 1st ellipse from end of last arc;
.PE
.KE
.KS
.LP
The comment and the name are essentially equal to the parser.
The comment however holds no semantic value whereas the name does.
.PS
ellipse "comment ="
line -> 0.2; box "name"
line -> 0.2; circle rad 0.1 "x"
.PE
.KE


