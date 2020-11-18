
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

/*
 * FORMALIZATION OF TOKENS IN CHEF：
 * Reserved keywords(* activates ing. parsing):
 *  Take*
 *  Put*
 *  Fold*
 *  Add(*)
 *  Remove*
 *  Combine*
 *  Divide*
 *  Liquefy(*)
 *  Stir*
 *  Mix*
 *  Clean*
 *  Pour*
 *  Set
 *  Serve
 *  Refridgerate
 *  Method
 *  Ingredients
 *  into
 *  to
 *  from
 *  for
 *  until
 *  the
 *  mixing bowl
 *  baking sheet
 *  Serves
 * Ingredient names:
 *  Anything after a number on a line after the Ingredients keyword but before the Method keyword
 *  includes spaces
 *  Anything after a keyword which activates ingredient parsing
 * Routine (recipe) names:
 *  Anything on the first line of the program
 *  Anything on the first line after the Serve or Refridgerate lines
 *  Identify subrecipes
 * Measures (* dry, + either):
 *  On the ingredients block after the amount
 *  Identifies ingredient type
     * g*
     * kg*
     * pinch[es]*
     * ml
     * l
     * dash[es]
     * cup[s]+
     * teaspoon[s]+
     * tablespoon[s]+
 * Integers:
 *  before the "mixing bowl" keyword in certain contexts
 *  before the "baking sheet" keyword in certain contexts
 *  after the "for" keyword
 * Separators:
 * . – the line end separator
 */

void lex(const char* prog) {
    size_t tokenc;
    Token* tokenv = malloc(tokenc*sizeof(Token));
}
