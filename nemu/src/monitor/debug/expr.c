#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <sys/types.h>

enum {
  NOTYPE = 256,
  EQ,

  /* TODO: Add more token types */
  NEQ,
  DEC,
  HEX,
  BIN,
  REG,
  PTR,
  VAR,
  NEG,
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", NOTYPE},            // spaces
    {"==", EQ},                // equal
    {"!=", NEQ},               // not equal
    {"\\+", '+'},              // plus
    {"-", '-'},                // minus
    {"\\*", '*'},              // multiply
    {"/", '/'},                // divide
    {"\\(", '('},              // left parenthesis
    {"\\)", ')'},              // right parenthesis
    {"!", '!'},                // not
    {"\\^", '^'},              // xor
    {"&", '&'},                // bit and
    {"\\|", '|'},              // bit or
    {"~", '~'},                // bit not
    {"<<", '<'},               // left shift
    {">>", '>'},               // right shift
    {"&&", '&'},               // and
    {"\\|\\|", '|'},           // or
    {"0x[0-9a-fA-F]+", HEX},   // hexadecimal
    {"0b[01]+", BIN},          // binary
    {"[0-9]+", DEC},           // decimal
    {"\\$[a-zA-Z]{2,3}", REG}, // register
    {"\\w+", VAR}              // variable
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg,
             rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 &&
          pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i,
            rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
        case NOTYPE:
          break;
        case '-':
          if (nr_token == 0 || (tokens[nr_token - 1].type != DEC &&
                                tokens[nr_token - 1].type != HEX &&
                                tokens[nr_token - 1].type != BIN &&
                                tokens[nr_token - 1].type != REG &&
                                tokens[nr_token - 1].type != VAR &&
                                tokens[nr_token - 1].type != ')')) {
            tokens[nr_token].type = NEG;
          } else {
            tokens[nr_token].type = '-';
          }
          nr_token++;
          break;
        case '*':
          if (tokens[nr_token - 1].type == DEC ||
              tokens[nr_token - 1].type == HEX ||
              tokens[nr_token - 1].type == BIN ||
              tokens[nr_token - 1].type == REG ||
              tokens[nr_token - 1].type == VAR ||
              tokens[nr_token - 1].type == ')') {
            tokens[nr_token].type = '*';
          } else {
            tokens[nr_token].type = PTR;
          }
          nr_token++;
		  break;
        case DEC:
        case HEX:
        case BIN:
        case REG:
        case VAR:
          if (substr_len >= 32) {
			printf("string too long\n");
            return false;
          }
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
        case '/':
        case '+':
        case '(':
        case ')':
        case '!':
        case '&':
        case '|':
        case '^':
        case '~':
        case '<':
        case '>':
        case EQ:
        case NEQ:
          tokens[nr_token].type = rules[i].token_type;
          nr_token++;
          break;
        default:
          panic("please implement me");
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  panic("please implement me");
  return 0;
}
