#include <stdio.h>
#include <stdint.h>

#include "../lexer/token.h"
#include "../lexer/tokencode.h"
#include "../variable/variable.h"
#include "../vm/opcode.h"
#include "../utils/util.h"

struct opcode_t {
    str_t name;
    uint32_t code;
};

static const struct opcode_t opcodes[] = {
    {"Nop",     OpNop     },
    {"CpyD",    OpCpyD    },
    {"CpyP",    OpCpyP    },
    {"Push",    OpPush    },
    {"PushC",   OpPushC   },
    {"Add",     OpAdd     },
    {"Sub",     OpSub     },
    {"Mul",     OpMul     },
    {"Div",     OpDiv     },
    {"Mod",     OpMod     },
    {"And",     OpAnd     },
    {"Or",      OpOr      },
    {"Add2",    OpAdd2    },
    {"Sub2",    OpSub2    },
    {"Mul2",    OpMul2    },
    {"Div2",    OpDiv2    },
    {"Mod2",    OpMod2    },
    {"Eq",      OpEq      },
    {"NEq",     OpNEq     },
    {"LtCmp",   OpLtCmp   },
    {"LtEqCmp", OpLtEqCmp },
    {"RiCmp",   OpRiCmp   },
    {"RiEqCmp", OpRiEqCmp },
    {"Loop",    OpLoop    },
    {"Jmp",     OpJmp     },
    {"Jz",      OpJz      },
    {"Jnz",     OpJnz     },
    {"Print",   OpPrint   },
    {"DefS",    OpDefS    },
    {"Beep",    OpBeep    },
    {"Play",    OpPlay    },
    {"Filter",  OpFilter  },
    {"Exit",    OpExit    },
};

void print_var_name(tokenbuf_t *tcbuf, uint32_t tc, int32_t field_width) {
    int32_t blank = (int32_t)(field_width - tcbuf->conv_tokens[tc]->tl);
    for (int32_t i = 0; i < blank; i++) {
        printf(" ");
    }
    for (int32_t i = 0; i < tcbuf->conv_tokens[tc]->tl; i++) {
        printf("%c", tcbuf->conv_tokens[tc]->ts[i]);
    }
}

void print_token_name(tokenbuf_t *tcbuf, uint32_t tc, uint32_t idx) {
    if (tc == TcLF) {
        printf("tc[%d] : \\n", idx);
    } else {
        printf("tc[%d] : ", idx);
        print_var_name(tcbuf, tc, 0);
    }
    printf("\n");
}

void print_opcodes(tokenbuf_t *tcbuf, var_t **ic) {
    var_t **p = ic;
    uint64_t ic_num = 0;

    printf("\nExecute code\n");
    while (1) {
        uint64_t op = (uint64_t)p[0];

        printf("%5I64u : %9s    ", ic_num, opcodes[op].name);

        for (int32_t i = 1; i <= 4; i++) {
            if ((OpLoop <= op && op <= OpJnz) && i == 1) {
                printf("%14I64u", (uint64_t)p[i]);
            } else if (p[i] == 0) {
                break;
            } else {
                print_var_name(tcbuf, p[i]->tc, 14);
            }
            printf("    ");
        }
        printf("\n");
 
        if (op == OpExit) {
            break;
        }

        ic_num += 5;
        p += 5;
    }
    printf("\n");
}

void print_converted_source(tokenbuf_t *tcbuf, uint32_t len) {
    printf("Converted source code\n");
    for (int32_t i = 0; i < len; i++) {
        uint32_t tc = tcbuf->tc_list[i];
        print_token_name(tcbuf, tc, i);
    }
    printf("\n");
}

void print_tokenlist(tokenbuf_t *tcbuf) {
    printf("Tokenize result\n");
    for (int i = 0; i < tcbuf->tcs; i++) {
        uint32_t tc = tcbuf->conv_tokens[i]->tc;
        print_token_name(tcbuf, tc, i);
    }
    printf("\n");
    
}

void print_rpn_tc(tokenbuf_t *tcbuf, uint32_t *rpn_tc_list, uint32_t len) {
    printf("rpn : ");
    for (int32_t i = 0; i < len; i++) {
        uint32_t tc = rpn_tc_list[i];
        print_var_name(tcbuf, tc, 0);      
    }
    printf("\n");
}
