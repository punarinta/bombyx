#include "text.h"
#include "../../core/array_2.h"

#define LIBRARY_VERSION 0.1

/**
 * Report library version.
 *
 * @return double
 */
var version_()
{
    var v = {0};

    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    *(double *)v.data = LIBRARY_VERSION;

    return v;
}

/**
 * Performs string replacement.
 *
 * @version 0.1
 *
 * @param string where
 * @param string old
 * @param string new
 *
 * @return mixed
 */
var replace_(bombyx_env_t *env, BYTE argc, var *stack)
{
    // TODO: error processing
    // TODO: support replacement without return
    // TODO: support replacement limit
    // TODO: support replacement maps

    if (argc != 3 || stack[0].type != VAR_STRING || stack[1].type != VAR_STRING || stack[2].type != VAR_STRING)
    {
        return cocoon_error(env, "Function requires 3 arguments, type STRING.");
    }

    var v = {0};
    v.data = str_replace(stack[0].data, stack[1].data, stack[2].data);
    v.data_size = strlen(v.data);
    v.type = VAR_STRING;

    return v;
}

/**
 * Trims the string both sides.
 *
 * @param string text
 *
 * @return string
 */
var trim_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc != 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Parameters should be of type STRING.");
    }

    var v = {0};
    v.data = malloc(stack[0].data_size);
    memcpy(v.data, stack[0].data, stack[0].data_size);
    trim(v.data);
    v.data_size = strlen(v.data) + 1;
    v.type = VAR_STRING;

    return v;
}

/**
 * Counts the length of a string in symbols (not in bytes).
 *
 * @param string text
 *
 * @return double
 */
var length_(bombyx_env_t *env, BYTE argc, var *stack)
{
    if (argc != 1 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Parameters should be of type STRING.");
    }

    var v = {0};
    v.type = VAR_DOUBLE;
    v.data_size = sizeof(double);
    v.data = malloc(sizeof(double));
    *(double *)v.data = (double)cp_strlen_utf8(stack[0].data);

    return v;
}

/**
 * Splits the string using the separator and packs everything into an array.
 *
 * @param string text
 * @param string separator
 *
 * @return array
 */
var split_(bombyx_env_t *env, BYTE argc, var *stack)
{
    // TODO: support 1 argument mode — split onto letters (not bytes)

    if (argc != 2 || stack[0].type != VAR_STRING)
    {
        return cocoon_error(env, "Function requires 2 arguments, type STRING.");
    }

    var v = {0};
    char *saveptr;
    char *tok = NULL;
    v.type = VAR_ARRAY;
    v.data_size = sizeof(array_t);
    v.data = array_create(0);

    tok = strtok_r(stack[0].data, stack[1].data, &saveptr);

    while (tok)
    {
        var vt = {0};
        vt.type = VAR_STRING;
        vt.data_size = strlen(tok) + 1;
        vt.data = malloc(vt.data_size);
        memcpy(vt.data, tok, vt.data_size);

        array_push(v.data, vt);

        tok = strtok_r(NULL, stack[1].data, &saveptr);

        if (!tok) break;
    }

    return v;
}

/*
 *
 *  Credits: from http://www.daemonology.net/blog/2008-06-05-faster-utf8-strlen.html
 *
 */

#define ONEMASK ((size_t)(-1) / 0xFF)

static size_t cp_strlen_utf8(const char *_s)
{
	size_t u;
	const char * s;
	unsigned char b;
	size_t count = 0;

	/* Handle any initial misaligned bytes. */
	for (s = _s; (uintptr_t)(s) & (sizeof(size_t) - 1); s++)
	{
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0') goto done;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

	/* Handle complete blocks. */
	for (; ; s += sizeof(size_t))
	{
		/* Prefetch 256 bytes ahead. */
		__builtin_prefetch(&s[256], 0, 0);

		/* Grab 4 or 8 bytes of UTF-8 data. */
		u = *(size_t *)(s);

		/* Exit the loop if there are any zero bytes. */
		if ((u - ONEMASK) & (~u) & (ONEMASK * 0x80)) break;

		/* Count bytes which are NOT the first byte of a character. */
		u = ((u & (ONEMASK * 0x80)) >> 7) & ((~u) >> 6);
		count += (u * ONEMASK) >> ((sizeof(size_t) - 1) * 8);
	}

	/* Take care of any left-over bytes. */
	for (; ; s++)
	{
		b = *s;

		/* Exit if we hit a zero byte. */
		if (b == '\0') break;

		/* Is this byte NOT the first byte of a character? */
		count += (b >> 7) & ((~b) >> 6);
	}

done:
	return ((s - _s) - count);
}

#undef ONEMASK
