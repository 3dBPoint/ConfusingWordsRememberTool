#ifndef PART_OF_SPEECH_H
#define PART_OF_SPEECH_H

#include "QString"

// n = 名词，noun的缩写
// u = 不可数名词，uncountable noun的缩写
// c = 可数名词，countable noun的缩写
// v = 动词，verb的缩写
// vi = 不及物动词，intransitive verb的缩写
// vt = 及物动词，transitive verb的缩写
// aux.v = 助动词，auxiliary verb的缩写
// model verb情态动词
// conj = 连接词，conjunction的缩写
// adj. = 形容词，adjective的缩写
// adv.= 副词，adverb的缩写
// art. = 冠词，article的缩写
// prep = 介词；preposition的缩写
// pron = 代名词，pronoun的缩写
// num = 数词，numeral的缩写
// int. = 感叹词，interjection的缩写

enum PartOfSpeech_e
{
    POS_ad,
    POS_art,
    POS_num,
    POS_prep,
    POS_pron,
    POS_n,
    POS_ad,
    POS_art,
    POS_num,
    POS_v,
    POS_conj,
    POS_vi,
    POS_vt,
    POS_aux_v,
    POS_a,
    POS_u,
    POS_c,
    POS_pl,
    POS_s,
    POS_sc,
    POS_o,
    POS_oc
};

struct PartOfSpeech_t
{
    QString fullName;
    PartOfSpeech_e abbr;
};

PartOfSpeech_t POS_Tbl[] =
{
    {"ad", POS_ad},
};

#endif