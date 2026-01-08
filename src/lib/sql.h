/**
 * @file sql.h
 * @brief SQL related functions including a basic parser
 */

#pragma once

#include "pyc.h"

#define ICOL_URL 0
#define ICOL_HEADERS 1
#define ICOL_BODY 2

#define ICOL_BIT(i)  (1u << (i))

struct column_def {
    struct str name;
    struct str typename;
    struct str default_value;

    struct str *generated_always_as;
    size_t generated_always_as_len;
};

/**
 * Allocate the #column_def from user ARGC and ARGV, optionally writing out the number
 * resolved columns to NUM_COLUMNS if it isn't NULL.
 */
struct column_def *parse_column_defs(int argc, const char *const *argv,
                                     size_t *num_columns);

/**
 * @brief 
 */
char *resolve_hidden_col(struct column_def *column_defs, uint icol, int plan_mask, char *arg_value);
