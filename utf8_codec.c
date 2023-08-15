#include <stdint.h>
#include <stddef.h>

#include "utf8_codec.h"



/**
 * [First code point] [Last code point] [Byte 1] [Byte 2] [Byte 3] [Byte 4]
 *       U+0000             U+007F      0xxxxxxx 
 *       U+0080             U+07FF      110xxxxx 10xxxxxx 
 *       U+0800             U+FFFF      1110xxxx 10xxxxxx 10xxxxxx 
 *       U+10000           U+10FFFF     11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 */

/* Leading bits of 1st byte of UTF-8 encoding. */
#define UTF8_ENC_1ST_BYTE_LEADING_BITS_1B           ((uint32_t)0x00000000U)
#define UTF8_ENC_1ST_BYTE_LEADING_BITS_2B           ((uint32_t)0x000000C0U)
#define UTF8_ENC_1ST_BYTE_LEADING_BITS_3B           ((uint32_t)0x000000E0U)
#define UTF8_ENC_1ST_BYTE_LEADING_BITS_4B           ((uint32_t)0x000000F0U)

/* Leading bits of other bytes of UTF-8 encoding. */
#define UTF8_ENC_OTH_BYTE_LEADING_BITS              ((uint32_t)0x00000080U)



/* Trailing bit mask of 1st byte of UTF-8 encoding. */
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_1B     ((uint32_t)0x0000007FU)
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_2B     ((uint32_t)0x0000001FU)
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_3B     ((uint32_t)0x0000000FU)
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_4B     ((uint32_t)0x00000007U)

/* Trailing bit mask of other bytes of UTF-8 encoding. */
#define UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK        ((uint32_t)0x0000003FU)



/* Trailing bit length of 1st byte of UTF-8 encoding. */
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_LEN_1B      7U
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_LEN_2B      5U
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_LEN_3B      4U
#define UTF8_ENC_1ST_BYTE_TRAILING_BITS_LEN_4B      3U

/* Trailing bit length of other bytes of UTF-8 encoding. */
#define UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN         6U



/* Code point range for 1-byte encoding of UTF-8. */
#define UTF8_ENC_CP_1B_MIN      ((ucs4_t)0x00000000U)
#define UTF8_ENC_CP_1B_MAX      ((ucs4_t)0x0000007FU)

/* Code point range for 2-byte encoding of UTF-8. */
#define UTF8_ENC_CP_2B_MIN      ((ucs4_t)0x00000080U)
#define UTF8_ENC_CP_2B_MAX      ((ucs4_t)0x000007FFU)

/* Code point range for 3-byte encoding of UTF-8. */
#define UTF8_ENC_CP_3B_MIN      ((ucs4_t)0x00000800U)
#define UTF8_ENC_CP_3B_MAX      ((ucs4_t)0x0000FFFFU)

/* Code point range for 4-byte encoding of UTF-8. */
#define UTF8_ENC_CP_4B_MIN      ((ucs4_t)0x00010000U)
#define UTF8_ENC_CP_4B_MAX      ((ucs4_t)0x001FFFFFU)



utf8_res_t utf8_codec_init(utf8_codec_t *codec) {
    codec->kind = UTF8_KIND_DEF;

    return UTF8_OK;
}

utf8_res_t utf8_codec_config(utf8_codec_t *codec,
                             utf8_opt_t opt, void *arg) {
    switch (opt) {
        case UTF8_OPT_SET_KIND: {
            utf8_kind_t kind = *(utf8_kind_t *)arg;
            switch (kind) {
                case UTF8_KIND_1B:
                case UTF8_KIND_2B:
                case UTF8_KIND_4B:
                    codec->kind = kind;
                    break;

                default:
                    return UTF8_ERR_BAD_KIND;
            }
            break;
        }

        default:
            return UTF8_ERR_BAD_OPT;
    }

    return UTF8_OK;
}

utf8_res_t utf8_encode(utf8_codec_t *codec,
                       uint8_t *buff, size_t max_buff_size,
                       const void *cp_array, size_t cp_num) {
    void *lb_fetch_cp;
    ucs4_t code_point = 0;
    size_t cp_idx = 0;
    size_t encoded_data_size = 0;
    size_t remain_buff_size = max_buff_size;
    utf8_res_t res;

    switch (codec->kind) {
        case UTF8_KIND_1B:
            lb_fetch_cp = &&fetch_1b_cp;
            break;

        case UTF8_KIND_2B:
            lb_fetch_cp = &&fetch_2b_cp;
            break;

        case UTF8_KIND_4B:
            lb_fetch_cp = &&fetch_4b_cp;
            break;

        default:
            return UTF8_ERR_BAD_KIND;
    }

    for (cp_idx = 0; cp_idx < cp_num; cp_idx++) {
        goto *lb_fetch_cp;

    fetch_1b_cp:
        code_point = (ucs4_t)((uint8_t *)cp_array)[cp_idx];
        goto check_cp_range;

    fetch_2b_cp:
        code_point = (ucs4_t)((uint16_t *)cp_array)[cp_idx];
        goto check_cp_range;

    fetch_4b_cp:
        code_point = (ucs4_t)((uint32_t *)cp_array)[cp_idx];

    check_cp_range:
        if (code_point <= UTF8_ENC_CP_1B_MAX) {
            if (remain_buff_size == 0) {
                goto err_no_buff;
            }

            buff[encoded_data_size++] = (uint8_t)code_point;
        } else if (code_point <= UTF8_ENC_CP_2B_MAX) {
            if (remain_buff_size < 2) {
                goto err_no_buff;
            }

            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_1ST_BYTE_LEADING_BITS_2B | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 1)) & UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_2B));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | (code_point & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
        } else if (code_point <= UTF8_ENC_CP_3B_MAX) {
            if (remain_buff_size < 3) {
                goto err_no_buff;
            }

            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_1ST_BYTE_LEADING_BITS_3B | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 2)) & UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_3B));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 1)) & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | (code_point & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
        } else if (code_point <= UTF8_ENC_CP_4B_MAX) {
            if (remain_buff_size < 4) {
                goto err_no_buff;
            }

            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_1ST_BYTE_LEADING_BITS_4B | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 3)) & UTF8_ENC_1ST_BYTE_TRAILING_BITS_MASK_4B));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 2)) & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | ((code_point >> (UTF8_ENC_OTH_BYTE_TRAILING_BITS_LEN * 1)) & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
            buff[encoded_data_size++] = (uint8_t)(UTF8_ENC_OTH_BYTE_LEADING_BITS | (code_point & UTF8_ENC_OTH_BYTE_TRAILING_BITS_MASK));
        } else {
            res = UTF8_ERR_BAD_CP;
            goto exit;
        }

        remain_buff_size = max_buff_size - encoded_data_size;

        continue;

    err_no_buff:
        res = UTF8_ERR_NO_BUFF;
        goto exit;
    }

    res = UTF8_OK;

exit:
    codec->proc.code_point = code_point;
    codec->proc.encoded_no_buff = encoded_data_size;
    codec->proc.encoded_cp_num = cp_idx;

    return res;
}

utf8_res_t utf8_decode(utf8_codec_t *codec,
                       void *cp_array, size_t max_cp_num,
                       uint8_t *enc_data, size_t encoded_no_buff) {

    return UTF8_OK;
}
