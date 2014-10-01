#include "expression.h"
#include "bytecode.h"
#include "block.h"
#include "sys.h"
#include "larva.h"

void parse()
{
    BYTE quote_opened = 0;
    BYTE br_level = 0;
    char *expression;
    size_t expression_size;
    size_t expression_start = code_pos;

    // find expression end, note that newline does don't count if it's inside a string
    while (code_pos < code_length)
    {
        if (!quote_opened)
        {
            if (code[code_pos] == '\n') break;
            else if (code[code_pos] == '(') ++br_level;
            else if (code[code_pos] == ')') --br_level;
            else if (code[code_pos] == ',' && !br_level) break;
        }

        if (code[code_pos] == '\'') quote_opened = !quote_opened;

        ++code_pos;
    }

    expression_size = code_pos - expression_start;
    expression = malloc(expression_size + 1);
    memcpy(expression, code + expression_start, expression_size);
    expression[expression_size] = '\0';

    parse_expression(expression, expression_size);
    free(expression);
}


/******************************************************
        The code below is based on a parser by
        James Gregson (james.gregson@gmail.com)
 ******************************************************/

void parse_expression(const char *expr, size_t size)
{
    if (!expr[0]) return;

    parser_data pd;
    pd.str = expr;
    pd.len = ++size;
    pd.pos = 0;
    pd.error = NULL;
    parser_parse(&pd);

    if (pd.error)
    {
        fprintf(stderr, "Failed to parse expression '%s'. ", expr);
        larva_error((char *)pd.error);
    }

    /*if (verbose)
    {
        fputs("expression [", stdout);
        fputs(expr, stdout);
	    fputs("] -> [", stdout);
	    var_echo(val);
	    puts("]");
	}*/
}

void parser_parse(parser_data *pd)
{
	// set the jump position and launch the parser
	if (!setjmp(pd->err_jmp_buf))
    {
	    parser_read_boolean_or(pd);
        parser_eat_whitespace(pd);

        if (pd->pos < pd->len - 1)
        {
            parser_error(pd, "Failed to reach end of input expression, likely malformed input");
        }
	}
	else
	{
		parser_error(pd, "Setjmp failed. It's dangerous to continue.");
	}
}

void parser_error(parser_data *pd, const char *err)
{
	pd->error = err;
	longjmp(pd->err_jmp_buf, 1);
}

char parser_peek(parser_data *pd)
{
    unsigned int p = pd->pos;
	if (p < pd->len) return pd->str[p];
	parser_error(pd, "Tried to read past end of string!");

	return 0;
}

char parser_peek_n(parser_data *pd, int n)
{
	if (pd->pos+n < pd->len) return pd->str[pd->pos+n];
	parser_error(pd, "Tried to read past end of string!");

	return 0;
}

char parser_eat(parser_data *pd)
{
	if (pd->pos < pd->len) return pd->str[pd->pos++];
	parser_error(pd, "Tried to read past end of string!");

	return 0;
}

void parser_eat_whitespace( parser_data *pd )
{
	while (isspace(parser_peek(pd))) parser_eat(pd);
}

void parser_skip(parser_data *pd)
{
	if (pd->pos < pd->len) pd->pos++;
	else parser_error(pd, "Tried to read past end of string!");
}

void parser_read_double(parser_data *pd)
{
	int pos = 0;
	char c, token[PARSER_MAX_TOKEN_SIZE];

	// read a leading sign
	c = parser_peek(pd);
	if (c == '+' || c == '-') token[pos++] = parser_eat(pd);

	// is a string?
	if (c == '\'')
	{
	    parser_skip(pd);

	    // read until closed
	    // TODO: support escaping
        while (parser_peek(pd) != c) token[pos++] = parser_eat(pd);
        token[pos] = '\0';

        bc_add_cmd(BCO_AS_STRING);
        bc_add_string(token);

        // the closing quote
        parser_skip(pd);
	}
	else
	{
        // read optional digits leading the decimal point
        while (isdigit(parser_peek(pd))) token[pos++] = parser_eat(pd);

        // read the optional decimal point
        c = parser_peek(pd);
        if (c == '.') token[pos++] = parser_eat(pd);

        // read optional digits after the decimal point
        while (isdigit(parser_peek(pd))) token[pos++] = parser_eat(pd);

        // read the exponent delimiter
        c = parser_peek(pd);
        if (c == 'e' || c == 'E')
        {
            token[pos++] = parser_eat(pd);

            // check if the exponent has a sign,
            // if so, read it
            c = parser_peek(pd);
            if (c == '+' || c == '-')
            {
                token[pos++] = parser_eat(pd);
            }
        }

        // read the exponent delimiter
        while (isdigit(parser_peek(pd))) token[pos++] = parser_eat(pd);

        parser_eat_whitespace(pd);

        // null-terminate the string
        token[pos] = '\0';

        // check that a double-precision was read, otherwise throw an error
        if (pos == 0) parser_error(pd, "Failed to read operand");

        double d = strtod(token, NULL);
        bc_add_cmd(BCO_AS_DOUBLE);
        bc_add_double(d);
    }
}

void parser_read_argument(parser_data *pd)
{
    parser_eat_whitespace(pd);
	// read the argument
	parser_read_expr(pd);

	parser_eat_whitespace(pd);

	// check if there's a comma
	if (parser_peek(pd) == ',') parser_skip(pd);

	parser_eat_whitespace(pd);
}

int parser_read_argument_list(parser_data *pd, int *num_args, var *args)
{
	char c;

	// set the initial number of arguments to zero
	*num_args = 0;
    parser_eat_whitespace(pd);

	while (parser_peek(pd) != ')')
    {
		// check that we haven't read too many arguments
		if (*num_args >= PARSER_MAX_ARGUMENT_COUNT) parser_error(pd, "Exceeded maximum argument count for function call, increase PARSER_MAX_ARGUMENT_COUNT and recompile!");

		// read the argument and add it to the list of arguments
		/*var *this_arg = parser_read_expr(pd);
		args[*num_args] = *this_arg;
		*num_args = *num_args + 1;*/

        // no need to memorize args, they will be in the stack
		parser_read_expr(pd);

		parser_eat_whitespace(pd);

		// check the next character
		c = parser_peek(pd);
		if (c == ')')
        {
			// closing parenthesis, end of argument list, return
			// and allow calling function to match the character
			break;
	    }
	    else if (c == ',')
        {
			// comma, indicates another argument follows, match
			// the comma and continue
			// parsing arguments
			parser_skip(pd);
			parser_eat_whitespace(pd);
		}
		else
		{
			// invalid character, print an error and return
			parser_error(pd, "Expected ')' or ',' in function argument list!");
			return PARSER_FALSE;
		}
	}
	return PARSER_TRUE;
}

void parser_read_builtin(parser_data *pd)
{
	int num_args, pos = 0;
	char c, token[PARSER_MAX_TOKEN_SIZE];
	var args[PARSER_MAX_ARGUMENT_COUNT];

	c = parser_peek(pd);
	if (isalpha(c) || c == '_')
    {
		// alphabetic character or underscore, indicates that either a function
		// call or variable follows
		while (isalpha(c) || isdigit(c) || c == '_' || c == '.')
        {
			token[pos++] = parser_eat(pd);
			c = parser_peek(pd);
		}
		token[pos] = '\0';

		// check for an opening bracket, which indicates a function call
		if (parser_peek(pd) == '(')
        {
			// eat the bracket
			parser_skip(pd);

			// start handling the specific built-in functions
			if (memcmp(token, "print\0", 6) == 0)
            {
				parser_read_boolean_or(pd);
                bc_add_cmd(BCO_PRINT);
			}
			else if (memcmp(token, "swap\0", 5) == 0)
            {
            	/*v0 = parser_read_argument(pd);
				v1 = parser_read_argument(pd);

				var_t *i0 = var_lookup(vars, v0->name),
				      *i1 = var_lookup(vars, v1->name);

                // exchange data, data_size, type
                BYTE temp_type = i0->type;
                i0->type = i1->type;
                i1->type = temp_type;

                char *temp_data = malloc(i0->data_size);
                memcpy(temp_data, i0->data, i0->data_size);
                if (i0->data) free(i0->data);
                i0->data = malloc(i1->data_size);
                memcpy(i0->data, i1->data, i1->data_size);
                if (i1->data) free(i1->data);
                i1->data = malloc(i0->data_size);
                memcpy(i1->data, temp_data, i0->data_size);
                free(temp_data);

                unsigned int temp_data_size = i0->data_size;
                i0->data_size = i1->data_size;
                i1->data_size = temp_data_size;

                var_free(v1);*/
			}
			else if (memcmp(token, "microtime\0", 10) == 0)
            {
				bc_add_cmd(BCO_MICROTIME);
			}
			else
			{
			    // this is a 'block' function call

				parser_read_argument_list(pd, &num_args, args);

				bc_add_cmd(BCO_CALL);
				bc_add_token(token);
			}

			// eat closing bracket of function call
			if (parser_eat(pd) != ')') parser_error(pd, "Expected ')' in function call.");
		}
		else
		{
		    bc_add_cmd(BCO_AS_VAR);
            bc_add_token(token);
		}
	}
	else
	{
		// not a built-in function call, just read a literal double
		parser_read_double(pd);
	}

	parser_eat_whitespace(pd);
}

void parser_read_paren(parser_data *pd)
{
	// check if the expression has a parenthesis
	if (parser_peek(pd) == '(')
    {
		// eat the character
		parser_skip(pd);

		parser_eat_whitespace(pd);

		// if there is a parenthesis, read it
		// and then read an expression, then
		// match the closing brace
		parser_read_boolean_or(pd);

		parser_eat_whitespace(pd);

		// match the closing brace
		if (parser_peek(pd) != ')') parser_error(pd, "Expected ')'!");
		parser_skip(pd);
	}
	else
	{
		// otherwise just read a literal value
		parser_read_builtin(pd);
	}

	parser_eat_whitespace(pd);
}

void parser_read_unary(parser_data *pd)
{
	char c = parser_peek(pd);

	if (c == '!')
    {
		// if the first character is a '!', perform a boolean not operation
		parser_skip(pd);
		parser_eat_whitespace(pd);
		parser_read_paren(pd);

		bc_add_cmd(BCO_INVERT);
	}
	else if (c == '-')
    {
    	if (parser_peek(pd) == '-')
    	{
			parser_skip(pd);
			parser_eat_whitespace(pd);
			parser_read_term(pd);
			bc_add_cmd(BCO_DECR);
    	}
    	else
    	{
    		puts("DEBUG: DOUBLE UNARY MINUS");
			// perform unary negation
			parser_skip(pd);
			parser_eat_whitespace(pd);
			parser_read_paren(pd);
			bc_add_cmd(BCO_UNARY_MINUS);
		}
	}
	else if (c == '+')
    {
    	if (parser_peek(pd) == '+')
    	{
			parser_skip(pd);
			parser_eat_whitespace(pd);
			parser_read_term(pd);
		    bc_add_cmd(BCO_INCR);
    	}
    	else
    	{
			// consume extra '+' sign and continue reading
			puts("DEBUG: DOUBLE UNARY PLUS");
			parser_skip(pd);
			parser_eat_whitespace(pd);
			parser_read_paren(pd);
    	}
	}
	else
	{
		parser_read_paren(pd);
	}

	parser_eat_whitespace(pd);
}

void parser_read_power(parser_data *pd)
{
	// read the first operand
	parser_read_unary(pd);

	parser_eat_whitespace(pd);

	// attempt to read the exponentiation operator
	/*while (parser_peek(pd) == '^')
    {
        var *v1 = var_as_double(1.0), *s = var_as_double(1.0);

		parser_skip(pd);
		parser_eat_whitespace(pd);

		// handles case of a negative immediately
		// following exponentiation but leading
		// the parenthetical exponent
		if (parser_peek(pd) == '-')
        {
			parser_skip(pd);
			var_set_double(s, -1.0);
			parser_eat_whitespace(pd);
		}

		// read the second operand
		var *term = parser_read_power(pd);

		op_multiply(s, term);
		op_copy(v1, s);

		// perform the exponentiation
		// TODO:
	//	v0 = pow(v0, v1);

	    parser_eat_whitespace(pd);

		var_free(term);
		var_free(v1);
        var_free(s);
	}*/
}

void parser_read_term(parser_data *pd)
{
	// read the first operand
	parser_read_power(pd);

	parser_eat_whitespace(pd);

	// check to see if the next character is a
	// multiplication or division operand
	char c = parser_peek(pd);

	while (c == '*' || c == '/')
    {
		// eat the character
		parser_skip(pd);
		parser_eat_whitespace(pd);

		// perform the appropriate operation
		if (c == '*')
        {
        	parser_read_power(pd);
			bc_add_cmd(BCO_MUL);
		}
		else if (c == '/')
        {
        	parser_read_power(pd);
			bc_add_cmd(BCO_DIV);
		}

		parser_eat_whitespace(pd);

		// update the character
		c = parser_peek(pd);
	}
}

void parser_read_expr(parser_data *pd)
{
	char c = parser_peek(pd);

	// handle unary minus
	if (c == '+' || c == '-')
    {
		parser_skip(pd);
		parser_eat_whitespace(pd);

		if (c == '-' && parser_peek(pd) != '-')
		{
			parser_read_term(pd);
			bc_add_cmd(BCO_UNARY_MINUS);
		}
		else parser_read_term(pd);
	}
	else
	{
		parser_read_term(pd);
	}

	parser_eat_whitespace(pd);

	// check if there is an addition or
	// subtraction operation following
	c = parser_peek(pd);
	while (c == '+' || c == '-')
    {
		// advance the input
		parser_skip(pd);
		parser_eat_whitespace(pd);

		// perform the operation
		if (c == '+')
        {
			parser_read_term(pd);
			bc_add_cmd(BCO_ADD);
		}
		else if (c == '-')
        {
        	// reset name for v0
			parser_read_term(pd);
			bc_add_cmd(BCO_SUB);
		}

		parser_eat_whitespace(pd);

		// update the character being tested in the while loop
		c = parser_peek(pd);
	}
}

void parser_read_boolean_comparison(parser_data *pd)
{
    parser_eat_whitespace(pd);

	// read the first value
	parser_read_expr(pd);

	parser_eat_whitespace(pd);

	// try to perform boolean comparison operator. Unlike the other operators
	// like the arithmetic operations and the boolean and/or operations, we
	// only allow one operation to be performed. This is done since cascading
	// operations would have unintended results: 2.0 < 3.0 < 1.5 would
	// evaluate to true, since (2.0 < 3.0) == 1.0, which is less than 1.5, even
	// though the 3.0 < 1.5 does not hold.
	char c = parser_peek(pd);

	if (c == '>' || c == '<')
    {
	    BYTE val;
        char oper[] = { '\0', '\0', '\0' };

		// read the operation
		oper[0] = parser_eat(pd);
		c = parser_peek(pd);
		if (c == '=') oper[1] = parser_eat(pd);

		parser_eat_whitespace(pd);

		// try to read the next term
		parser_read_expr(pd);

		// perform the boolean operations
		if (memcmp(oper, "<\0", 2) == 0)
        {
			bc_add_cmd(BCO_LESS);
		}
		else if (memcmp(oper, ">\0", 2) == 0)
        {
			bc_add_cmd(BCO_MORE);
		}
		else if (memcmp(oper, "<=\0", 3) == 0)
        {
			bc_add_cmd(BCO_LESS_EQ);
		}
		else if (memcmp(oper, ">=\0", 3) == 0)
        {
			bc_add_cmd(BCO_MORE_EQ);
		}
		else
		{
			parser_error(pd, "Unknown comparison operation.");
		}

		parser_eat_whitespace(pd);
	}
}

void parser_read_boolean_equality(parser_data *pd)
{
	parser_eat_whitespace(pd);

	// read the first value
	parser_read_boolean_comparison(pd);

	parser_eat_whitespace(pd);

	// try to perform boolean equality operator
	char c = parser_peek(pd);

	if (c == '=' || c == '!')
    {
        char oper[] = { '\0', '\0', '\0' };

		if (c == '!')
        {
			// try to match '!=' without advancing input to not clobber unary not
			if (parser_peek_n(pd, 1) == '=')
            {
				oper[0] = parser_eat(pd);
				oper[1] = parser_eat(pd);
			}
			// TODO: analyse this case
			else return;
		}
		else
		{
			// try to match '=='
			oper[0] = parser_eat(pd);
			c = parser_peek(pd);
			if (c == '=')
			{
				oper[1] = parser_eat(pd);
				if (oper[1] != '=') parser_error(pd, "Expected a '=' for boolean '==' operator!");
			}
		}

		parser_eat_whitespace(pd);

		// try to read the next term
		parser_read_boolean_comparison(pd);

		// perform the boolean operations
		if (memcmp(oper, "==\0", 3) == 0)
        {
			bc_add_cmd(BCO_CMP);
		}
		else if (memcmp(oper, "!=\0", 3) == 0)
        {
			bc_add_cmd(BCO_CMP_NOT);
		}
		else if (memcmp(oper, "=\0", 2) == 0)
        {
            bc_add_cmd(BCO_SET);
		}
		else
		{
			parser_error(pd, "Unknown operation.");
		}

		parser_eat_whitespace(pd);
	}
}


void parser_read_boolean_and(parser_data *pd)
{
	// tries to read a boolean comparison operator (<, >, <=, >=)
	// as the first operand of the expression
	parser_read_boolean_equality(pd);

	parser_eat_whitespace(pd);

	// grab the next character and check if it matches an 'and'
	// operation. If so, match and perform and operations until
	// there are no more to perform
	char c = parser_peek(pd);

	while (c == '&')
    {
		// eat the first '&'
		parser_skip(pd);

		// check for and eat the second '&'
		c = parser_peek(pd);
		if (c != '&') parser_error(pd, "Expected '&' to follow '&' in logical AND operation!");
		parser_skip(pd);

		parser_eat_whitespace(pd);

		// read the second operand of the
		parser_read_boolean_equality(pd);

		// perform the operation, returning 1.0 for TRUE and 0.0 for FALSE
		//var_set_double(v0, (fabs(var_extract_double(v0)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD && fabs(var_extract_double(v1)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);
		bc_add_cmd(BCO_AND);

        parser_eat_whitespace(pd);

		// grab the next character to continue trying to perform 'and' operations
		c = parser_peek(pd);
	}
}

void parser_read_boolean_or(parser_data *pd)
{
	// read the first term
	parser_read_boolean_and(pd);

	parser_eat_whitespace(pd);

	// grab the next character and check if it matches an 'or'
	// operation. If so, match and perform and operations until
	// there are no more to perform
	char c = parser_peek(pd);

	while (c == '|')
    {
		// match the first '|' character
		parser_skip(pd);

		// check for and match the second '|' character
		c = parser_peek(pd);
		if (c != '|') parser_error(pd, "Expected '|' to follow '|' in logical OR operation!");
		parser_skip(pd);

		parser_eat_whitespace(pd);

		// read the second operand
		parser_read_boolean_and(pd);

		// perform the 'or' operation
		//var_set_double(v0, (fabs(var_extract_double(v0)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD || fabs(var_extract_double(v1)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);
		bc_add_cmd(BCO_OR);

        parser_eat_whitespace(pd);

		// grab the next character to continue trying to match
		// 'or' operations
		c = parser_peek(pd);
	}
}
