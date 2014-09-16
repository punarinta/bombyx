#include "expression.h"

char temp_error[256];

var parse()
{
    var result;

    BYTE quote_opened = 0;
    char *expression;
    size_t expression_size;
    size_t expression_start = code_pos;

    // find expression end, note that newline does don't count if it's inside a string
    while (code[code_pos])
    {
        if (code[code_pos] == '\'') quote_opened = !quote_opened;
        if (code[code_pos] == '\n' && !quote_opened) break;
        code_pos++;
    }

    expression_size = code_pos - expression_start;
    expression = malloc(sizeof(char) * (expression_size + 1));
    memcpy(expression, (void *)&code[expression_start], expression_size);
    expression[expression_size] = '\0';
    trim(expression);

    result = parse_expression(expression);

    if (verbose)
    {
		fprintf(stdout, "expression '%s' {", expression);
	    fprintf(stdout, "} -> ");
	    var_echo(result);
	    fprintf(stdout, "\n");
	}

    free(expression);

    return result;
}


/******************************************************
        The code below is based on a parser by
        James Gregson (james.gregson@gmail.com)
 ******************************************************/

var parse_expression(const char *expr)
{
	return parse_expression_with_callbacks(expr, NULL, NULL, NULL);
}

var parse_expression_with_callbacks(const char *expr, parser_variable_callback variable_cb, parser_function_callback function_cb, void *user_data)
{
	if (!strlen(expr)) return vars[0];

	var val;
	parser_data pd;
	parser_data_init(&pd, expr, variable_cb, function_cb, user_data);
	val = parser_parse(&pd);
	if (pd.error)
    {
		fprintf(stderr, "Failed to parse expression '%s'. ", expr);
		fprintf(stderr, "%s", pd.error);
		larva_error();
	}

	return val;
}

parser_data *parser_data_new(const char *str, parser_variable_callback variable_cb, parser_function_callback function_cb, void *user_data)
{
	parser_data *pd = malloc(sizeof(parser_data));
	if (!pd) return NULL;
	pd->str = str;
	pd->len = strlen(str) + 1;
	pd->pos = 0;
	pd->error = NULL;
	pd->user_data   = user_data;
	pd->variable_cb = variable_cb;
	pd->function_cb = function_cb;
	return pd;
}

int parser_data_init(parser_data *pd, const char *str, parser_variable_callback variable_cb, parser_function_callback function_cb, void *user_data)
{
	pd->str = str;
	pd->len = strlen(str) + 1;
	pd->pos = 0;
	pd->error = NULL;
	pd->user_data   = user_data;
	pd->variable_cb = variable_cb;
	pd->function_cb = function_cb;
	return PARSER_TRUE;
}

void parser_data_free(parser_data *pd)
{
	free(pd);
}

var parser_parse(parser_data *pd)
{
	// set the jump position and launch the parser
	if (!setjmp(pd->err_jmp_buf))
    {
	    var result = parser_read_boolean_or(pd);

        parser_eat_whitespace(pd);

        if (pd->pos < pd->len - 1)
        {
            parser_error(pd, "Failed to reach end of input expression, likely malformed input");

            // this is just to shut the compiler, in fact this line is never reached
            return vars[0];
        }
        else return result;
	}
	else
	{
		// error was returned, output a nan silently
		return vars[0]; //sqrt(-1.0);
	}
}

void parser_error(parser_data *pd, const char *err)
{
	pd->error = err;
	longjmp(pd->err_jmp_buf, 1);
}

char parser_peek(parser_data *pd)
{
	if (pd->pos < pd->len) return pd->str[pd->pos];
	parser_error(pd, "Tried to read past end of string!");

	return '\0';
}

char parser_peek_n(parser_data *pd, int n)
{
	if (pd->pos+n < pd->len) return pd->str[pd->pos+n];
	parser_error(pd, "Tried to read past end of string!");

	return '\0';
}

char parser_eat(parser_data *pd)
{
	if (pd->pos < pd->len) return pd->str[pd->pos++];
	parser_error(pd, "Tried to read past end of string!");

	return '\0';
}

void parser_eat_whitespace(parser_data *pd)
{
	while (isspace(parser_peek(pd))) parser_eat(pd);
}

var parser_read_double(parser_data *pd)
{
	char c, token[PARSER_MAX_TOKEN_SIZE];
	int pos = 0;
    var val = var_as_double(0.0);

	// read a leading sign
	c = parser_peek(pd);
	if (c == '+' || c == '-') token[pos++] = parser_eat(pd);

	// is a string?
	if (c == '\'')
	{
	    parser_eat(pd);

	    // read until closed
	    // TODO: support escaping
        while (parser_peek(pd) != /*'\''*/c) token[pos++] = parser_eat(pd);
        token[pos] = '\0';

        val = var_set_string(val, token);

        // the closing quote
        parser_eat(pd);
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

        // remove any trailing whitespace
        parser_eat_whitespace(pd);

        // null-terminate the string
        token[pos] = '\0';

        double d_val = 0.0;

        // check that a double-precision was read, otherwise throw an error
        if (pos == 0 || sscanf(token, "%lf", &d_val) != 1) parser_error(pd, "Failed to read real number");

        val = var_set_double(val, d_val);
    }

    // return the parsed value
	return val;
}

var parser_read_argument(parser_data *pd)
{
	char c;
	var val;

	// eat leading whitespace
	parser_eat_whitespace(pd);

	// read the argument
	val = parser_read_expr(pd);

	// read trailing whitespace
	parser_eat_whitespace(pd);

	// check if there's a comma
	c = parser_peek(pd);
	if (c == ',') parser_eat(pd);

	// eat trailing whitespace
	parser_eat_whitespace(pd);

	// return result
	return val;
}

int parser_read_argument_list(parser_data *pd, int *num_args, var *args)
{
	char c;

	// set the initial number of arguments to zero
	*num_args = 0;

	// eat any leading whitespace
	parser_eat_whitespace(pd);

	while (parser_peek(pd) != ')')
    {

		// check that we haven't read too many arguments
		if (*num_args >= PARSER_MAX_ARGUMENT_COUNT) parser_error(pd, "Exceeded maximum argument count for function call, increase PARSER_MAX_ARGUMENT_COUNT and recompile!");

		// read the argument and add it to the list of arguments
		args[*num_args] = parser_read_expr(pd);
		*num_args = *num_args+1;

		// eat any following whitespace
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
			// the comma, eat any remaining whitespace and continue
			// parsing arguments
			parser_eat(pd);
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

var parser_read_builtin(parser_data *pd)
{
	var v0 = var_as_double(0.0), v1 = var_as_double(0.0), args[PARSER_MAX_ARGUMENT_COUNT];
	char c, token[PARSER_MAX_TOKEN_SIZE];
	int num_args, pos = 0;

	c = parser_peek(pd);
	if (isalpha(c) || c == '_')
    {
		// alphabetic character or underscore, indicates that either a function
		// call or variable follows
		while (isalpha(c) || isdigit(c) || c == '_')
        {
			token[pos++] = parser_eat(pd);
			c = parser_peek(pd);
		}
		token[pos] = '\0';

		// check for an opening bracket, which indicates a function call
		if (parser_peek(pd) == '(')
        {
			// eat the bracket
			parser_eat(pd);

			// start handling the specific built-in functions
			if (strcmp(token, "swap") == 0)
            {
				v0 = parser_read_argument(pd);
				v1 = parser_read_argument(pd);
				unsigned int i0 = var_get_index(v0.name),
				             i1 = var_get_index(v1.name);

                // save names
                char *n0 = v0.name, *n1 = v1.name;

				var temp = vars[i0];
				vars[i0] = vars[i1];
				vars[i1] = temp;

				// restore names
				vars[i0].name = n0;
				vars[i1].name = n1;

				v0 = var_as_double(1);
			}
			else if (strcmp(token, "print") == 0)
            {
				v0 = parser_read_boolean_or(pd);

				var_echo(v0);
			}
			else if (strcmp(token, "microtime") == 0)
            {
				v0 = var_as_double(get_microtime());
			}
			else
			{
				parser_read_argument_list(pd, &num_args, args);
				if (pd->function_cb && pd->function_cb(pd->user_data, token, num_args, args, &v1))
                {
					v0 = v1;
				}
				else
				{
					sprintf(temp_error, "Unknown function '%s'.", token);
                	parser_error(pd, temp_error);
				}
			}

			// eat closing bracket of function call
			if (parser_eat(pd) != ')') parser_error(pd, "Expected ')' in function call.");
		}
		else if (parser_peek(pd) == '[')
		{
			// eat the bracket
        	parser_eat(pd);

        	unsigned int i = var_get_index(token);
        	if (!i)
        	{
        	    sprintf(temp_error, "Unknown variable '%s'.", token);
                parser_error(pd, temp_error);
        	}

            // array index
        	v0 = var_array_element(vars[i], var_to_dword(parser_read_argument(pd)));

			// eat closing bracket of function call
			if (parser_eat(pd) != ']') parser_error(pd, "Expected ']' in the array access.");
		}
		else
		{
			// no opening bracket, indicates a variable lookup
			if (pd->variable_cb != NULL && pd->variable_cb(pd->user_data, token, &v1))
            {
				v0 = var_assign(v0, v1);
			}
			else
			{
			    unsigned int i = var_get_index(token);
                if (i)
              	{
               	    v0 = var_assign(v0, vars[i]);
               	}
               	else
               	{
               		sprintf(temp_error, "Unknown variable '%s'.", token);
                  	parser_error(pd, temp_error);
               	}
			}
		}
	}
	else
	{
		// not a built-in function call, just read a literal double
		v0 = var_assign(v0, parser_read_double(pd));
	}

	// consume whitespace
	parser_eat_whitespace(pd);

	// return the value
	return v0;
}

var parser_read_paren(parser_data *pd)
{
	var val;

	// check if the expression has a parenthesis
	if (parser_peek(pd) == '(')
    {
		// eat the character
		parser_eat(pd);

		// eat remaining whitespace
		parser_eat_whitespace(pd);

		// if there is a parenthesis, read it
		// and then read an expression, then
		// match the closing brace
		val = parser_read_boolean_or(pd);

		// consume remaining whitespace
		parser_eat_whitespace(pd);

		// match the closing brace
		if (parser_peek(pd) != ')') parser_error(pd, "Expected ')'!");
		parser_eat(pd);
	}
	else
	{
		// otherwise just read a literal value
		val = parser_read_builtin(pd);
	}
	// eat following whitespace
	parser_eat_whitespace(pd);

	// return the result
	return val;
}

var parser_read_unary(parser_data *pd)
{
	char c;
	var v0;
	c = parser_peek(pd);
	
	if (c == '!')
    {
		// if the first character is a '!', perform a boolean not operation
#if !defined(PARSER_EXCLUDE_BOOLEAN_OPS)
		parser_eat(pd);
		parser_eat_whitespace(pd);
		v0 = parser_read_paren(pd);
		v0 = (fabs(var_to_double(v0)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? var_as_double(0.0) : var_as_double(1.0);
#else
		parser_error(pd, "Expected '+' or '-' for unary expression, got '!'");
#endif
	}
	else if (c == '-')
    {
    	if (parser_peek(pd) == '-')
    	{
			parser_eat(pd);
			v0 = parser_read_term(pd);
			v0 = var_decrement(v0);
    	}
    	else
    	{
			// perform unary negation
			parser_eat(pd);
			parser_eat_whitespace(pd);
			v0 = var_invert(parser_read_paren(pd));
		}
	}
	else if (c == '+')
    {
    	if (parser_peek(pd) == '+')
    	{
			parser_eat(pd);
			v0 = parser_read_term(pd);
			v0 = var_increment(v0);
    	}
    	else
    	{
			// consume extra '+' sign and continue reading
			parser_eat(pd);
			parser_eat_whitespace(pd);
			v0 = parser_read_paren(pd);
    	}
	}
	else
	{
		v0 = parser_read_paren(pd);
	}

	parser_eat_whitespace(pd);

	return v0;
}

var parser_read_power(parser_data *pd)
{
	var v0, v1 = var_as_double(1.0), s = var_as_double(1.0);

	// read the first operand
	v0 = parser_read_unary(pd);

	// eat remaining whitespace
	parser_eat_whitespace(pd);

	// attempt to read the exponentiation operator
	while (parser_peek(pd) == '^')
    {
		parser_eat(pd);

		// eat remaining whitespace
		parser_eat_whitespace(pd);

		// handles case of a negative immediately
		// following exponentiation but leading
		// the parenthetical exponent
		if (parser_peek(pd) == '-')
        {
			parser_eat(pd);
			s = var_as_double(-1.0);    // <- memory leak
			parser_eat_whitespace(pd);
		}

		// read the second operand
		v1 = var_assign(v1, var_multiply(s, parser_read_power(pd)));

		// perform the exponentiation
		// TODO:
	//	v0 = pow(v0, v1);

		// eat remaining whitespace
		parser_eat_whitespace(pd);
	}

	// return the result
	return v0;
}

var parser_read_term(parser_data *pd)
{
	char c;

	// read the first operand
	var v0 = parser_read_power(pd);

	// eat remaining whitespace
	parser_eat_whitespace(pd);

	// check to see if the next character is a
	// multiplication or division operand
	c = parser_peek(pd);
	while (c == '*' || c == '/')
    {   
		// eat the character
		parser_eat(pd);

		// eat remaining whitespace
		parser_eat_whitespace(pd);

		// perform the appropriate operation
		if (c == '*')
        {
			v0 = var_multiply(v0, parser_read_power(pd));
		}
		else if (c == '/')
        {
			v0 = var_divide(v0, parser_read_power(pd));
		}

		// eat remaining whitespace
		parser_eat_whitespace(pd);

		// update the character
		c = parser_peek(pd);
	}
	
	return v0;
}

var parser_read_expr(parser_data *pd)
{
	var v0 = var_as_double(0.0);
	char c;

	// handle unary minus
	c = parser_peek(pd);
	if (c == '+' || c == '-')
    {
		parser_eat(pd);
		parser_eat_whitespace(pd);
		if (c == '+') v0 = var_add(v0, parser_read_term(pd));
		else if (c == '-') v0 = var_subtract(v0, parser_read_term(pd));
	}
	else
	{
		v0 = var_assign(v0, parser_read_term(pd));
	}

	parser_eat_whitespace(pd);

	// check if there is an addition or
	// subtraction operation following
	c = parser_peek(pd);
	while (c == '+' || c == '-')
    {
		// advance the input
		parser_eat(pd);

		// eat any extra whitespace
		parser_eat_whitespace(pd);

		// perform the operation
		if (c == '+')
        {
			v0 = var_add(v0, parser_read_term(pd));
		}
		else if (c == '-')
        {
			v0 = var_subtract(v0, parser_read_term(pd));
		}

		// eat whitespace
		parser_eat_whitespace(pd);

		// update the character being tested in the while loop
		c = parser_peek(pd);
	}

	// return expression result
	return v0;
}

var parser_read_boolean_comparison(parser_data *pd)
{
	char c, oper[] = { '\0', '\0', '\0' };
	var v0, v1;
	BYTE val;

	// eat whitespace
	parser_eat_whitespace(pd);

	// read the first value
	v0 = parser_read_expr(pd);

	// eat trailing whitespace
	parser_eat_whitespace(pd);

	// try to perform boolean comparison operator. Unlike the other operators
	// like the arithmetic operations and the boolean and/or operations, we
	// only allow one operation to be performed. This is done since cascading
	// operations would have unintended results: 2.0 < 3.0 < 1.5 would
	// evaluate to true, since (2.0 < 3.0) == 1.0, which is less than 1.5, even
	// though the 3.0 < 1.5 does not hold.
	c = parser_peek(pd);
	if (c == '>' || c == '<')
    {
		// read the operation
		oper[0] = parser_eat(pd);
		c = parser_peek(pd);
		if (c == '=') oper[1] = parser_eat(pd);

		// eat trailing whitespace
		parser_eat_whitespace(pd);

		// try to read the next term
		v1 = parser_read_expr(pd);

		// perform the boolean operations
		if (strcmp(oper, "<") == 0)
        {
			val = var_is_less(v0, v1);
		}
		else if (strcmp(oper, ">") == 0)
        {
			val = var_is_more(v0, v1);
		}
		else if (strcmp(oper, "<=") == 0)
        {
			val = var_is_less_equal(v0, v1);
		}
		else if (strcmp(oper, ">=") == 0)
        {
			val = var_is_more_equal(v0, v1);
		}
		else
		{
			parser_error(pd, "Unknown comparison operation.");
		}

		v0 = var_set_double(v0, val * 1.0);

		// read trailing whitespace
		parser_eat_whitespace(pd);
	}
	
	return v0;
}

var parser_read_boolean_equality(parser_data *pd)
{
	char c, oper[] = { '\0', '\0', '\0' };
	var v0, v1;

	// eat whitespace
	parser_eat_whitespace(pd);

	// read the first value
	v0 = parser_read_boolean_comparison(pd);

	// eat trailing whitespace
	parser_eat_whitespace(pd);

	// try to perform boolean equality operator
	c = parser_peek(pd);
	if (c == '=' || c == '!')
    {
		if (c == '!')
        {
			// try to match '!=' without advancing input to not clobber unary not
			if (parser_peek_n(pd, 1) == '=')
            {
				oper[0] = parser_eat(pd);
				oper[1] = parser_eat(pd);
			}
			else
			{
				return v0;
			}
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

		// eat trailing whitespace
		parser_eat_whitespace(pd);

		// try to read the next term
		v1 = parser_read_boolean_comparison(pd);

		// perform the boolean operations
		if (strcmp(oper, "==") == 0)
        {
			v0 = var_set_double(v0, (fabs(var_to_double(v0) - var_to_double(v1)) < PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);
		}
		else if (strcmp(oper, "!=") == 0)
        {
			v0 = var_set_double(v0, (fabs(var_to_double(v0) - var_to_double(v1)) > PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);
		}
		else if (strcmp(oper, "=") == 0)
        {
			v0 = var_assign(v0, v1);
		}
		else
		{
			parser_error(pd, "Unknown operation!");
		}

		// read trailing whitespace
		parser_eat_whitespace(pd);
	}
	
	return v0;
}


var parser_read_boolean_and(parser_data *pd)
{
	char c;
	var v0, v1;

	// tries to read a boolean comparison operator (<, >, <=, >=)
	// as the first operand of the expression
	v0 = parser_read_boolean_equality(pd);

	// consume any whitespace befor the operator
	parser_eat_whitespace(pd);

	// grab the next character and check if it matches an 'and'
	// operation. If so, match and perform and operations until
	// there are no more to perform
	c = parser_peek(pd);
	
	while (c == '&')
    {
		// eat the first '&'
		parser_eat(pd);

		// check for and eat the second '&'
		c = parser_peek(pd);
		if (c != '&') parser_error(pd, "Expected '&' to follow '&' in logical and operation!");
		parser_eat(pd);

		// eat any remaining whitespace
		parser_eat_whitespace(pd);

		// read the second operand of the
		v1 = parser_read_boolean_equality(pd);

		// perform the operation, returning 1.0 for TRUE and 0.0 for FALSE
		v0 = var_set_double(v0, (fabs(var_to_double(v0)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD && fabs(var_to_double(v1)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);

		// eat any following whitespace
		parser_eat_whitespace(pd);

		// grab the next character to continue trying to perform 'and' operations
		c = parser_peek(pd);
	}

	return v0;
}

var parser_read_boolean_or(parser_data *pd)
{
	char c;
	var v0, v1;

	// read the first term
	v0 = parser_read_boolean_and(pd);

	// eat whitespace
	parser_eat_whitespace(pd);

	// grab the next character and check if it matches an 'or'
	// operation. If so, match and perform and operations until
	// there are no more to perform
	c = parser_peek(pd);
	
	while (c == '|')
    {
		// match the first '|' character
		parser_eat(pd);

		// check for and match the second '|' character
		c = parser_peek(pd);
		if (c != '|') parser_error(pd, "Expected '|' to follow '|' in logical or operation!");
		parser_eat(pd);

		// eat any following whitespace
		parser_eat_whitespace(pd);

		// read the second operand
		v1 = parser_read_boolean_and(pd);

		// perform the 'or' operation
		v0 = var_set_double(v0, (fabs(var_to_double(v0)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD || fabs(var_to_double(v1)) >= PARSER_BOOLEAN_EQUALITY_THRESHOLD) ? 1.0 : 0.0);

		// eat any following whitespace
		parser_eat_whitespace(pd);

		// grab the next character to continue trying to match
		// 'or' operations
		c = parser_peek(pd);
	}

	// return the resulting value
	return v0;
}
