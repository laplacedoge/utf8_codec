#ifndef __UTF8_CODEC_H__
#define __UTF8_CODEC_H__

#include <stdint.h>
#include <stddef.h>



typedef uint32_t ucs4_t;



/* Category of the code point width while encoding and decoding in UTF-8. */
typedef int utf8_kind_t;

/* No kind is specified yet. */
#define UTF8_KIND_NONE  0

/* About how many bytes are used to store a code point. */
#define UTF8_KIND_1B    1
#define UTF8_KIND_2B    2
#define UTF8_KIND_4B    4

/* Default kind. */
#define UTF8_KIND_DEF   UTF8_KIND_2B



typedef uint32_t utf8_opt_t;

#define UTF8_OPT_SET_KIND   1U



typedef int utf8_res_t;

/* All is well. */
#define UTF8_OK             0L

/* Common error. */
#define UTF8_ERR            -1L

/* No enough memory. */
#define UTF8_ERR_NO_MEM     -2L

/* Invalid operation code. */
#define UTF8_ERR_BAD_OPT    -3L

/* Invalid kind. */
#define UTF8_ERR_BAD_KIND   -4L

/* No enough buffer. */
#define UTF8_ERR_NO_BUFF    -5L

/* Invalid code point value. */
#define UTF8_ERR_BAD_CP     -6L



typedef struct _utf8_codec {
    utf8_kind_t kind;
    struct _utf8_codec_proc {
        ucs4_t code_point;
        size_t encoded_no_buff;
        size_t encoded_cp_num;
    } proc;
} utf8_codec_t;



utf8_res_t utf8_codec_init(utf8_codec_t *codec);

utf8_res_t utf8_codec_config(utf8_codec_t *codec,
                             utf8_opt_t opt, void *arg);

utf8_res_t utf8_encode(utf8_codec_t *codec,
                       uint8_t *buff, size_t max_buff_size,
                       const void *cp_array, size_t cp_num);

utf8_res_t utf8_decode(utf8_codec_t *codec,
                       void *cp_array, size_t max_cp_num,
                       uint8_t *enc_data, size_t encoded_no_buff);

#endif
