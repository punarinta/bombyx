#ifndef _BOMBYX_EXPRESSION_H_
#define _BOMBYX_EXPRESSION_H_ 1

#include <math.h>
#include <string.h>
#include "common.h"
#include "larva.h"

void parse();

/******************************************************
        The code below is based on a parser by
        James Gregson (james.gregson@gmail.com)
 ******************************************************/

#include <setjmp.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 @brief define a threshold for defining true and false for boolean expressions on vars
*/
#if !defined(PARSER_BOOLEAN_EQUALITY_THRESHOLD)
#define PARSER_BOOLEAN_EQUALITY_THRESHOLD	(1e-10)
#endif

/**
 @brief maximum length for tokens in characters for expressions, define this in the compiler options to change the maximum size
*/
#if !defined(PARSER_MAX_TOKEN_SIZE)
#define PARSER_MAX_TOKEN_SIZE 256
#endif

/**
 @brief maximum number of arguments to user-defined functions, define this in the compiler opetions to change.
*/
#if !defined(PARSER_MAX_ARGUMENT_COUNT)
#define PARSER_MAX_ARGUMENT_COUNT 16
#endif

/**
 @brief definitions for parser true and false
*/
#define PARSER_FALSE 0
#define PARSER_TRUE  (!PARSER_FALSE)

/**
 @brief main data structure for the parser, holds a pointer to the input string and the index of the current position of the parser in the input
*/
typedef struct
{
	/** @brief input string to be parsed */
	const char *str; 
	
	/** @brief length of input string */
	unsigned int len;
	
	/** @brief current parser position in the input */
	unsigned int pos;
	
	/** @brief position to return to for exception handling */
	jmp_buf		err_jmp_buf;
	
	/** @brief error string to display, or query on failure */
	const char *error;

	bombyx_env_t *env;
} parser_data;

/**
 @brief convenience function for using the library, handles initialization and destruction. basically just wraps parser_parse().
 @param[in] expr expression to parse
 @return expression value
 */
void parse_expression(bombyx_env_t *env, const char *expr, size_t size);

/**
 @brief primary public routine for the library
 @param[in] expr expression to parse
 @return expression value
 */
void parser_parse( parser_data *pd );

/**
 @brief error function for the parser, simply bails on the code
 @param[in] error string to print
 */
void parser_error( parser_data *pd, const char *err );

/**
 @brief looks at a input character, potentially offset from the current character, without consuming any
 @param[in] pd input parser_data structure to operate on
 @param[in] offset optional offset for character, relative to current character
 @return character that is offset characters from the current input
 */
char parser_peek( parser_data *pd );
char parser_peek_n( parser_data *pd, int n );

/**
 @brief returns the current character, and advances the input position
 @param[in] pd input parser_data structure to operate on
 @return current character
 */
char parser_eat( parser_data *pd );
void parser_eat_whitespace( parser_data *pd );
void parser_skip(parser_data *pd);

/**
 @brief reads and converts a var precision floating point value in one of the many forms,
 e.g. +1.0, -1.0, -1, +1, -1., 1., 0.5, .5, .5e10, .5e-2
 @param[in] pd input parser_data structure to operate on
 @return parsed value as var precision floating point number
 */
void parser_read_var( parser_data *pd );

/**
 @brief reads arguments for the builtin functions, auxilliary function for 
 parser_read_builtin()
 @param[in] pd input parser_data structure to operate upon
 @return value of the argument that was read
 */
void parser_read_argument( parser_data *pd );

/**
 @brief reads and calls built-in functions, like sqrt(.), pow(.), etc.
 @param[in] pd input parser_data structure to operate upon
 @return resulting value
*/
void parser_read_builtin( parser_data *pd );

/**
 @brief attempts to read an expression in parentheses, or failing that a literal value
 @param[in] pd input parser_data structure to operate upon
 @return expression/literal value
 */
void parser_read_paren( parser_data *pd );

/**
 @brief attempts to read a unary operation, or failing that, a parenthetical or literal value
 @param[in] pd input parser_data structure to operate upon
 @return expression/literal value
*/
void parser_read_unary( parser_data *pd );

/**
 @brief attempts to read an exponentiation operator, or failing that, a parenthetical expression 
 @param[in] pd input parser_data structure to operate upon
 @return exponentiation value
 */
void parser_read_power( parser_data *pd );
	
/**
 @brief reads a term in an expression
 @param[in] pd input parser_data structure to operate on
 @return value of the term
 */
void parser_read_term( parser_data *pd );

/**
 @brief attempts to read an expression
 @param[in] pd input parser_data structure
 @return expression value
 */
void parser_read_expr( parser_data *pd );

/**
 @brief reads and performs a boolean comparison operations (<,>,<=,>=,==) if found
 @param[in] pd input parser_data structure
 @return sub-expression value
 */
void parser_read_boolean_comparison( parser_data *pd );

/**
 @brief reads and performs a boolean 'and' operation (if found)
 @param[in] pd input parser_data structure
 @return sub-expression value
*/
void parser_read_boolean_and( parser_data *pd );
	
/**
 @brief reads and performs a boolean or operation (if found)
 @param[in] pd input parser_data structure
 @return expression value
*/
void parser_read_boolean_or( parser_data *pd );

#ifdef __cplusplus
};
#endif

#endif