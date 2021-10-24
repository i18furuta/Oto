#include "token.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tokencode.h"
#include "../error/error.h"
#include "../util/util.h"
#include "../variable/variable.h"
#include "../variable/type.h"
#include "../sound/filter/filter.h"

/* 最初にlexerしておく記号・予約語 */
typedef struct symbol {
    int8_t *s;
    tokencode_t tc;
} Symbol;

static const Symbol symbols[] = {
    {"\n", TcLF},      {",",  TcComma},   {":", TcColon},
    {"[",  TcSqBrOpn}, {"]",  TcSqBrCls}, {"(", TcBrOpn}, {")",  TcBrCls},
    {"<-", TcLArrow},  {"->", TcRArrow},  {"=", TcEqu},   {"+",  TcPlus},
    {"-",  TcMinus},   {"*",  TcAster},   {"/", TcSlash}, {"%",  TcPerce},
    {"==", TcEEq},     {"!=", TcNEq},     {">", TcGt},    {"<", TcLt},
    {">=", TcGe},      {"<=", TcLe}
};

typedef struct rsvword {
    int8_t *lower;
    int8_t *upper;
    tokencode_t tc;
} Rsvword;

static const Rsvword rsvwords[] = {
    {"begin",     "BEGIN",     TcBegin     },
    {"end",       "END",       TcEnd       },
    {"define",    "DEFINE",    TcDefine    },
    {"func",      "FUNC",      TcFunc      },
    {"track",     "TRACK",     TcTrack     },
    {"filter",    "FILTER",    TcFilter    },
    {"if",        "IF",        TcIf        },
    {"elsif",     "ELSIF",     TcElsif     },
    {"else",      "ELSE",      TcElse      },
    {"then",      "THEN",      TcThen      },
    {"loop",      "LOOP",      TcLoop      }, 
    {"and",       "AND",       TcAnd       },
    {"or",        "OR",        TcOr        },
    {"not",       "NOT",       TcNot       },
    {"oscil",     "OSCIL",     TcOscil     },
    {"sound",     "SOUND",     TcSound     },
    {"print",     "PRINT",     TcPrint     },
    {"BEEP",      "BEEP",      TcBeep      },
    {"play",      "PLAY",      TcPlay      },
    {"note",      "NOTE",      TcNote      },
    {"mute",      "MUTE",      TcMute      },
    {"bpm",       "BPM",       TcBpm       },
    {"printwav",  "PRINTWAV",  TcPrintwav  },
    {"exportwav", "EXPORTWAV", TcExportwav },
    {"importwav", "IMPORTWAV", TcImportwav },
    {"defse",     "DEFSE",     TcDefse     },
    {"spectrum",  "SPECTRUM",  TcSpectrum  },
    {"setfs",     "SETFS",     TcSetfs     },
    {"midiin",    "MIDIIN",    TcMidiin    },
    {"midiout",   "MIDIOUT",   TcMidiout   },
    {"exit",      "EXIT",      TcExit      },
};

static tokencode_t get_rsvword_tc(int8_t *s, size_t len) {
    bool is_rw = false;
    uint32_t rw_list_num = GET_ARRAY_LENGTH(rsvwords);

    for (int32_t i = 0; i < rw_list_num; i++) {
        size_t rw_len = strlen(rsvwords[i].lower);
        if (len != rw_len) continue;

        is_rw = strncmp(rsvwords[i].lower, s, rw_len);
        if (is_rw == 0) return rsvwords[i].tc;

        is_rw = strncmp(rsvwords[i].upper, s, rw_len);
        if (is_rw == 0) return rsvwords[i].tc;
    }
    return 0;
}

/* ----------------------------- */
static Token token_list[MAX_TC] = {0};  // 変換済みトークン一覧
static uint32_t tcs = 0;  // 今まで発行したトークンコードの個数
static uint32_t tcb = 0;  // tcsBuf[]の未使用領域

static tokencode_t converted_src_tc[TC_LIST_SIZE];  // ソースコードをトークン列に変換したもの
static size_t converted_src_size = 0;

/* トークンの内容(文字列)を記憶するための領域 */
static char tcsbuf[(MAX_TC + 1) * 15];
/* ----------------------------- */

/* トークンを新しく追加する */
static void append_tc(tokencode_t tc, int8_t *s, size_t len) {
    if (tcs >= MAX_TC) {
        call_error(TOO_MANY_TOKEN_ERROR);
        return;
    }

    strncpy(&tcsbuf[tcb], (char *)s, len);

    token_list[tc].tc = tc;
    token_list[tc].tl = len;
    token_list[tc].ts = &tcsbuf[tcb];

    tcsbuf[tcb + len] = 0;
    tcb += len + 1;

    tcs++;
};

void set_tc(tokencode_t tc, uint32_t pc) {
    converted_src_tc[pc] = tc;
    converted_src_size++;
}

tokencode_t get_tc(uint32_t pc) {
    return converted_src_tc[pc];
}

size_t get_conv_source_size() {
    return converted_src_size + 1;
}

size_t get_conv_token_size() {
    return tcs;
}

size_t get_token_strlen(tokencode_t tc) {
    return token_list[tc].tl;
}

void print_token_str(tokencode_t tc) {
    if (tc == TcLF) {
        printf("\\n");
        return;
    }

    size_t len = get_token_strlen(tc);
    for (int32_t i = 0; i < len; i++) {
        printf("%c", token_list[tc].ts[i]);
    }
}


static bool init_flag = false;
tokencode_t allocate_tc(int8_t *s, size_t len, type_t type) {
    tokencode_t tc = 0;

    if (init_flag) {
        tc = get_rsvword_tc(s, len);
        if (tc != 0) return tc;
    }

    /* 登録済みの中から探す */
    for (tc = 0; tc < tcs; tc++) {
        if (len == token_list[tc].tl &&
            strncmp(s, token_list[tc].ts, len) == 0) {
            break;
        }
    }

    /* 新規作成時の処理 */
    if (tc == tcs) {
        append_tc(tc, s, len);

        double value = 0;
        if (type == TyConst) {
            value = strtod((char *)(token_list[tc].ts), 0);
        }

        assign_float(tc, type, value);
    }

    return tc;
}

/* OSCで指定する基本波の番号 */
void init_osc_wave() {
    tokencode_t tc;
    tc = allocate_tc("OSC_SINE_WAVE", strlen("OSC_SINE_WAVE"), TyConst);
    assign_float(tc, TyConst, 0);

    tc = allocate_tc("OSC_SAW_WAVE", strlen("OSC_SAW_WAVE"), TyConst);
    assign_float(tc, TyConst, 1);

    tc = allocate_tc("OSC_SQUARE_WAVE", strlen("OSC_SQUARE_WAVE"), TyConst);
    assign_float(tc, TyConst, 2);

    tc = allocate_tc("OSC_TRIANGLE_WAVE", strlen("OSC_TRIANGLE_WAVE"), TyConst);
    assign_float(tc, TyConst, 3);

    tc = allocate_tc("OSC_WHITE_NOISE", strlen("OSC_WHITE_NOISE"), TyConst);
    assign_float(tc, TyConst, 4);
}

void init_token() {
    if (init_flag) return;
    
    tokencode_t tc = 0;

    for (uint32_t i = 0; i < GET_ARRAY_LENGTH(symbols); i++) {
        tc = allocate_tc(symbols[i].s, strlen(symbols[i].s), TySymbol);
        TEST_EQ(tc, symbols[i].tc);
    }
    for (uint32_t i = 0; i < GET_ARRAY_LENGTH(rsvwords); i++) {
        tc = allocate_tc(rsvwords[i].lower, strlen(rsvwords[i].lower), TyRsvWord);
        TEST_EQ(tc, rsvwords[i].tc);
    }

    init_filter();
    init_osc_wave();
    
    init_flag = true;
}