#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <errno.h>
#include <stdbool.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, const char* literal) {
    int i = 1;
    const char* p = NULL;
    EXPECT(c, literal[0]);
    for (p = c->json; literal[i] != '\0'; p++, i++) {
        if (*p != literal[i]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json += i - 1;
    switch(literal[0]) {
        case 't':
            v->type = LEPT_TRUE;
            break;
        case 'f':
            v->type = LEPT_FALSE;
            break;
        case 'n':
            v->type = LEPT_NULL;
            break;
        default:
            return LEPT_PARSE_INVALID_VALUE;
    }
    return LEPT_PARSE_OK;
}

#if 0
static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
#endif

static int lept_parse_number(lept_context* c, lept_value* v) {
    const char* end;
    double ans;
    end = c->json;
    if (*end == '-') {
        end++;
    }
    if (*end == '0') {
        end++;
    } else {
        if (!ISDIGIT(*end)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (; ISDIGIT(*end); end++);
    }
    if (*end == '.') {
        end++;
        if (!ISDIGIT(*end)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (; ISDIGIT(*end); end++);
    }
    if (*end == 'E' || *end == 'e') {
        end++;
        if (*end == '+' || *end == '-') {
            end++;
        }
        if (!ISDIGIT(*end)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (; ISDIGIT(*end); end++);
    }
#if 0
    for (end = c->json; *end != '\0'; end++) {
        if (*end == '+') {
            if (!is_first_digit) {
                return LEPT_PARSE_INVALID_VALUE;
            }
        } else if (*end == '.') {
            if (end - 1 < c->json || !ISDIGIT(*(end - 1)) || !ISDIGIT(*(end + 1))) {
                return LEPT_PARSE_INVALID_VALUE;
            }
        } else if (*end == 'e' || *end == 'E') {
            end++;
            if (*end == '+' || *end == '-') {
                end++;
            }
            for (; ISDIGIT1TO9(*end); end++);
            break;
        } else if (!ISDIGIT(*end) && (*end != '-')){
            return LEPT_PARSE_INVALID_VALUE;
        } else {
            if (!is_first_digit && ISDIGIT(*end)) {
                is_first_digit = true;
                if (*end == '0' && ISDIGIT(*(end + 1))) {
                    return LEPT_PARSE_INVALID_VALUE;
                }
            }
        }
    }
#endif
    ans = strtod(c->json, NULL);
    if (errno == ERANGE) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    v->n = ans;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
#if 0
        case 't':  return lept_parse_true(c, v);
        case 'f':  return lept_parse_false(c, v);
        case 'n':  return lept_parse_null(c, v);
#endif
        case 't': return lept_parse_literal(c, v, "true");
        case 'f': return lept_parse_literal(c, v, "false");
        case 'n': return lept_parse_literal(c, v, "null");
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
