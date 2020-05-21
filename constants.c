#include <stddef.h>

const int BAD_USAGE_RETURN_VALUE = 2;
const size_t MAX_LINE_LENGTH = 4096;

// Remember at least 10 commands
const size_t MAX_HISTORY_LENGTH = 10 * MAX_LINE_LENGTH;

const int RETVAL_BAD_SYNTAX = 254;
const int RETVAL_ERROR = 3;
