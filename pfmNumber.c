/* 
 * @file			pfmnum.c
 * @brief			pfm_number 함수
 * @target			
 * @exec-sys			
 * @exec-dir			
 *
 * @dep-header		
 * @dep-module		
 * @dep-table		
 * @dep-infile		
 * @dep-outfile		
 *
 * @history
 *  버   전  :  성  명  :  일    자  :  근 거 자 료  :       변       경       내       용       
 * ---------   --------   ----------   -------------   ------------------------------------------
 *  Ver 1.00   황재섭     20050422     NF3.0           CREATE
 *  Ver 2.00   서승우     20050522     NF3.0           wrapper version으로 재개발
 *  Ver 2.01   최동규     20050211     NF3.0           불필요한 변수 COPY 제거
 *  Ver 2.02   이한광     20120913                     테스트  20120913
 * @pcode
 *
 * 유의 : number formatting시에 맨뒤에 null이 들어감: HOW ? 
 *
 * TODO : 성능 테스트 및 tuning : calc, formatting
 * 
 */

/* --------------------------------------- include files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#if 1 /* IMS. 141014 : Multi Thread Utility */
#include <pthread.h>
#endif
#include "pfmNumberConst.h"
//#include "pfmNumber.h"

#ifdef _PFM_SHORTNAME_WRAPPER
#include "../64.pfmWrapperApi/pfmNumberWrapper.c"
#endif

#ifdef NH_API
#include "pfmNumberN.h"
#else
#include "pfmNumber.h"
#endif


/* -------------------------------- constant, macro definitions --------------------------------- */
#define PFMNUM_DEBUG
#define PFMNUM_TO_STRN          1       /* print용  */
#define PFMNUM_TO_CMASTRN       2       /* print용  */
#define PFMNUM_MAX_BUF_CNT      10      /* print용  */

/* long type이 64bit인지 32bit인지 설정 */
//#define LONG64BIT

#define MY_MIN(a, b)            ( (a) < (b) ? (a) : (b) )
#define ERROR_ARGUMENT_NULL                         -999
#define ERROR_TOO_BIG_BUFSIZE                       -998
#define ERROR_TOO_FEW_ARGUMENT                      -997
#define ERROR_TOO_BIG_LONG_VALUE                    -996
#define ERROR_WRONG_LENGTH                          -995
#define ERROR_NO_FRMT                               -994
#define ERROR_INVALID_FRMT                          -993
#define ERROR_INVALID_CONV_STR                      -992
#define ERROR_INVALID_CONV_STR2                     -991
#define ERROR_TOO_BIG_FRMT                          -990
#define PFMNUM_BUF_SIZE         256
#define PFMNUM_MAX_BUF_SIZE     1024
#define PFMNUM_EMSG_BUF_SIZE    256
#define PFMNUM_MAX_STR_LEN      50

#define PFM_CHECK_NULL(x) do {                  \
        if (x == NULL)  {                       \
            rc = ERROR_ARGUMENT_NULL;           \
            goto PFM_CATCH ;                    \
        }                                       \
    } while ( 0 )
#ifdef _AIX
#define    PFMNUM_VA_OP_NUM(IN_CNT, OUT_NUM, IN_NUM, IN_FN) do {    \
        long            _cnt   = (IN_CNT);                          \
        va_list        ap;                                          \
        pfmnum_t       _tmpnum;                                     \
        pfmnum_t       _x;                                          \
                                                                    \
        if (_cnt < 2)    {                                          \
            rc = ERROR_TOO_FEW_ARGUMENT;                            \
            goto PFM_CATCH;                                         \
        }                                                           \
                                                                    \
        va_start(ap, (IN_NUM));                                     \
        *(OUT_NUM) = (IN_NUM);                                      \
        pfm_inconv(OUT_NUM);                                        \
                                                                    \
        for ( _cnt-- ; _cnt > 0 ;_cnt--) {                          \
			memcpy( &_tmpnum, ap, sizeof(pfmnum_t) );               \
            va_arg(ap, pfmnum_t );                      \
            pfm_inconv(&_tmpnum);                                   \
            if ( (rc = IN_FN((OUT_NUM), *(OUT_NUM), _tmpnum) ) != RC_NRM) { \
                va_end(ap);                                         \
                goto PFM_CATCH;                                     \
            }                                                       \
        }                                                           \
                                                                    \
        va_end(ap);                                                 \
        pfm_outconv(OUT_NUM);                                       \
                                                                    \
        return RC_NRM;                                              \
      PFM_CATCH:                                                    \
        EMSG_INIT();                                                \
        EMSG_APND_LONG(IN_CNT);                                     \
        EMSG_APND_PTR(OUT_NUM);                                     \
        EMSG_APND_NUM(IN_NUM);                                      \
        va_start(ap, (IN_NUM));                                     \
        for (        ; _cnt > 0 ;_cnt--) {                          \
            _x = va_arg(ap, pfmnum_t );                             \
            EMSG_APND_NUM(_x);                                      \
        }                                                           \
                                                                    \
        va_end(ap);                                                 \
        return RC_ERR;                                              \
    } while(0)
#else

#define    PFMNUM_VA_OP_NUM(IN_CNT, OUT_NUM, IN_NUM, IN_FN) do {    \
        long            _cnt   = (IN_CNT);                          \
        va_list        ap;                                          \
        pfmnum_t       _tmpnum;                                     \
        pfmnum_t       _x;                                          \
                                                                    \
        if (_cnt < 2)    {                                          \
            rc = ERROR_TOO_FEW_ARGUMENT;                            \
            goto PFM_CATCH;                                         \
        }                                                           \
                                                                    \
        va_start(ap, (IN_NUM));                                     \
        *(OUT_NUM) = (IN_NUM);                                      \
        pfm_inconv(OUT_NUM);                                        \
                                                                    \
        for ( _cnt-- ; _cnt > 0 ;_cnt--) {                          \
            _tmpnum = (va_arg(ap, pfmnum_t ));                      \
            pfm_inconv(&_tmpnum);                                   \
            if ( (rc = IN_FN((OUT_NUM), *(OUT_NUM), _tmpnum) ) != RC_NRM) { \
                va_end(ap);                                         \
                goto PFM_CATCH;                                     \
            }                                                       \
        }                                                           \
                                                                    \
        va_end(ap);                                                 \
        pfm_outconv(OUT_NUM);                                       \
                                                                    \
        return RC_NRM;                                              \
      PFM_CATCH:                                                    \
        EMSG_INIT();                                                \
        EMSG_APND_LONG(IN_CNT);                                     \
        EMSG_APND_PTR(OUT_NUM);                                     \
        EMSG_APND_NUM(IN_NUM);                                      \
        va_start(ap, (IN_NUM));                                     \
        for (        ; _cnt > 0 ;_cnt--) {                          \
            _x = va_arg(ap, pfmnum_t );                             \
            EMSG_APND_NUM(_x);                                      \
        }                                                           \
                                                                    \
        va_end(ap);                                                 \
        return RC_ERR;                                              \
    } while(0)
#endif

#ifdef _AIX
#define MY_VA_ARG(DATA, VARG, TYPE)  do {                           \
			memcpy( &DATA, VARG, sizeof(TYPE) );                    \
            va_arg(VARG, TYPE );                                    \
	} while(0)
#else
#define MY_VA_ARG(DATA, VARG, TYPE)  do {                           \
            DATA = va_arg(VARG, TYPE );                             \
	} while(0)
#endif

#ifdef va_copy
#define MY_VA_START(VARG1, VARG2) va_copy( VARG1, VARG2 )
#define MY_VA_END(VARG)           va_end( VARG )
#else
#define MY_VA_START(VARG1, VARG2) memcpy( &(VARG1), &(VARG2), sizeof(va_list) )
#define MY_VA_END(VARG)           
#endif

#define    PFMNUM_VA_OP_NUM_V2(IN_CNT, OUT_NUM, IN_NUM, VARG, IN_FN) do { \
        long            _cnt   = (IN_CNT);                          \
        va_list        ap;                                          \
        pfmnum_t       _tmpnum;                                     \
                                                                    \
        if (_cnt < 2)    {                                          \
            rc = ERROR_TOO_FEW_ARGUMENT;                            \
            goto PFM_CATCH;                                         \
        }                                                           \
                                                                    \
        MY_VA_START(ap, VARG);                                      \
        *(OUT_NUM) = (IN_NUM);                                      \
        pfm_inconv(OUT_NUM);                                        \
                                                                    \
        for ( _cnt-- ; _cnt > 0 ;_cnt--) {                          \
            MY_VA_ARG(_tmpnum, ap, pfmnum_t );                      \
            pfm_inconv(&_tmpnum);                                   \
            if ( (rc = IN_FN((OUT_NUM), *(OUT_NUM), _tmpnum) ) != RC_NRM) { \
				MY_VA_END(ap);                                      \
                goto PFM_CATCH;                                     \
            }                                                       \
        }                                                           \
                                                                    \
		MY_VA_END(ap);                                              \
        pfm_outconv(OUT_NUM);                                       \
                                                                    \
        return RC_NRM;                                              \
                                                                    \
      PFM_CATCH:                                                    \
                                                                    \
        EMSG_INIT();                                                \
        EMSG_APND_LONG(IN_CNT);                                     \
        EMSG_APND_PTR(OUT_NUM);                                     \
        EMSG_APND_NUM(IN_NUM);                                      \
                                                                    \
        MY_VA_START(ap, VARG);                                      \
        for (        ; _cnt > 0 ;_cnt--) {                          \
            MY_VA_ARG(_tmpnum, ap, pfmnum_t );                      \
            EMSG_APND_NUM(_tmpnum);                                 \
        }                                                           \
                                                                    \
		MY_VA_END(ap);                                              \
        return RC_ERR;                                              \
    } while(0)

#ifdef _AIX
#define    PFMNUM_VA_OP2(IN_CNT, OUT_NUM, IN_NUM, IN_FN, TYPE) do { \
        long            _cnt   = IN_CNT;                            \
        long            rc    = RC_NRM;                             \
        va_list        ap;                                          \
        TYPE           _tmp;                                        \
                                                                    \
        if (_cnt < 2)    {                                          \
            rc = ERROR_TOO_FEW_ARGUMENT;                            \
            goto PFM_CATCH;                                         \
        }                                                           \
                                                                    \
        va_start(ap, IN_NUM);                                       \
        *OUT_NUM = IN_NUM;                                          \
        pfm_inconv(OUT_NUM);                                        \
                                                                    \
        for ( _cnt-- ; _cnt > 0 ;_cnt--) {                          \
			memcpy( &_tmp, ap, sizeof(TYPE) );                      \
            va_arg(ap, TYPE);                                       \
            if (rc = IN_FN(OUT_NUM, *OUT_NUM, _tmp) != RC_NRM) {    \
                va_end(ap);                                         \
                goto PFM_CATCH;                                     \
            }                                                       \
        }                                                           \
                                                                    \
                                                                    \
        va_end(ap);                                                 \
        pfm_outconv(OUT_NUM);                                       \
                                                                    \
        return RC_NRM;                                              \
      PFM_CATCH:                                                    \
        EMSG_INIT();                                                \
                                                                    \
        va_end(ap);                                                 \
        return RC_ERR;                                              \
    } while(0)

#else
#define    PFMNUM_VA_OP2(IN_CNT, OUT_NUM, IN_NUM, IN_FN, TYPE) do { \
        long            _cnt   = IN_CNT;                            \
        long            rc    = RC_NRM;                             \
        va_list        ap;                                          \
        TYPE           _tmp;                                        \
                                                                    \
        if (_cnt < 2)    {                                          \
            rc = ERROR_TOO_FEW_ARGUMENT;                            \
            goto PFM_CATCH;                                         \
        }                                                           \
                                                                    \
        va_start(ap, IN_NUM);                                       \
        *OUT_NUM = IN_NUM;                                          \
        pfm_inconv(OUT_NUM);                                        \
                                                                    \
        for ( _cnt-- ; _cnt > 0 ;_cnt--) {                          \
            _tmp = va_arg(ap, TYPE);                                \
            if (rc = IN_FN(OUT_NUM, *OUT_NUM, _tmp) != RC_NRM) {    \
                va_end(ap);                                         \
                goto PFM_CATCH;                                     \
            }                                                       \
        }                                                           \
                                                                    \
                                                                    \
        va_end(ap);                                                 \
        pfm_outconv(OUT_NUM);                                       \
                                                                    \
        return RC_NRM;                                              \
      PFM_CATCH:                                                    \
        EMSG_INIT();                                                \
                                                                    \
        va_end(ap);                                                 \
        return RC_ERR;                                              \
    } while(0)
#endif

#define NUM_TRY( ftn ) do {                     \
        rc = ftn ;                              \
        if (rc != ERROR_NONE) {                 \
            goto PFM_CATCH;                     \
        }                                       \
    } while ( 0 )
#define MY_TRY( ftn ) do {                      \
        myrc = ftn ;                            \
        if (myrc != RC_NRM) {                   \
            goto PFM_CATCH2;                    \
        }                                       \
    } while ( 0 )

#define EMSG_INIT() do {                        \
        char _buf[PFMNUM_BUF_SIZE];             \
                                                \
        pfmnum_emsg_init(rc);                   \
        sprintf(_buf, ":%s", __func__);         \
        pfmnum_emsg_apnd(_buf, strlen(_buf));   \
    } while ( 0 )

#define EMSG_INIT2() do {                       \
        pfmnum_emsg_init2();                    \
    } while ( 0 )

#define EMSG_APND_PTR( ptr ) do {                   \
        char _buf[PFMNUM_BUF_SIZE];                 \
                                                    \
        if (ptr == NULL) {                          \
            sprintf(_buf, ":%s is null", #ptr);     \
        }                                           \
        else {                                      \
            sprintf(_buf, ":%s[%p]", #ptr, ptr);    \
            pfmnum_emsg_apnd(_buf, strlen(_buf));   \
        }                                           \
    } while ( 0 )

#define EMSG_APND_STR( str ) do {                   \
        char _buf[PFMNUM_BUF_SIZE];                 \
                                                    \
        if (str == NULL) {                          \
            sprintf(_buf, ":%s is null", #str);     \
        }                                           \
        else {                                      \
            sprintf(_buf, ":%s[", #str);            \
            pfmnum_emsg_apnd(_buf, strlen(_buf));   \
            pfmnum_emsg_apnd(str, strlen(str));     \
            pfmnum_emsg_apnd("]", 1);               \
        }                                           \
    } while ( 0 )

#define EMSG_APND_STRN( str , len) do {                 \
        char _buf[PFMNUM_BUF_SIZE];                     \
                                                        \
        sprintf(_buf, ":%s[", #str);                    \
        pfmnum_emsg_apnd(_buf, strlen(_buf));           \
        if (len <= 0) {                                 \
            sprintf(_buf, "illegal strlen(%ld)", len);  \
            pfmnum_emsg_apnd(_buf, strlen(_buf));       \
        }                                               \
        else {                                          \
            pfmnum_emsg_apnd(str, len);                 \
        }                                               \
        pfmnum_emsg_apnd("]", 1);                       \
    } while ( 0 )

#define EMSG_APND_LONG( x ) do {                \
        char _buf[PFMNUM_BUF_SIZE];             \
                                                \
        sprintf(_buf, ":%s[%ld]", #x, x);       \
        pfmnum_emsg_apnd(_buf, strlen(_buf));   \
    } while ( 0 )

#define EMSG_APND_DOUBLE( x ) do {              \
        char _buf[PFMNUM_BUF_SIZE];             \
                                                \
        sprintf(_buf, ":%s[%e]", #x, x);       \
        pfmnum_emsg_apnd(_buf, strlen(_buf));   \
    } while ( 0 )

#define EMSG_APND_NUM( x ) do {                 \
        char _buf[PFMNUM_BUF_SIZE];             \
                                                \
        sprintf(_buf, ":%s[", #x);              \
        pfmnum_emsg_apnd(_buf, strlen(_buf));   \
        bzero(_buf, sizeof(_buf));              \
        pfmNumDump(_buf, &x);                   \
        pfmnum_emsg_apnd(_buf, strlen(_buf));   \
        pfmnum_emsg_apnd("]", 1);               \
    } while ( 0 )

#define PFM_MSGINIT(x) do {                     \
        sprintf(g_pfmnum_emsg, "%s", #x);       \
    } while ( 0 )

#ifdef PFMNUM_DEBUG
#define PRINT(...) do { printf(__VA_ARGS__); printf("\n"); } while ( 0 );
#else
#define PRINT(...) 
#endif
/* -------------------------------- constant, macro definitions --------------------------------- */
#define STACK_MAX  256

//#undef TEST

#ifdef TEST
#define PRINTF(...)   do {                                      \
        printf("%s:%s(%5d), ", __FILE__, __func__, __LINE__);   \
        printf(__VA_ARGS__);                                    \
        fflush(stdout);                                         \
    } while(0)
#else
#define PRINTF(...)
#endif
/* ----------------------------------- structure definitions ------------------------------------ */
const pfmnum_t      g_pfmnum_zero = { 0 };
const pfmnum_t      g_pfmnum_one  = { 0x02, 0xc1, { 0x81 } };
const pfmnum_t      g_pfmnum_ten  = { 0x02, 0xc1, { 0x8a } };

#if 1 /* IMS. 141014 : Multi Thread Utility */
 struct thread_idx_s {
	long thread_id ;
	long thread_idx;
};
typedef struct thread_idx_s thread_idx_t;



static int cmp_svr(const void *key, const void *list) {
        return ( *(long*)key - *(long*)list );
}


enum FLAG_PFM_NUM_THREAD_USE_TYPE { FLAG_NUM_CALC_NOT_CHECK=9, FLAG_PFM_NUM_THREAD_USE_Y=TRUE, FLAG_PFM_NUM_THREAD_USE_N=FALSE };
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER; /* Mutext Lock */
#endif

/* the base of natural logarithms */
#if NUMBER_MAX_PREC >= 48

/* 50자리        : 2.71828182845904523536028747135266249775724709369 */
const pfmnum_t      g_pfmnum_e    = { 0x1a, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xcb,
										  0xc8, 0xaf, 0x89, 0xa4, 0xda } }; 

#elif NUMBER_MAX_PREC >= 46

/* 50자리        : 2.718281828459045235360287471352662497757247093 */
const pfmnum_t      g_pfmnum_e    = { 0x19, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xcb,
										  0xc8, 0xaf, 0x89, 0x9e } }; 

#elif NUMBER_MAX_PREC >= 44

/* 44자리        : 2.7182818284590452353602874713526624977572470 */

const pfmnum_t      g_pfmnum_e    = { 0x17, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xcb,
										  0xc8, 0xaf } };
#elif NUMBER_MAX_PREC >= 42

/* 42자리        : 2.71828182845904523536028747135266249775724 */
const pfmnum_t      g_pfmnum_e    = { 0x17, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xcb,
										  0xc8, 0xa8 } };

#elif NUMBER_MAX_PREC >= 40

/* 40자리        : 2.718281828459045235360287471352662497757 */
const pfmnum_t      g_pfmnum_e    = { 0x16, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xcb,
										  0xc6 } };
#else /* NUMBER_MAX_PREC >= 38 */

/* 38자리 : 2.7182818284590452353602874713526624977  */
const pfmnum_t      g_pfmnum_e    = { 0x15, 0xc1, 
										{ 0x82, 0xc7, 0xd2, 0xd1, 0xd2, 0xd4, 0xbb, 0x84, 0xb4, 0xa3, 
										  0xa4, 0x82, 0xd7, 0xaf, 0x8d, 0xb4, 0xc2, 0x98, 0xe1, 0xc6 } }; 

#endif


char g_pfmnum_emsg        [PFMNUM_EMSG_BUF_SIZE + 1];

 
/* -------------------------------------- global variables -------------------------------------- */
/* ------------------------------------ function prototypes ------------------------------------- */
static void pfm_inconv(pfmnum_t *x);
static void pfm_outconv(pfmnum_t *x);
static long pfm_is_int_range(long in);
static void pfmnum_emsg_apnd(char *buf, long len);
static void pfmnum_emsg_init2(void);
static void pfmnum_emsg_init(long rc);
static int  pfm_zrstrconv(char *dest, char *src, long buf_size);
static int  pfm_strconv(char *dest, char *src, long buf_size);
static int pfm_cmastrconv(char *dest, char *src, long buf_size);
static void init_int_stack(void);
static long int_push(int int_t);
static long int_pop(void);
static long get_int_stack_top(void);
static long is_int_stack_empty(void);
static void init_num_stack(void);
static long num_push(pfmnum_t num_t);
static long num_pop(pfmnum_t *num_t);
//static long get_num_stack_top(pfmnum_t *num_t);
//static long is_num_stack_empty(void);
static long postfix(char *dst, char *src);
static int  is_operator(int ch);
static int  is_legal(char *expr);
static char *calc(char *expr);
//static long pfm_isnumber(char *str);
#if 0
static int pfm_format_align(char *dst, long dst_len, char *fmt, char *src, char align, char lpad_zero);
static int pfm_format_str(char *dst, char *fmt, char *src);
#else
static long pfm_format_align(char *dst, long dst_len, const char *fmt, char *src, char align, char lpad_zero);
static long pfm_format_str(char *dst, const char *fmt, char *src);
static long pfm_min_strlen(char *str, long len);
static long pfm_repeat_mul(pfmnum_t *v, pfmnum_t *x, long cnt);
#endif
#if 1 /* IMS. 141014 : Multi Thread Utility */
static long pfmNumThreadSupportYN( void );
static long pfmGetThreadIdx(long ThreadId);
#endif
#if 1 /* 20080731: MRH. NEW NUMBER */

/**************************************************************
	과거버전의 proMapper가 number_set_zero 를 사용하고 있는데,
	새 버전의 Number는 이 함수를 매크로 함수로 처리하고 있다.
	따라서, old 사이트의 경우에 Number를 패치하려면 
	proMapper도 같이 패치 하는 문제가 발생..
	이러한 영향도를 없애기 위해 여기에 해당 함수를 추가한다.
***************************************************************/
#ifdef number_set_zero
#undef number_set_zero
#endif

void number_set_zero( number_t *x )
{
	number_assign((x), _number_zero);
}
/**************************************************************/

static int
pfm_number_cmp_abs(number_t *x, number_t *y)
{
    number_t z;

    /* used TB_ASSERT instead of return ERROR_..INVALID_ARGUMENT
       because this function isn't supposed to return error code */
    TB_ASSERT(number_is_valid_ptr(x) && number_is_valid_ptr(y));

#ifdef NUMBER_NULLVAL_SUPPORTED
    /* if either x or y is null, normal comparison semantic does not apply
       because its 'size' field means nothing. since we want to support
       null-value equality comparison at least, return 0 if x==y==null
       and nonzero value otherwise. */
    if (number_is_null(x))
        return (!number_is_null(y));
    if (number_is_null(y))
        return 1;
#endif

    if (number_is_positive(x))
        if (number_is_positive(y))
            return number_cmp(x, y);
        else {
            number_assign(&z, y);
            number_negate(&z); 
            return number_cmp(x, &z); 
        }
    else
        if (number_is_positive(y)) {
            number_assign(&z, x);
            number_negate(&z);
            return number_cmp(&z, y);
        }
        else
            return (-(number_cmp(x, y)));
}

int
number_intdiv(number_t *x, number_t *y)
{
    int rc;

    TB_ASSERT(number_is_valid_ptr(x) && number_is_valid_ptr(y));

#ifdef NUMBER_NULLVAL_SUPPORTED
    if (number_is_null(x))
        return ERROR_NONE;
    if (number_is_null(y))
        return number_set_null(x);
#endif

#ifdef NUMBER_EXTVAL_SUPPORTED
    if (!number_is_ordinary(x) || !number_is_ordinary(y)) return ERROR_NUMBER_INVALID_ARGUMENT;
#endif

    if (!number_is_integer(x) || !number_is_integer(y))
        return ERROR_NUMBER_INVALID_ARGUMENT;
    if ((rc = number_div(x, y)) != ERROR_NONE)
        return rc;
    if ((rc = number_trunc(x)) != ERROR_NONE)
        return rc;
    return ERROR_NONE;
}

#endif

static char * 
pfmnum_nltrim(char * str, long len)
{
    long pos = 0;
    long pos2 = 0;
    
    if (str == NULL){
        return NULL;   
    }

    if (len < 1) {
        return str;
    }

    for (pos = 0; pos < len; pos++) { 
        if (!isspace(str[pos])) {
            break;
        }
    }
    
    if (pos == 0) {
        return str;
    }
    
    pos2 = 0;
    for (  ; pos < len; pos++) {
        str[pos2++] = str[pos];
    }

    for ( ; pos2 < len; pos2++) {
        str[pos2] = 0x00;
    }
    
    return str;
        
}
/* --------------------------------------- function body ---------------------------------------- */
static char * 
pfmnum_ltrim(char * str)
{
    return pfmnum_nltrim(str, strlen(str));
}
/* --------------------------------------- function body ---------------------------------------- */
static char * 
pfmnum_nrtrim(char * str, long len)
{
    long i;

    if (str == NULL){
        return NULL;   
    }

    if (len < 1) {
        return str;
    }

    len = pfm_min_strlen(str, len);

    for (i = len - 1; i >= 0; i--) {
        if (!isspace(str[i])) {
            break;
        }
        str[i] = '\0';
    }

    return str;
   
    
}
/* --------------------------------------- function body ---------------------------------------- */
static char * 
pfmnum_rtrim(char * str)
{
    
    return pfmnum_nrtrim(str, strlen(str));

}

static char *
pfmnum_nrmspace(char *str, long len)
{
    long src_len;
    long s, d;
    
    if (str == NULL || len <= 0) {
        return NULL;
    }

    src_len = pfm_min_strlen(str, len);

    for (s = 0; s < src_len; s++) {
        if (str[s] == 0x20) {
            break;
        }
    }
    d = s;

    for (s = s + 1; s < src_len; s++) {
        if (str[s] != 0x20) {
            str[d++] = str[s];
        }
    }

    memset(str + d, 0x00, len - d);

    return str;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAssign(pfmnum_t *x, pfmnum_t y) 
{
    long rc;

    PFM_CHECK_NULL(x);

    pfm_inconv(&y);

#if 0 /* 20080731: MRH. NEW NUMBER */
    NUM_TRY( number_assign(x, &y));
#else
    number_assign(x, &y);
#endif

    pfm_outconv(x);

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToLong(long *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);


    pfm_inconv(&y);

#ifdef LONG64BIT 
    NUM_TRY( number_to_int64(&y, x));
#else
    NUM_TRY( number_to_int32(&y, (int32_t*)x));
#endif

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromLong(pfmnum_t *x, long y)
{
    long rc ;

    PFM_CHECK_NULL(x);

#ifdef LONG64BIT
    NUM_TRY( number_from_int64(x, y));
#else
    NUM_TRY( number_from_int32(x, y));
#endif

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToInt16( int16_t *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);


    pfm_inconv(&y);

    NUM_TRY( number_to_int16(&y, x));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromInt16(pfmnum_t *x, int16_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    NUM_TRY( number_from_int16(x, y));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToInt32( int32_t *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);


    pfm_inconv(&y);

    NUM_TRY( number_to_int32(&y, x));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromInt32(pfmnum_t *x, int32_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    NUM_TRY( number_from_int32(x, y));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToInt64( int64_t *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);


    pfm_inconv(&y);

    NUM_TRY( number_to_int64(&y, x));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromInt64(pfmnum_t *x, int64_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    NUM_TRY( number_from_int64(x, y));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToDouble(double *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    pfm_inconv(&y);

    NUM_TRY( number_to_double(&y, (double*)x));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromDouble(pfmnum_t *x, double y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    NUM_TRY( number_from_double(x, y));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_DOUBLE(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long
longDoubleGetFromStr(long double *x, char *str)
{
    long rc ;

    pfmnum_t z;    
    pfm_inconv(&z);

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(str);

    /* null string 일때 0으로 */
    if (str[0] == 0x00) {
        bzero(x, sizeof(pfmnum_t));
        return RC_NRM;
    }
    bzero(x, sizeof(long double));
    PRINTF("[%s][len:%ld]\n", str, strlen(str));

    NUM_TRY( number_from_str(&z, (int)strlen(str), str, "N"));
    //NUM_TRY( number_to_long_double(&z, (long double*)x)); 
    NUM_TRY( number_to_long_double(&z, x)); 
    pfm_outconv(&z);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}


/* --------------------------------------- function body ---------------------------------------- */
long
longDoubleGetFromStr2(long double *x, char *str, long size, long decimal)
{
    long rc ;

    pfmnum_t z;
    pfm_inconv(&z);

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(str);

    /* null string 일때 0으로 */
    if (str[0] == 0x00) {
        bzero(x, sizeof(pfmnum_t));
        return RC_NRM;
    }
    bzero(x, sizeof(long double));

    PRINTF("[%s][len:%ld]\n", str, strlen(str));

    NUM_TRY( number_from_str(&z, (int)strlen(str), str, "N"));
    sscanf(pfmNumPrint(z), "%Lf", x);
    //NUM_TRY( number_to_long_double2(&z, x, size, decimal));
//    printf("longDoubleGetFromStr2 x value after  %33.5Lf \n ", *x);
    printf("longDoubleGetFromStr2 x value after  %33.5Lf \n ", *x);
    pfm_outconv(&z);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumToLongDouble(long double *x, pfmnum_t y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    pfm_inconv(&y);

    NUM_TRY( number_to_long_double(&y, (long double*)x));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumGetFromLongDouble(pfmnum_t *x, long double y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    NUM_TRY( number_from_long_double(x, y));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_DOUBLE(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromDouble2(pfmnum_t *x, double y, long precision)
{
    long rc ;
	pfmnum_t tmp;

    PFM_CHECK_NULL(x);

	rc = pfmNumGetFromDouble( &tmp, y );
	if ( rc != RC_NRM ) goto PFM_CATCH;

	rc = pfmNumRoundAt( x, tmp, precision );
	if ( rc != RC_NRM ) goto PFM_CATCH;

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_DOUBLE(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumGetFromLongDouble2(pfmnum_t *x, long double y, long precision)
{
    long rc ;
    pfmnum_t tmp;

    PFM_CHECK_NULL(x);

    rc = pfmNumGetFromLongDouble( &tmp, y );
    if ( rc != RC_NRM ) goto PFM_CATCH;

    rc = pfmNumRoundAt( x, tmp, precision );
    if ( rc != RC_NRM ) goto PFM_CATCH;

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_DOUBLE(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromStr(pfmnum_t *x, char *str)
{
    long rc ;
    
    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(str);

    /* null string 일때 0으로 */
    if (str[0] == 0x00) {
        bzero(x, sizeof(pfmnum_t));
        return RC_NRM;
    }

    PRINTF("[%s][len:%ld]\n", str, strlen(str));

    NUM_TRY( number_from_str(x, (int)strlen(str), str, "N"));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetFromStrn(pfmnum_t *x, char *str, long len)
{
    long rc ;

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(str);

    if (len <= 0) {

        rc = ERROR_WRONG_LENGTH;
        
        goto PFM_CATCH;
    }

    /* null string 일때 0으로 */
    if (str[0] == 0x00) {
        bzero(x, sizeof(pfmnum_t));
        return RC_NRM;
    }

    NUM_TRY( number_from_str(x, (int)len, str, "N"));

    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumGetStrLPadZeroN(char *str, pfmnum_t x, long buf_size)
{
    long rc ;

    char mybuf[PFMNUM_MAX_BUF_SIZE + 1];

    PFM_CHECK_NULL(str);

    if (buf_size <= 0) {
        rc = ERROR_WRONG_LENGTH;
        goto PFM_CATCH;
    }
    
    if (buf_size > PFMNUM_MAX_BUF_SIZE) {
        rc = ERROR_TOO_BIG_BUFSIZE;
        goto PFM_CATCH;
    }

    if (pfm_is_int_range(buf_size) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }



    pfm_inconv(&x);
    
#if 0    
    NUM_TRY( number_to_str(&x, (int)buf_size + 1, mybuf, "N"));
#else
    rc = number_to_str(&x, PFMNUM_MAX_BUF_SIZE, mybuf, "N");       
    if (rc != ERROR_NONE) {                                         
        memset(str, '?', buf_size);                                 
        goto PFM_CATCH;                                             
    }  
#endif    

    /* shinhan 표준에 의해, buffer의 앞부분이 '0'으로 padding 되고, 숫자가 우정렬이 되어야함 */
    NUM_TRY( pfm_zrstrconv(str, mybuf, buf_size));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(str);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(buf_size);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
static long allowOutputTruncated = FALSE;

long
pfmNumInitAllowOutputTruncated( void )
{
	allowOutputTruncated = FALSE;
}

long
pfmNumSetAllowOutputTruncated( long flag )
{
	allowOutputTruncated = flag;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToStrn(char *str, pfmnum_t x, long buf_size)
{
    long rc ;

    char mybuf[PFMNUM_MAX_BUF_SIZE + 1];
	long mybuf_size = PFMNUM_MAX_BUF_SIZE;

    PFM_CHECK_NULL(str);

    if (buf_size <= 0) {
        rc = ERROR_WRONG_LENGTH;
        goto PFM_CATCH;
    }

    if (buf_size > PFMNUM_MAX_BUF_SIZE) {
        rc = ERROR_TOO_BIG_BUFSIZE;
        goto PFM_CATCH;
    }

    if (pfm_is_int_range(buf_size) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }
    pfm_inconv(&x);
    
/*    NUM_TRY( number_to_str(&x, (int)buf_size + 1, mybuf, "N"));*/
    
#if 0   /* 20060216 */                                              
    NUM_TRY( number_to_str(&x, (int)buf_size + 1, mybuf, "N"));         
#else                                                               
    //rc = number_to_str(&x, PFMNUM_MAX_BUF_SIZE, mybuf, "N");       
	if ( mybuf_size > buf_size ) mybuf_size = buf_size;

    rc = number_to_str(&x, mybuf_size + 1, mybuf, "N");       
    if (rc != ERROR_NONE)
	{
		/******************************************************************************************
			"버퍼크기가 부족해서 반올림하면 값이 0이 되는 경우" 

			DB실의 스펙은 ERROR_BUF_NOT_ENOUGH 에러가 발생하게 되는데,
			Proframe 의 스펙은 ERROR_NUMBER_OUTPUT_TRUNCATED 가 발생하는 것으로 하기로 함.
		******************************************************************************************/
		if ( rc == ERROR_BUF_NOT_ENOUGH && number_get_exp10( (&x) ) < 0 ) 
		{
			long rc_tmp;
			pfmnum_t num_tmp = x;

			if ( mybuf_size > 2 )
				number_round_at( &num_tmp, -(mybuf_size - 2) );
			else
				number_round_at( &num_tmp, 0 );

			rc_tmp = number_to_str( &num_tmp, mybuf_size + 1, mybuf, "N" );

			/* 아래 IF 문은 항상 성공할 것으로 기대하지만, 예상치못한 버그를 피하기 위해 넣어둠 */
			if ( rc_tmp == ERROR_NONE ) rc = ERROR_NUMBER_OUTPUT_TRUNCATED;
		}
		/******************************************************************************************/

		/* Truncate & RoundUp if allowed ==> Success */
		if ( rc == ERROR_NUMBER_OUTPUT_TRUNCATED && allowOutputTruncated )
		{
			pfm_strconv(str, mybuf, buf_size);
			return RC_NRM;
		}
#if 1
		/*
		   기존 버전은 ERROR_NUMBER_NOT_ENOUGH_BUFFER 일때도, ???가 아닌 부분결과를 리턴한다.
		   호환성 유지를 위해 이 부분을 추가..
		*/
		else if ( rc == ERROR_NUMBER_OUTPUT_TRUNCATED || rc == ERROR_BUF_NOT_ENOUGH )
		{
			long rc_tmp = number_to_str(&x, PFMNUM_MAX_BUF_SIZE, mybuf, "N");       
			if ( rc_tmp != ERROR_NONE )
			{
				memset(str, '?', buf_size);
				goto PFM_CATCH;
			}
			pfm_strconv(str, mybuf, buf_size);
			rc = ERROR_BUF_NOT_ENOUGH;
		}
#endif
		else
		{
			memset(str, '?', buf_size);
		}

		goto PFM_CATCH;                                             
    }                                                               
#endif     
    
    NUM_TRY( pfm_strconv(str, mybuf, buf_size));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(str);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(buf_size);
    return RC_ERR;
}



/* --------------------------------------- function body ---------------------------------------- */
long 
longDoubleToStrn(char *str, long double ld,  long buf_size)
{

    long rc ;
    pfmnum_t tmp;

//    pfmnum_t x;
//   pfm_inconv(&x);
    long precision = 3;

    pfm_inconv(&tmp);

    //rc = pfmNumGetFromLongDouble2( &tmp, ld , precision);
    rc = pfmNumGetFromLongDouble( &tmp, ld );
    if ( rc != RC_NRM ) goto PFM_CATCH;


    pfmNumToStrn(str, tmp, buf_size);
    //printf(" ==== before delete zero %s \n ", str);
    //pfmDeleteZero(str);
    //printf(" ==== after delete zero %s \n ", str);
    pfm_outconv(&tmp);

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(str);
    EMSG_APND_DOUBLE(ld);
    EMSG_APND_LONG(buf_size);
    return RC_ERR;
}


/* --------------------------------------- function body ---------------------------------------- */
void
pfmDeleteZero(char *str)
{
   int ix;
    int len = 0;
    char *ptr=NULL;

    if(strchr( str, '.')){
       len = strlen(str);
       printf("debug str  : %s \n ", str);
       ptr = str+len;
       ptr = ptr--;
       while(ptr > str){
           printf("current  string value  : %s \n ", ptr);
           if(*ptr =='0'){
              *ptr = '\0';
           } else break;
       ptr--;
       printf("debug str : %s \n ", str);
       }
    }
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToCommaStrN(char *str, pfmnum_t x, long buf_size)
{
    long rc ;

    char mybuf[PFMNUM_MAX_BUF_SIZE + 1];

    PFM_CHECK_NULL(str);
    

    if (buf_size <= 0) {
        rc = ERROR_WRONG_LENGTH;
        goto PFM_CATCH;
    }

    if (buf_size > PFMNUM_MAX_BUF_SIZE) {
        rc = ERROR_TOO_BIG_BUFSIZE;
        goto PFM_CATCH;
    }

    if (pfm_is_int_range(buf_size) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }


    pfm_inconv(&x);

    NUM_TRY( number_to_str(&x, (int)buf_size + 1, mybuf, "N"));
    
    NUM_TRY( pfm_cmastrconv(str, mybuf, buf_size));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(str);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(buf_size);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToStr(char *str, pfmnum_t x)
{
    long myrc;

    MY_TRY( pfmNumToStrn(str, x, PFMNUM_MAX_BUF_SIZE));

    return RC_NRM;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(str);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCmp(pfmnum_t x, pfmnum_t y)
{

    pfm_inconv(&x);
    pfm_inconv(&y);

    return number_cmp(&x, &y);
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCmpAbs(pfmnum_t x, pfmnum_t y) 
{

    pfm_inconv(&x);
    pfm_inconv(&y);
#if 0 /* 20080731: MRH. NEW NUMBER */
    return number_cmp_abs(&x, &y);
#else
    return pfm_number_cmp_abs(&x, &y);
#endif
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumIsInteger(pfmnum_t x)
{
    pfm_inconv(&x);

    return number_is_integer(&x);
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCmpLong(pfmnum_t x, long y)
{
    long rc;
    pfmnum_t _y;

    NUM_TRY( pfmNumGetFromLong(&_y, y));

    return pfmNumCmp(x, _y);

  PFM_CATCH:
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCmpStr(pfmnum_t x, char *str)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(str);

    NUM_TRY( pfmNumGetFromStr(&_y, str));

    return pfmNumCmp(x, _y);

  PFM_CATCH:
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCmpStrn(pfmnum_t x, char *str, long len)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(str);

    rc = pfmNumGetFromStrn(&_y, str, len);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumCmp(x, _y);
  PFM_CATCH:
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumNegate(pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    rc = number_negate(&x);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAbs(pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    rc = number_abs(&x);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAdd(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    NUM_TRY( number_add(&x, &y) ); 
    
    *v = x;
    pfm_outconv(v);

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSub(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    NUM_TRY( number_sub(&x, &y) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumMul(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    NUM_TRY( number_mul(&x, &y) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumDiv(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    NUM_TRY( number_div(&x, &y) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumIntDiv(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    rc = number_intdiv(&x, &y); 
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumMod(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);

    NUM_TRY( number_mod(&x, &y) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumModDiv(pfmnum_t *v, pfmnum_t *mod, pfmnum_t x, pfmnum_t y)
{
    long rc;
    pfmnum_t _v;
    pfmnum_t _z;
    
    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(mod);

    if (pfmNumCmp(y, g_pfmnum_zero) == 0) {
        rc = ERROR_NUMBER_DIVIDE_BY_ZERO;
        goto PFM_CATCH; 
    }

    pfm_inconv(&x);
    pfm_inconv(&y);

    /*  
     * 20050530 : ABS가 빠져도 mod값과는 관계없는 것으로 보임 
     */
#if 0   
	if ((rc = number_abs(&y)) != ERROR_NONE)
		goto PFM_CATCH;
#endif

#if 0 /* 20080731: MRH. NEW NUMBER */
	if ((rc = number_assign(&_z, &x)) != ERROR_NONE)
		goto PFM_CATCH;
#else
	number_assign(&_z, &x);
#endif
	if ((rc = number_div(&_z, &y)) != ERROR_NONE)
		goto PFM_CATCH;
	if ((rc = number_trunc(&_z)) != ERROR_NONE)
		goto PFM_CATCH;
    _v = _z;    /* 추가 */
	if ((rc = number_mul(&_z, &y)) != ERROR_NONE)
		goto PFM_CATCH;
	if ((rc = number_sub(&x, &_z)) != ERROR_NONE)
		goto PFM_CATCH;
    
    *v   = _v;
    *mod = x;
    pfm_outconv(v);
    pfm_outconv(mod);

	return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_PTR(mod);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;

#if 0
    pfmnum_t _x;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(y);

    x = *x;
    y = *y;
    pfm_inconv(&x);
    pfm_inconv(&y);

    rc = number_mod(&x, &y); 
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);

    return RC_NRM;
#endif
    /* XXX 딴거하고 나서 */
}
/* --------------------------------------- function body ---------------------------------------- */
#if 0 /* 20050914 제거 pfmNumTruncAt/pfmNumRoundAt로 대체  */
long 
pfmNumberTruncateAt(pfmnum_t *v, pfmnum_t x, long pos)
{
    long rc;

    PFM_CHECK_NULL(v);

    if (pfm_is_int_range(pos) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }
    
    pfm_inconv(&x);

    NUM_TRY( number_trunc_at(&x, (int) pos) );
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(pos);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumberRoundAt(pfmnum_t *v, pfmnum_t x, long pos)
{
    long rc;

    PFM_CHECK_NULL(v);

    if (pfm_is_int_range(pos) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }
    
    pfm_inconv(&x);

    NUM_TRY( number_round_at(&_x, (int) pos) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(pos);
    return RC_ERR;
}
#endif
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumTrunc(pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    rc = number_trunc(&x);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumRound(pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    rc = number_round(&x);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSqr (pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    NUM_TRY( number_sqr(&x));
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSqrt(pfmnum_t *v, pfmnum_t x)
{
    long rc;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    NUM_TRY( number_sqrt(&x));
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
#ifdef NH_API
long 
pfmNumPowerN (pfmnum_t *v, pfmnum_t x, pfmnum_t y)
#else
long 
pfmNumPower (pfmnum_t *v, pfmnum_t x, pfmnum_t y)
#endif
{
    long rc;
    long myrc;
    long l;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);
    
#if 0
    if (number_is_integer(&y)) {
        MY_TRY(pfmNumToLong(&l, y));
        return pfmNumPowLong(v, x, l);
    }
#endif

#if 0 /* IMS.107095 : pfmNumPower - START*/
    NUM_TRY(number_pow(&x, &y)); 
#else
    static long env=-1;
    if(env==-1) {
        if(getenv("NUMBER_UNDERFLOW_SET_ZERO_YN") != NULL ) {
            if(strncmp(getenv("NUMBER_UNDERFLOW_SET_ZERO_YN"), "Y", 1) == 0) {
                env = TRUE;
            } else {
                env = FALSE;
            }
        } else {
            env = FALSE;
        }
    }
    rc = number_pow(&x, &y);
    if(rc == ERROR_NUMBER_UNDERFLOW) {
        if(env == TRUE){
            x=PFMNUM_ZERO;
            rc = RC_NRM;
        } else {
            goto PFM_CATCH;
        }
    }
#endif /* IMS.107095 : pfmNumPower - START*/
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
#ifdef NH_API
long 
pfmNumExpN (pfmnum_t *v, pfmnum_t x )
{
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_one ) == 0 )
	{
		*v = g_pfmnum_e;

		return RC_NRM;
	}

	return pfmNumPowerN( v, g_pfmnum_e, x );
}
#else
long 
pfmNumExp (pfmnum_t *v, pfmnum_t x )
{
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_one ) == 0 )
	{
		*v = g_pfmnum_e;

		return RC_NRM;
	}

	return pfmNumPower( v, g_pfmnum_e, x );
}
#endif

/* --------------------------------------- function body ---------------------------------------- */
#if 0
#define LOG_PRINTF(...) printf( __VA_ARGS__ );
#else
#define LOG_PRINTF(...) 
#endif
long
pfmNumLn(pfmnum_t *v, pfmnum_t x)
{
	long rc = 0;
    long i  = 0;
	pfmnum_t num_minus_1;
	pfmnum_t num_0_5;
	pfmnum_t num_1_5;
	pfmnum_t err;

    pfmnum_t a;
    pfmnum_t b;
    pfmnum_t c;
	pfmnum_t pow_e;
	pfmnum_t tmp;
    pfmnum_t sum;

	double dval;
	long   intpart;
	long   sign;

	struct timeval tm;

	a   = *_number_zero;
	b   = *_number_zero;
	c   = *_number_zero;
	tmp = *_number_zero;
	sum = *_number_zero;
	err = *_number_zero;
	*v  = *_number_zero;

#if 0
gettimeofday( &tm, NULL ); printf("START: %d.%06d\n", tm.tv_sec, tm.tv_usec);
#endif

	/* Set Constants */
	NUM_TRY( number_from_str( &err        , 5, "1e-40", "N" ) );
	NUM_TRY( number_from_str( &num_minus_1, 2, "-1", "N" ) );
	NUM_TRY( number_from_str( &num_0_5    , 3, "0.5", "N" ) );
	NUM_TRY( number_from_str( &num_1_5    , 3, "1.5", "N" ) );

	/* Set vars 'a' */
	a = x;

	/* Calc intpart */
	NUM_TRY( number_to_double( &a, &dval ) );
	intpart = (long) log( dval );
	if ( intpart < 0 ) intpart = -intpart;

	/* Calc e^abs(intpart) */
	NUM_TRY( pfm_repeat_mul( &pow_e, (pfmnum_t *)&g_pfmnum_e, intpart ) );

#if 0
gettimeofday( &tm, NULL ); printf("MAKE : %d.%06d, i=%d\n", tm.tv_sec, tm.tv_usec, i);
#endif

	/* PART1: make 0.5 <= a <= 1.5 , if a < 0.5 */
    if ( number_cmp( &a, &num_0_5 ) < 0)
    {
        LOG_PRINTF("!! a=%s\n", pfmNumPrint(a));
		NUM_TRY( number_mul( &a, &pow_e ) );
        for ( i = intpart ; number_cmp( &a,  &num_0_5) < 0 ; i++)
        {
            NUM_TRY( number_mul(&a, (pfmnum_t *)&g_pfmnum_e) );
            LOG_PRINTF("!! a=%s\n", pfmNumPrint(a));
        }
#ifdef LONG64BIT
		NUM_TRY( number_from_int64( &sum, -i ));
#else
		NUM_TRY( number_from_int32( &sum, -i ));
#endif
    }

#if 0
gettimeofday( &tm, NULL ); printf("PART1: %d.%06d, i=%d\n", tm.tv_sec, tm.tv_usec, i);
#endif

	/* PART1: make 0.5 <= a <= 1.5 , if a > 1.5 */
    if ( number_cmp( &a, &num_1_5 ) > 0 )
    {
        LOG_PRINTF("!! a=%s, pow_e=%s, intpart=%d\n", pfmNumPrint(a), pfmNumPrint(pow_e), intpart);
		NUM_TRY( number_div( &a, &pow_e ) );
        for ( i = intpart ; number_cmp( &a, &num_1_5 ) > 0 ; i++)
        {
            NUM_TRY( number_div(&a, (pfmnum_t *)&g_pfmnum_e) );
            LOG_PRINTF("!! a=%s\n", pfmNumPrint(a));
            //if ( i == 1 ) exit(0);
        }
#ifdef LONG64BIT
		NUM_TRY( number_from_int64( &sum, i ));
#else
		NUM_TRY( number_from_int32( &sum, i ));
#endif
    }
	LOG_PRINTF("a=%s\n", pfmNumPrint(a));

#if 0
gettimeofday( &tm, NULL ); printf("PART2: %d.%06d, i=%d\n", tm.tv_sec, tm.tv_usec, i);
#endif

	/* Calc -(a - 1) */
    NUM_TRY( number_sub( &a, (pfmnum_t *)&g_pfmnum_one ) );
	NUM_TRY( number_mul( &a, &num_minus_1) );

	/* Set var 'b' */
	b = num_minus_1;

	/* Calc Error limit */
	if ( !number_is_zero( &sum ) ) NUM_TRY( number_mul( &err, &sum ) );

    for (i = 1 ; i < 10000; i++)
    {
        LOG_PRINTF("\n===================[%d]=========================\n", i);
        NUM_TRY( number_mul(&b, &a) );
#ifdef LONG64BIT
		NUM_TRY( number_from_int64( &tmp, i ));
#else
		NUM_TRY( number_from_int32( &tmp, i ));
#endif
		c = b;
		NUM_TRY( number_div( &c, &tmp ) );

        char *sum_str = pfmNumPrint(sum);

        NUM_TRY( number_add( &sum, &c) );

        LOG_PRINTF("================>\n");
        LOG_PRINTF("  a=%s\n", pfmNumPrint(a));
        LOG_PRINTF("  b=%s\n", pfmNumPrint(b));
        LOG_PRINTF("sum=%s\n", sum_str);
        LOG_PRINTF("  c=%s\n", pfmNumPrint(c));
        LOG_PRINTF("sum=%s\n", pfmNumPrint(sum));

        if ( pfm_number_cmp_abs( &c, &err ) < 0 ) break;
    }
#if 0
	printf("i=%d\n", i);
#endif

    *v = sum;

#if 0
gettimeofday( &tm, NULL ); printf("END  : %d.%06d\n", tm.tv_sec, tm.tv_usec);
#endif

    return 0;

PFM_CATCH:
    return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
#ifdef NH_API
long 
pfmNumLogEN(pfmnum_t *v, pfmnum_t x)
#else
long 
pfmNumLogE(pfmnum_t *v, pfmnum_t x)
#endif
{
    long rc;
    long myrc;
    long l;
	double d_x;
	double d_v;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    
	if ( !number_is_positive(&x) )
	{
		rc = ERROR_NUMBER_INVALID_ARGUMENT;
		goto PFM_CATCH;
	}

#if 0
	NUM_TRY(number_to_double(&x, &d_x)); 
    
	d_v = log(d_x);

	NUM_TRY(number_from_double(v, d_v)); 
#else
	NUM_TRY( pfmNumLn( v, x ) );
	number_round_at( v, number_get_exp10(v) + 1 - NUMBER_MAX_PREC );
#endif

	pfm_outconv(v);
	return RC_NRM;

  PFM_CATCH:
	pfm_outconv(&x);

    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
#ifdef NH_API
long 
pfmNumLog10N(pfmnum_t *v, pfmnum_t x)
#else
long 
pfmNumLog10(pfmnum_t *v, pfmnum_t x)
#endif
{
    long rc;
    long myrc;
    long l;
	double d_x;
	double d_v;
	static pfmnum_t num_ln10;
	static long     init_ln10 = 0;
	pfmnum_t num_ln;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    
	if ( !number_is_positive(&x) )
	{
		rc = ERROR_NUMBER_INVALID_ARGUMENT;
		goto PFM_CATCH;
	}

#if 0
	NUM_TRY(number_to_double(&x, &d_x)); 
    
	d_v = log10(d_x);

	NUM_TRY(number_from_double(v, d_v)); 
#else
#if 0
	NUM_TRY( number_from_str( &num_ln10, 39, "2.3025850929940456840179914546843642076", "N") );
#else
	if ( init_ln10 == 0 )
	{
		NUM_TRY( pfmNumLn( &num_ln10, g_pfmnum_ten ) );
		init_ln10 = 1;
	}
#endif
	NUM_TRY( pfmNumLn( v, x ) );
	NUM_TRY( number_div( v, &num_ln10 ) );

	number_round_at( v, number_get_exp10(v) + 1 - NUMBER_MAX_PREC );
#endif

	pfm_outconv(v);
	return RC_NRM;

  PFM_CATCH:
	pfm_outconv(&x);

    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
#ifdef NH_API
long 
pfmNumLogN(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
#else
long 
pfmNumLog(pfmnum_t *v, pfmnum_t x, pfmnum_t y)
#endif
{
    long rc;
    long myrc;
    long l;
	double d_x;
	double d_y;
	double d_v;
	pfmnum_t num_lnbase;

#ifdef NH_API
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_e  ) == 0 ) return pfmNumLogEN ( v, y );
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_ten) == 0 ) return pfmNumLog10N( v, y );
#else
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_e  ) == 0 ) return pfmNumLogE ( v, y );
	if ( number_cmp( &x, (pfmnum_t *)&g_pfmnum_ten) == 0 ) return pfmNumLog10( v, y );
#endif

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);
    pfm_inconv(&y);
    
	if ( !number_is_positive(&x) ||
		 number_cmp( &x, (pfmnum_t *)&g_pfmnum_one ) == 0 ||
		 !number_is_positive(&y) )
	{
		rc = ERROR_NUMBER_INVALID_ARGUMENT;
		goto PFM_CATCH;
	}

#if 0
	NUM_TRY(number_to_double(&x, &d_x)); 
	NUM_TRY(number_to_double(&y, &d_y)); 
    
	d_v = log(d_y) / log(d_x);

	NUM_TRY(number_from_double(v, d_v)); 
#else
	NUM_TRY( pfmNumLn( &num_lnbase, x ) );
	NUM_TRY( pfmNumLn( v, y ) );
	NUM_TRY( number_div( v, &num_lnbase ) );
	number_round_at( v, number_get_exp10(v) + 1 - NUMBER_MAX_PREC );
#endif

	pfm_outconv(v);
	return RC_NRM;

  PFM_CATCH:
	pfm_outconv(&x);
	pfm_outconv(&y);

    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumInc(pfmnum_t *x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(x);

    pfm_inconv(x);
    pfm_inconv(&y);

    NUM_TRY( number_add(x, &y)); 
    
    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumDec(pfmnum_t *x, pfmnum_t y)
{
    long rc;

    PFM_CHECK_NULL(x);

    pfm_inconv(x);
    pfm_inconv(&y);

    NUM_TRY( number_sub(x, &y)); 
    
    pfm_outconv(x);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(x);
    EMSG_APND_NUM(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAddStr(pfmnum_t *v, pfmnum_t x, char *str)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStr(&_str, str));

    MY_TRY( pfmNumAdd(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSubStr(pfmnum_t *v, pfmnum_t x, char *str)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStr(&_str, str));

    MY_TRY( pfmNumSub(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumMulStr(pfmnum_t *v, pfmnum_t x, char *str)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStr(&_str, str));

    MY_TRY( pfmNumMul(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumDivString(pfmnum_t *v, pfmnum_t x, char *str)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStr(&_str, str));

    MY_TRY( pfmNumDiv(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STR(str);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAddStrn(pfmnum_t *v, pfmnum_t x, char *str, long len)
{
    long		rc;
    long		myrc;
    pfmnum_t	_str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStrn(&_str, str, len));

    MY_TRY( pfmNumAdd(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSubStrn(pfmnum_t *v, pfmnum_t x, char *str, long len)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

#if 0 
    printf("[%s] pfmNumGetFromStrn call 전[str.....-[%s] len......[%ld]\n", __func__, str, len);
#endif 

    MY_TRY( pfmNumGetFromStrn(&_str, str, len));

#if 0
    printf("[%s] pfmNumSub call 전\n", __func__);
#endif 

    MY_TRY( pfmNumSub(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;

  PFM_CATCH2:
#if 0
    printf("[%s] test pfmNumSubStrn p\n", __func__);
#endif

    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumMulStrn(pfmnum_t *v, pfmnum_t x, char *str, long len)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStrn(&_str, str, len));

    MY_TRY( pfmNumMul(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumDivStrn(pfmnum_t *v, pfmnum_t x, char *str, long len)
{
    long rc;
    long myrc;
    pfmnum_t _str;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(str);

    MY_TRY( pfmNumGetFromStrn(&_str, str, len));

    MY_TRY( pfmNumDiv(v, x, _str));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_STRN(str, len);
    EMSG_APND_LONG(len);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumAddLong(pfmnum_t *v, pfmnum_t x, long y)
{
    long rc;
    long myrc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    MY_TRY( pfmNumGetFromLong(&_y, y));

    MY_TRY( pfmNumAdd(v, x, _y));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
    
  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumSubLong(pfmnum_t *v, pfmnum_t x, long y)
{
    long rc;
    long myrc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    MY_TRY( pfmNumGetFromLong(&_y, y));

    MY_TRY( pfmNumSub(v, x, _y));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumMulLong(pfmnum_t *v, pfmnum_t x, long y)
{
    long rc;
    long myrc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    MY_TRY( pfmNumGetFromLong(&_y, y));

    MY_TRY( pfmNumMul(v, x, _y));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumDivLong(pfmnum_t *v, pfmnum_t x, long y)
{
    long rc;
    long myrc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    MY_TRY( pfmNumGetFromLong(&_y, y));

    MY_TRY( pfmNumDiv(v, x, _y));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumModDivLong(pfmnum_t *v, pfmnum_t *mod, pfmnum_t x, long y)
{
    long rc;
    long myrc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);
    PFM_CHECK_NULL(mod);

    MY_TRY( pfmNumGetFromLong(&_y, y));

    MY_TRY( pfmNumModDiv(v, mod, x, _y));

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_PTR(mod);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;

  PFM_CATCH2:
    EMSG_INIT2();
    EMSG_APND_PTR(v);
    EMSG_APND_PTR(mod);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
char * pfmNumGetErrorMsg(void)
{
    return g_pfmnum_emsg;
}


/* ==============================================================================================
 *                                           static 함수
 * ============================================================================================== */
static void 
pfm_inconv(pfmnum_t *x)
{
    if ( memcmp(x, &g_pfmnum_zero, sizeof(pfmnum_t)) == 0) {
#if 0
        number_set_zero(x);
#else
		number_assign((x), _number_zero);
#endif
    }
}
/* --------------------------------------- function body ---------------------------------------- */
static void 
pfm_outconv(pfmnum_t *x)
{
    if (number_is_zero(x) == TRUE) {
        *x = g_pfmnum_zero;
    }
}
/* --------------------------------------- function body ---------------------------------------- */
static long
pfm_is_int_range(long in)
{
    return (INT_MIN <= in && in <= INT_MAX);
}
/* --------------------------------------- function body ---------------------------------------- */
void  
pfmNumDump(char *buf, pfmnum_t *x)
{
	long i;
    unsigned char *ptr;

    ptr = (unsigned char *) x;
    sprintf(buf, "%02x", *ptr);
    for (i = 1; i < (long)sizeof(pfmnum_t); i++) {
        sprintf(buf + 2 + ((i - 1) * 3), ",%02x", *(ptr + i));
    }
}
/* --------------------------------------- function body ---------------------------------------- */
static void
pfmnum_emsg_apnd(char *buf, long len)
{
    long curr_len;

    curr_len = strlen(g_pfmnum_emsg);
    if (curr_len + len > PFMNUM_EMSG_BUF_SIZE) {
        return;
    }
    strncpy(g_pfmnum_emsg + curr_len, buf, len);
}
/* --------------------------------------- function body ---------------------------------------- */
#if 0
static int
pfm_zrstrconv(char *dest, char *src, long buf_size)
{
    long size;

#if 0
    char *dbg_dest = dest;
    printf("strconv : src[%s]\n", src);
#endif

    /* 초기 버퍼 크기 체크 */
    if ((long)strlen(src) > buf_size) {
#if 0
        printf("strlen(src) [%ld] bufsize[%ld]\n", strlen(src), buf_size);
#endif
        return ERROR_BUF_NOT_ENOUGH;
    }
    
    /* 부호는 맨 앞으로 */
    if (src[0] == '-') {
        dest[0] = '-';
        src ++;
        dest ++;
        buf_size --;
    }
    
    /* . 이 앞에 있으면 0을 앞에 붙여줌 */
    if (src[0] == '.') {
        dest[0] = '0';
        dest ++;
        buf_size --;
    }

    size = strlen(src);

    /* ".123", "-.01" 과 같은 경우 .앞에 0을 붙여줘야 하므로 buf가 한 개더 필요
     * 따라서 여기서 한 번더 체크해 줄 필요가 있음 
     */
    if (size > buf_size) {
        return ERROR_BUF_NOT_ENOUGH;
    }

#if 0
    strcpy(dest, src);
#endif
    memset(dest, '0', buf_size - size);
    memcpy(dest + (buf_size - size), src, size);
#if 0
    printf("strconv : src[%s] dest[%s]\n", src, dest);
#endif

    return ERROR_NONE;
}
#else
static int
pfm_zrstrconv(char *dest, char *src, long buf_size)
{
    long s = 0;
    long d = 0;
    long size; 

    
    if (src[s] == '-') {
        dest[d] = '-';
        s++;
        d++;
    }

    if (src[s] == '.') {
        if (d > buf_size - 1) {
            strcpy(dest, "?");
            return ERROR_BUF_NOT_ENOUGH;
        }
        dest[d] = '0';
        d++;
    }
    
    size = strlen(src+s);
    
    strncpy(dest + d, src + s, (size > buf_size ? buf_size: size) - d);

    /* 구버젼과 동일하게 하기 위해서 */
    if (dest[buf_size - 1] != 0x00) {
        dest[buf_size] = 0x00;
    }

    size = pfm_min_strlen(src + s, buf_size + 2);

    if (size > buf_size - d) {
        return ERROR_BUF_NOT_ENOUGH;
    }

   
    memset(dest+d, '0', buf_size - size-d);
    memcpy(dest + (buf_size - size), src+s, size);    
    
    return ERROR_NONE;
}
#endif


/* --------------------------------------- function body ---------------------------------------- */
#if 0   /* 20060216 */
static int
pfm_strconv(char *dest, char *src, long buf_size)
{
    long size;

#if 0
    char *dbg_dest = dest;
    printf("strconv : src[%s]\n", src);
#endif
    if (strlen(src) > buf_size) {
#if 0
        printf("strlen(src) [%ld] bufsize[%ld]\n", strlen(src), buf_size);
#endif
        return ERROR_BUF_NOT_ENOUGH;
    }
    
    if (src[0] == '-') {
        dest[0] = '-';
        src ++;
        dest ++;
        buf_size --;
    }

    if (src[0] == '.') {
        dest[0] = '0';
        dest ++;
        buf_size --;
    }
    
#if 0
    printf("strconv : dbg      [%s]\n", dbg_dest);
    printf("strconv : dest     [%s]\n", dest);
    printf("strconv : src      [%s]\n", src);
    printf("strconv : buf_size [%s]\n", src);
#endif

    size = strlen(src);

    /* ".123", "-.01" 과 같은 경우 .앞에 0을 붙여줘야 하므로 buf가 한 개더 필요
     * 따라서 여기서 한 번더 체크해 줄 필요가 있음 
     */
    if (size > buf_size) {
        return ERROR_BUF_NOT_ENOUGH;
    }
    
    
    strcpy(dest, src);
#if 0
    printf("strconv : src[%s] dest[%s]\n", src, dest);
#endif

    return ERROR_NONE;
}
#else
static int
pfm_strconv(char *dest, char *src, long buf_size)
{
    long s = 0;
    long d = 0;
    long size; 

    
    if (src[s] == '-') {
        dest[d] = '-';
        s++;
        d++;
    }

    if (src[s] == '.') {
        if (d > buf_size - 1) {
            strcpy(dest, "?");
            return ERROR_BUF_NOT_ENOUGH;
        }
        dest[d] = '0';
        d++;
    }
    
#if 0    /*20060217*/

    size = strlen(src+s);
    
    strncpy(dest + d, src + s, (size > buf_size ? buf_size: size) - d);

    /* 구버젼과 동일하게 하기 위해서 */
    if (dest[buf_size - 1] != 0x00) {
        dest[buf_size] = 0x00;
    }

    size = pfm_min_strlen(src + s, buf_size + 2);

    if (size > buf_size - d) {
        return ERROR_BUF_NOT_ENOUGH;
    }
    
#else

    size = pfm_min_strlen(src + s, buf_size + 2);
                                                 
    if (size > buf_size - d) {                   
        strncpy(dest + d, src + s, buf_size - d);
        dest[buf_size] = 0x00;                   
        return ERROR_BUF_NOT_ENOUGH;   
    }                                            
                                                 
    strcpy(dest + d, src + s);                   
    
#endif
    
    return ERROR_NONE;
}
#endif
/* --------------------------------------- function body ---------------------------------------- */
static int
pfm_cmastrconv(char *dest, char *src, long buf_size)
{
    long is_neg = FALSE;
    long src_size = 0;
    long dot_pos ;
    long comma_cnt = 0;

    long src_pos = 0;
    long dest_pos = 0;

    src_size = strlen(src);
    if (src_size > buf_size) {
        return ERROR_BUF_NOT_ENOUGH;
    }

    if (src[0] == '-') {
        is_neg = TRUE;
    }
    
    /* 절대값이 0보다 작은 수일때 처리 */
    if (src[0] == '.') {
        if (src_size + 1 > buf_size) {
            return ERROR_BUF_NOT_ENOUGH;
        }
        else {
            dest[0] = '0';
            memcpy(dest + 1, src, src_size);
            return ERROR_NONE;
        }
    }
    else if (is_neg && src[1] == '.') {
        if (src_size + 1 > buf_size) {
            return ERROR_BUF_NOT_ENOUGH;
        }
        else {
            dest[0] = '-';
            dest[1] = '0';
            memcpy(dest + 2, src + 1, src_size - 1);
            return ERROR_NONE;
        }
    }

    /* 이제는 절대값이 0 이상의 수 */

    /* 소수점 위치를 얻는다 */
    for (dot_pos = 0; dot_pos < src_size; dot_pos ++) {
        if (src[dot_pos] == '.') {
            break;
        }
    }
    
    /* comma 개수 계산 */
    if (is_neg) {
        comma_cnt = (dot_pos - 2) / 3;
    }
    else {
        comma_cnt = (dot_pos - 1) / 3;
    }

    /* 충분한 buffer 가 있는지 체크 */
    if (src_size + comma_cnt > buf_size) {
        return ERROR_BUF_NOT_ENOUGH;
    }
    
    if (is_neg) {
        dest[dest_pos ++] = '-';
        src_pos ++;
    }

    while (src_pos < dot_pos) {
        dest[dest_pos ++] = src[src_pos++];
        if (comma_cnt > 0 && ((dot_pos - src_pos) % 3 == 0)) {
            dest[dest_pos++] = ',';
            comma_cnt --;
        }
    }

    memcpy(dest + dest_pos, src + src_pos, src_size - dot_pos);
    return ERROR_NONE;
}
/* --------------------------------------- function body ---------------------------------------- */
static void 
pfmnum_emsg_init2(void)
{
    char *ptr;
    long len;

    ptr = strchr(g_pfmnum_emsg, ':');
    if (ptr == NULL) {
        return;
    }

    ptr = strchr(ptr + 1, ':');
    if (ptr == NULL) {
        return;
    }
    len = ptr - g_pfmnum_emsg;
    bzero(g_pfmnum_emsg + len, PFMNUM_EMSG_BUF_SIZE - len);
}
static void 
pfmnum_emsg_init(long rc)
{
    bzero(g_pfmnum_emsg, sizeof(g_pfmnum_emsg));

    switch( rc ) {
        /* PFMNUM에서 정의한 에러 START */
    case ERROR_TOO_BIG_FRMT:
        PFM_MSGINIT(ERROR_TOO_BIG_FRMT);
        break;
    case ERROR_INVALID_CONV_STR:
        PFM_MSGINIT(ERROR_INVALID_CONV_STR);
        break;
    case ERROR_INVALID_CONV_STR2:
        PFM_MSGINIT(ERROR_INVALID_CONV_STR2);
        break;
    case ERROR_INVALID_FRMT:
        PFM_MSGINIT(ERROR_INVALID_FRMT);
        break;
    case ERROR_NO_FRMT:
        PFM_MSGINIT(ERROR_NO_FRMT);
        break;
    case ERROR_WRONG_LENGTH:
        PFM_MSGINIT(ERROR_WRONG_LENGTH);
        break;
    case ERROR_ARGUMENT_NULL:
        PFM_MSGINIT(ERROR_ARGUMENT_NULL);
        break;
    case ERROR_TOO_BIG_BUFSIZE:
        PFM_MSGINIT(ERROR_TOO_BIG_BUFSIZE);
        break;
    case ERROR_TOO_FEW_ARGUMENT:
        PFM_MSGINIT(ERROR_TOO_FEW_ARGUMENT);
        break;
    case ERROR_TOO_BIG_LONG_VALUE:
        PFM_MSGINIT(ERROR_TOO_BIG_LONG_VALUE);
        break;
        /* PFMNUM에서 정의한 에러 END */

        /* common_err_code.h에서 정의한 에러 START */
	case ERROR_OUT_OF_SHM                      : PFM_MSGINIT(ERROR_OUT_OF_SHM                      ); break;
	case ERROR_OUT_OF_SHP                      : PFM_MSGINIT(ERROR_OUT_OF_SHP                      ); break;
	case ERROR_OUT_OF_PHYSICAL_MEM             : PFM_MSGINIT(ERROR_OUT_OF_PHYSICAL_MEM             ); break;
	case ERROR_LOG_CANNOT_OPEN_FILE            : PFM_MSGINIT(ERROR_LOG_CANNOT_OPEN_FILE            ); break;
	case ERROR_BUF_NOT_ENOUGH                  : PFM_MSGINIT(ERROR_BUF_NOT_ENOUGH                  ); break;
	case ERROR_NOT_IMPLEMENTED                 : PFM_MSGINIT(ERROR_NOT_IMPLEMENTED                 ); break;
	case ERROR_ENV_NOT_SET                     : PFM_MSGINIT(ERROR_ENV_NOT_SET                     ); break;
	case ERROR_OUT_OF_FIXED_MEM                : PFM_MSGINIT(ERROR_OUT_OF_FIXED_MEM                ); break;
        /* common_err_code.h에서 정의한 에러 END   */
	
        /* datatype_err_code.h에서 정의한 에러 START */
	case ERROR_DT_ITV_OUT_OF_RANGE_PREC        : PFM_MSGINIT(ERROR_DT_ITV_OUT_OF_RANGE_PREC        ); break;
	case ERROR_DT_INVALID                      : PFM_MSGINIT(ERROR_DT_INVALID                      ); break;
	case ERROR_DT_INVALID_TIME                 : PFM_MSGINIT(ERROR_DT_INVALID_TIME                 ); break;
	case ERROR_DT_INVALID_TIMESTAMP            : PFM_MSGINIT(ERROR_DT_INVALID_TIMESTAMP            ); break;
	case ERROR_DT_INVALID_DATE                 : PFM_MSGINIT(ERROR_DT_INVALID_DATE                 ); break;
	case ERROR_DT_INVALID_UNIT                 : PFM_MSGINIT(ERROR_DT_INVALID_UNIT                 ); break;
	case ERROR_DT_INVALID_STR_LEN              : PFM_MSGINIT(ERROR_DT_INVALID_STR_LEN              ); break;
	case ERROR_DT_INVALID_FSEC_PREC_SPECIFIED  : PFM_MSGINIT(ERROR_DT_INVALID_FSEC_PREC_SPECIFIED  ); break;
	case ERROR_DT_INPUT_STR_ENDS_BEFORE_CONVERT: PFM_MSGINIT(ERROR_DT_INPUT_STR_ENDS_BEFORE_CONVERT); break;
	case ERROR_DT_FMT_ENDS_BEFORE_CONVERT      : PFM_MSGINIT(ERROR_DT_FMT_ENDS_BEFORE_CONVERT      ); break;
	case ERROR_DT_NOT_DATE_TOKEN               : PFM_MSGINIT(ERROR_DT_NOT_DATE_TOKEN               ); break;
	case ERROR_DT_NOT_TIME_TOKEN               : PFM_MSGINIT(ERROR_DT_NOT_TIME_TOKEN               ); break;
	case ERROR_DT_NOT_INPUT_TOKEN              : PFM_MSGINIT(ERROR_DT_NOT_INPUT_TOKEN              ); break;
	case ERROR_DT_INVALID_FORMAT_TOKEN         : PFM_MSGINIT(ERROR_DT_INVALID_FORMAT_TOKEN         ); break;
	case ERROR_DT_YEAR_ALREADY_UPDATED         : PFM_MSGINIT(ERROR_DT_YEAR_ALREADY_UPDATED         ); break;
	case ERROR_DT_MONTH_ALREADY_UPDATED        : PFM_MSGINIT(ERROR_DT_MONTH_ALREADY_UPDATED        ); break;
	case ERROR_DT_DAY_ALREADY_UPDATED          : PFM_MSGINIT(ERROR_DT_DAY_ALREADY_UPDATED          ); break;
	case ERROR_DT_HOUR_ALREADY_UPDATED         : PFM_MSGINIT(ERROR_DT_HOUR_ALREADY_UPDATED         ); break;
	case ERROR_DT_MINUTE_ALREADY_UPDATED       : PFM_MSGINIT(ERROR_DT_MINUTE_ALREADY_UPDATED       ); break;
	case ERROR_DT_SECOND_ALREADY_UPDATED       : PFM_MSGINIT(ERROR_DT_SECOND_ALREADY_UPDATED       ); break;
	case ERROR_DT_FSECOND_ALREADY_UPDATED      : PFM_MSGINIT(ERROR_DT_FSECOND_ALREADY_UPDATED      ); break;
	case ERROR_DT_AD_BC_ALREADY_UPDATED        : PFM_MSGINIT(ERROR_DT_AD_BC_ALREADY_UPDATED        ); break;
	case ERROR_DT_AM_PM_ALREADY_UPDATED        : PFM_MSGINIT(ERROR_DT_AM_PM_ALREADY_UPDATED        ); break;
	case ERROR_DT_MONTH_CONFLICT_WITH_DDD      : PFM_MSGINIT(ERROR_DT_MONTH_CONFLICT_WITH_DDD      ); break;
	case ERROR_DT_DAY_CONFLICT_WITH_DDD        : PFM_MSGINIT(ERROR_DT_DAY_CONFLICT_WITH_DDD        ); break;
	case ERROR_DT_HOUR_CONFLICT_WITH_SSSSS     : PFM_MSGINIT(ERROR_DT_HOUR_CONFLICT_WITH_SSSSS     ); break;
	case ERROR_DT_MIN_CONFLICT_WITH_SSSSS      : PFM_MSGINIT(ERROR_DT_MIN_CONFLICT_WITH_SSSSS      ); break;
	case ERROR_DT_SEC_CONFLICT_WITH_SSSSS      : PFM_MSGINIT(ERROR_DT_SEC_CONFLICT_WITH_SSSSS      ); break;
	case ERROR_DT_INVALID_YEAR_INPUT           : PFM_MSGINIT(ERROR_DT_INVALID_YEAR_INPUT           ); break;
	case ERROR_DT_INVALID_MONTH_INPUT          : PFM_MSGINIT(ERROR_DT_INVALID_MONTH_INPUT          ); break;
	case ERROR_DT_INVALID_MONTHNAME_INPUT      : PFM_MSGINIT(ERROR_DT_INVALID_MONTHNAME_INPUT      ); break;
	case ERROR_DT_INVALID_DAY_INPUT            : PFM_MSGINIT(ERROR_DT_INVALID_DAY_INPUT            ); break;
	case ERROR_DT_INVALID_HOUR_INPUT           : PFM_MSGINIT(ERROR_DT_INVALID_HOUR_INPUT           ); break;
	case ERROR_DT_INVALID_MINUTE_INPUT         : PFM_MSGINIT(ERROR_DT_INVALID_MINUTE_INPUT         ); break;
	case ERROR_DT_INVALID_SECOND_INPUT         : PFM_MSGINIT(ERROR_DT_INVALID_SECOND_INPUT         ); break;
	case ERROR_DT_INVALID_FSECOND_INPUT        : PFM_MSGINIT(ERROR_DT_INVALID_FSECOND_INPUT        ); break;
	case ERROR_DT_INVALID_AD_BC_INPUT          : PFM_MSGINIT(ERROR_DT_INVALID_AD_BC_INPUT          ); break;
	case ERROR_DT_INVALID_AM_PM_INPUT          : PFM_MSGINIT(ERROR_DT_INVALID_AM_PM_INPUT          ); break;
	case ERROR_DT_SYYY_WITH_AD_BC              : PFM_MSGINIT(ERROR_DT_SYYY_WITH_AD_BC              ); break;
	case ERROR_DT_HH24_WITH_AM_PM              : PFM_MSGINIT(ERROR_DT_HH24_WITH_AM_PM              ); break;
	case ERROR_DT_INVALID_FRACTION_FMT         : PFM_MSGINIT(ERROR_DT_INVALID_FRACTION_FMT         ); break;
	case ERROR_DT_INVALID_DIGIT_INPUT          : PFM_MSGINIT(ERROR_DT_INVALID_DIGIT_INPUT          ); break;
	case ERROR_DT_INVALID_SEC_OF_DAY_INPUT     : PFM_MSGINIT(ERROR_DT_INVALID_SEC_OF_DAY_INPUT     ); break;
	case ERROR_DT_NOT_ENOUGH_BUFFER            : PFM_MSGINIT(ERROR_DT_NOT_ENOUGH_BUFFER            ); break;
	case ERROR_DT_ONLY_TIME_FMT_OMMITTED       : PFM_MSGINIT(ERROR_DT_ONLY_TIME_FMT_OMMITTED       ); break;
	case ERROR_DT_INVALID_DAY_OF_YEAR_INPUT    : PFM_MSGINIT(ERROR_DT_INVALID_DAY_OF_YEAR_INPUT    ); break;
	case ERROR_DT_INVALID_DAY_NAME             : PFM_MSGINIT(ERROR_DT_INVALID_DAY_NAME             ); break;
	case ERROR_DT_TOO_MANY_TRUNC_FMT           : PFM_MSGINIT(ERROR_DT_TOO_MANY_TRUNC_FMT           ); break;
	case ERROR_DT_TOO_MANY_ROUND_FMT           : PFM_MSGINIT(ERROR_DT_TOO_MANY_ROUND_FMT           ); break;
	case ERROR_INVALID_USE_OF_ESCCHAR          : PFM_MSGINIT(ERROR_INVALID_USE_OF_ESCCHAR          ); break;
	case ERROR_INVALID_HEX_NUMBER              : PFM_MSGINIT(ERROR_INVALID_HEX_NUMBER              ); break;
	case ERROR_INVALID_STR_SEQ                 : PFM_MSGINIT(ERROR_INVALID_STR_SEQ                 ); break;
	case ERROR_INVALID_UNICODE                 : PFM_MSGINIT(ERROR_INVALID_UNICODE                 ); break;
	case ERROR_DT_NOT_IMPLEMENTED_YET          : PFM_MSGINIT(ERROR_DT_NOT_IMPLEMENTED_YET          ); break;
	case ERROR_ITV_INVALID_UNIT                : PFM_MSGINIT(ERROR_ITV_INVALID_UNIT                ); break;
	case ERROR_ITV_INVALID_INPUT_STR           : PFM_MSGINIT(ERROR_ITV_INVALID_INPUT_STR           ); break;
	case ERROR_ITV_INVALID_PREC_INPUT          : PFM_MSGINIT(ERROR_ITV_INVALID_PREC_INPUT          ); break;
	case ERROR_ITV_INVALID_FPREC_INPUT         : PFM_MSGINIT(ERROR_ITV_INVALID_FPREC_INPUT         ); break;
	case ERROR_ITV_INVALID_FMT                 : PFM_MSGINIT(ERROR_ITV_INVALID_FMT                 ); break;
	case ERROR_ITV_INVALID_MONTH_RANGE         : PFM_MSGINIT(ERROR_ITV_INVALID_MONTH_RANGE         ); break;
	case ERROR_ITV_INVALID_HOUR_RANGE          : PFM_MSGINIT(ERROR_ITV_INVALID_HOUR_RANGE          ); break;
	case ERROR_ITV_INVALID_MINUTE_RANGE        : PFM_MSGINIT(ERROR_ITV_INVALID_MINUTE_RANGE        ); break;
	case ERROR_ITV_INVALID_SECOND_RANGE        : PFM_MSGINIT(ERROR_ITV_INVALID_SECOND_RANGE        ); break;
	case ERROR_ITV_EXCEEDS_PRECISION           : PFM_MSGINIT(ERROR_ITV_EXCEEDS_PRECISION           ); break;
	case ERROR_ITV_DIVIDE_BY_ZERO              : PFM_MSGINIT(ERROR_ITV_DIVIDE_BY_ZERO              ); break;
	case ERROR_ITV_NOT_ENOUGH_BUFFER           : PFM_MSGINIT(ERROR_ITV_NOT_ENOUGH_BUFFER           ); break;
	case ERROR_NUMBER_OUTPUT_TRUNCATED         : PFM_MSGINIT(ERROR_NUMBER_OUTPUT_TRUNCATED         ); break;
	case ERROR_NUMBER_OVERFLOW                 : PFM_MSGINIT(ERROR_NUMBER_OVERFLOW                 ); break;
	case ERROR_NUMBER_UNDERFLOW                : PFM_MSGINIT(ERROR_NUMBER_UNDERFLOW                ); break;
	case ERROR_NUMBER_DIVIDE_BY_ZERO           : PFM_MSGINIT(ERROR_NUMBER_DIVIDE_BY_ZERO           ); break;
	case ERROR_NUMBER_EXCEEDS_PRECISION        : PFM_MSGINIT(ERROR_NUMBER_EXCEEDS_PRECISION        ); break;
	case ERROR_NUMBER_CONVERSION_FAILED        : PFM_MSGINIT(ERROR_NUMBER_CONVERSION_FAILED        ); break;
	case ERROR_NUMBER_INVALID_ARGUMENT         : PFM_MSGINIT(ERROR_NUMBER_INVALID_ARGUMENT         ); break;
	case ERROR_NUMBER_INVALID_NUMBER           : PFM_MSGINIT(ERROR_NUMBER_INVALID_NUMBER           ); break;
	case ERROR_NUMBER_INVALID_FORMAT           : PFM_MSGINIT(ERROR_NUMBER_INVALID_FORMAT           ); break;
	case ERROR_NUMBER_UNFIT_TO_FORMAT          : PFM_MSGINIT(ERROR_NUMBER_UNFIT_TO_FORMAT          ); break;
	case ERROR_NUMBER_INVALID_PREC             : PFM_MSGINIT(ERROR_NUMBER_INVALID_PREC             ); break;
	case ERROR_NUMBER_INVALID_SCALE            : PFM_MSGINIT(ERROR_NUMBER_INVALID_SCALE            ); break;
	case ERROR_DT_INVALID_LENGTH               : PFM_MSGINIT(ERROR_DT_INVALID_LENGTH               ); break;
	case ERROR_ROWID_WRONG                     : PFM_MSGINIT(ERROR_ROWID_WRONG                     ); break;
	case ERROR_ROWID_WRONG_SGMT                : PFM_MSGINIT(ERROR_ROWID_WRONG_SGMT                ); break;
	case ERROR_ROWID_WRONG_FILE                : PFM_MSGINIT(ERROR_ROWID_WRONG_FILE                ); break;
	case ERROR_ROWID_WRONG_BLOCK               : PFM_MSGINIT(ERROR_ROWID_WRONG_BLOCK               ); break;
	case ERROR_ROWID_WRONG_ROW                 : PFM_MSGINIT(ERROR_ROWID_WRONG_ROW                 ); break;
	case ERROR_INVALID_IP_STRING               : PFM_MSGINIT(ERROR_INVALID_IP_STRING               ); break;
        /* datatype_err_code.h에서 정의한 에러 END   */
    default:
        sprintf(g_pfmnum_emsg, "%s", "UNKNOWN ERRCODE");
    }
}

/* ============================================================================================== */
/* ---------------------------------------------------------------------------------------------- */
/*                                                                                                */
/*                  PROTOTYPE 지원을 위한 임시 함수 : 추후 기능 삭제 예정                         */
/*                                                                                                */
/* ---------------------------------------------------------------------------------------------- */
/* ============================================================================================== */
/*
  long
  xxxxxxxxxxxxxxxxxxxxxxxxxxxx
*/

#if 0
/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumAddN(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long rc = RC_NRM;

    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumAdd);
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumMulN(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumMul);
}

#else

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumAddNVaList(long cnt, pfmnum_t *v, pfmnum_t x, va_list *parg)
{
    long rc = RC_NRM;

    PFMNUM_VA_OP_NUM_V2(cnt, v, x, *parg, pfmNumAdd);
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumMulNVaList(long cnt, pfmnum_t *v, pfmnum_t x, va_list *parg)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM_V2(cnt, v, x, *parg, pfmNumMul);
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumAddN(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
	long rc = RC_NRM;
	va_list ap;

	va_start(ap, x);
	rc = pfmNumAddNVaList(cnt, v, x, &ap);
	va_end(ap);
	return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumMulN(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
	long            rc    = RC_NRM;
	va_list ap;

	va_start(ap, x);
	rc = pfmNumMulNVaList(cnt, v, x, &ap);
	va_end(ap);
	return rc;
}


#endif

/* --------------------------------------- function body ---------------------------------------- */
/* --------------------------------------- function body ---------------------------------------- */
/* --------------------------------------- function body ---------------------------------------- */

/* --------------------------------------- function body ---------------------------------------- */

/* 고객의 추가 요구사항이 없는 이상은  simple하게 하기 위해서 floor와 ceil은 제공하지 않음 */
#if 0
long
pfmnum_subn(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumSub);
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmnum_divn(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumDiv);
}
long
pfmnum_intdivn(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumIntDiv);
}
long
pfmnum_modn(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP_NUM(cnt, v, x, pfmNumMod);
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_from_int(pfmnum_t *x, int y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    rc = number_from_int(x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
    return RC_NRM;
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_to_int(pfmnum_t *x, int *y)
{
    long rc ;
    pfmnum_t _x;

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(y);

    _x = *x;

    pfm_inconv(&_x);

    rc = number_to_int(&_x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    return RC_NRM;
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
long 
pfmnum_from_dbl(pfmnum_t *x, double y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    rc = number_from_double(x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
    return RC_NRM;
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}

long 
pfmnum_to_dbl(pfmnum_t *x, double *y)
{
    long rc ;
    pfmnum_t _x;

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(y);

    _x = *x;

    pfm_inconv(&_x);

    rc = number_to_double(&_x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
long 
pfmnum_add_int(pfmnum_t *v, pfmnum_t x, int y)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    rc = pfmnum_from_int(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumAdd(v, x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long 
pfmnum_sub_int(pfmnum_t *v, pfmnum_t x, int y)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    rc = pfmnum_from_int(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumSub(v, x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long 
pfmnum_cmp_int(pfmnum_t x, int y)
{
    long rc;
    pfmnum_t _y;

    rc = pfmnum_from_int(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumCmp(x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long 
pfmnum_add_dbl(pfmnum_t *v, pfmnum_t x, double y)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);

    rc = pfmnum_from_dbl(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumAdd(v, x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long 
pfmnum_sub_dbl(pfmnum_t *v, pfmnum_t x, double y)
{
    long rc;
    pfmnum_t _y;

    PFM_CHECK_NULL(v);


    rc = pfmnum_from_dbl(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumSub(v, x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long 
pfmnum_cmp_dbl(pfmnum_t x, double y)
{
    long rc;
    pfmnum_t _y;

    rc = pfmnum_from_dbl(&_y, y);
    if (rc != RC_NRM) {
        return RC_ERR;
    }

    return pfmNumCmp(x, _y);
  PFM_CATCH:
    return RC_ERR;
}
long
pfmnum_addn_int(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP2(cnt, v, x, pfmnum_add_int, int);
}
long
pfmnum_subn_int(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP2(cnt, v, x, pfmnum_sub_int, int);
}
long
pfmnum_addn_dbl(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP2(cnt, v, x, pfmnum_add_dbl, double);
}
long
pfmnum_subn_dbl(long cnt, pfmnum_t *v, pfmnum_t x, ...)
{
    long            rc    = RC_NRM;
    PFMNUM_VA_OP2(cnt, v, x, pfmnum_sub_dbl, double);
}
long 
pfmnum_floor(pfmnum_t *x)
{
    long rc;

    pfm_inconv(x);

    rc = number_floor(x); 
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
    return RC_NRM;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_ceil(pfmnum_t *x)
{
    long rc;

    pfm_inconv(x);

    rc = number_ceil(x); 
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
    return RC_NRM;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_is_positive(pfmnum_t *x)
{
/*    pfmnum_t _x;*/

    PFM_CHECK_NULL(x);

/*    _x = *x;*/

    pfm_inconv(x);

    return number_is_positive(x);
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_is_negative(pfmnum_t *x)
{
/*    pfmnum_t _x;*/

    PFM_CHECK_NULL(x);

    /*   _x = *x;*/

    pfm_inconv(x);

    return number_is_negative(x);
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_sqrt(pfmnum_t *x)
{
    long rc;

    PFM_CHECK_NULL(x);

    pfm_inconv(x);

    rc =  number_sqrt(x);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
    return RC_NRM;
}
/* --------------------------------------- function body ---------------------------------------- */

/* ==============================================================================================
 *                    original number 함수를 직접 사용하지 않고 구현된 함수들
 * ============================================================================================== */

/* --------------------------------------- function body ---------------------------------------- */
long pfmnum_set_zero(pfmnum_t *x)
{
    /* pfmnum_t 에서는 all null을  Zero의 표준으로 사용 */
    PFM_CHECK_NULL(x);

    bzero(x, sizeof(pfmnum_t));

    return RC_NRM;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_is_zero(pfmnum_t *x)
{
    pfmnum_t zero;
    
    if (x == NULL) return FALSE;

    pfmnum_set_zero(&zero);

    if (memcmp(x, &zero, sizeof(pfmnum_t)) == 0) {
        return TRUE;
    }

    return FALSE;
}
/* DBIO 용 */
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_to_int32(pfmnum_t *x, int *y)
{
    long rc ;
/*    pfmnum_t _x;*/

    PFM_CHECK_NULL(x);
    PFM_CHECK_NULL(y);

    /*   _x = *x;*/

    pfm_inconv(x);

    rc = number_to_int32(x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    return RC_NRM;
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmnum_from_int32(pfmnum_t *x, int y)
{
    long rc ;

    PFM_CHECK_NULL(x);

    rc = number_from_int32(x, y);
    if (rc != ERROR_NONE) {
        return RC_ERR;
    }

    pfm_outconv(x);
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmnum_println(pfmnum_t *x)
{
    long rc;
    char buf[512];

    bzero (buf, sizeof(buf) );
#if 0
    rc = pfmNumToStrn (x, buf, 512);
#else
    rc = pfmNumToStrn (x, buf, 512);
#endif
    printf("[%s]\n",buf);
    return RC_ERR;
  PFM_CATCH:
    EMSG_INIT();
    return RC_ERR;
}
#endif


/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* 다음은 add 되는 함수들 입니다 20050907 */
/* -------------------------------------- global variables -------------------------------------- */
int      g_pfmutil_int_stack[STACK_MAX];
pfmnum_t g_pfmnum_stack[STACK_MAX];
int g_pfmutil_int_top;
int g_pfmnum_top;
/* ------------------------------------ function prototypes ------------------------------------- */
static char * pfmnum_print(pfmnum_t *x, long type);

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumPowLong(pfmnum_t *v, pfmnum_t x, long y)
{
    long rc;
    pfmnum_t _z;

	if ( memcmp(&x, &g_pfmnum_zero, sizeof(pfmnum_t)) == 0) {
		*v = PFMNUM_ZERO;
	}
	else
	{
		if (y == 0) {
			*v = PFMNUM_ONE;
		}
		else if (y > 0) {
			NUM_TRY(pfm_repeat_mul(v, &x, y));
		}
		else {
			NUM_TRY(pfm_repeat_mul(&_z, &x, -y));
			*v = PFMNUM_ONE;
			NUM_TRY(number_div(v, &_z));
		}
	}
    pfm_outconv(v);

    return RC_NRM;
    
  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(y);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
static long
pfm_repeat_mul(pfmnum_t *v, pfmnum_t *x, long cnt)
{
    long rc;

    if (cnt < 0) {
        return ERROR_NUMBER_INVALID_ARGUMENT;
    }
    else if (cnt == 0) {
        *v = PFMNUM_ONE;
        return ERROR_NONE;
    }
    else if (cnt == 1) {
        *v = *x;
        return ERROR_NONE;
    }

    NUM_TRY(pfm_repeat_mul(v, x, cnt / 2));
    NUM_TRY(number_mul(v, v));
    if (cnt % 2 != 0) {
        NUM_TRY(number_mul(v, x));
    }
    return ERROR_NONE;

  PFM_CATCH:
    return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
char *
pfmNumPrint(pfmnum_t x)
{

#if 0 /* IMS. 141014 : Multi Thread Utility */	
    return pfmnum_print(&x, PFMNUM_TO_STRN);
#else
    static char * buf = NULL;	
	static long NowThreadMaxIdx=-1;
	long thread_idx;
	
	if(pfmNumThreadSupportYN() == FLAG_PFM_NUM_THREAD_USE_Y) {
		pthread_mutex_lock(&mutex);	
		thread_idx = pfmGetThreadIdx(pthread_self());
		if(NowThreadMaxIdx < thread_idx) {
				NowThreadMaxIdx = thread_idx;
				buf = realloc(buf, (PFMNUM_MAX_BUF_SIZE + 1) * (NowThreadMaxIdx+1));  
		}

		memcpy(buf + (PFMNUM_MAX_BUF_SIZE + 1) * thread_idx, pfmnum_print(&x, PFMNUM_TO_STRN), PFMNUM_MAX_BUF_SIZE + 1);
		pthread_mutex_unlock(&mutex);
		return buf + (PFMNUM_MAX_BUF_SIZE + 1) * thread_idx;
	} else {
		return pfmnum_print(&x, PFMNUM_TO_STRN);
	}
#endif

}

/* --------------------------------------- function body ---------------------------------------- */
char *
pfmNumPrintComma(pfmnum_t x)
{
#if 0 /* IMS. 141014 : Multi Thread Utility */	
    return pfmnum_print(&x, PFMNUM_TO_CMASTRN);
#else
    static char * buf = NULL;	
	static long NowThreadMaxIdx=-1;
	long thread_idx;
	
	if(pfmNumThreadSupportYN() == FLAG_PFM_NUM_THREAD_USE_Y) {
		pthread_mutex_lock(&mutex);	
		thread_idx = pfmGetThreadIdx(pthread_self());
		if(NowThreadMaxIdx < thread_idx) {
				NowThreadMaxIdx = thread_idx;
				buf = realloc(buf, (PFMNUM_MAX_BUF_SIZE + 1) * (NowThreadMaxIdx+1));  
		}

		memcpy(buf + (PFMNUM_MAX_BUF_SIZE + 1) * thread_idx, pfmnum_print(&x, PFMNUM_TO_CMASTRN), PFMNUM_MAX_BUF_SIZE + 1);
		pthread_mutex_unlock(&mutex);
		return buf + (PFMNUM_MAX_BUF_SIZE + 1) * thread_idx;
	} else {
		return pfmnum_print(&x, PFMNUM_TO_CMASTRN);
	}
#endif
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumRoundAt(pfmnum_t *v, pfmnum_t x, long pos)
{
    long rc;
    long neg_pos;

    PFM_CHECK_NULL(v);

    if (pfm_is_int_range(pos) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }
    
    pfm_inconv(&x);
    
    neg_pos = - pos;

    NUM_TRY( number_round_at(&x, (int)neg_pos) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(pos);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumTruncAt(pfmnum_t *v, pfmnum_t x, long pos)
{
    long rc;
    long neg_pos;

    PFM_CHECK_NULL(v);

    if (pfm_is_int_range(pos) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }
    
    pfm_inconv(&x);
    
    neg_pos = - pos;

    NUM_TRY( number_trunc_at(&x, (int)neg_pos) ); 
    
    *v = x;
    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(pos);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumRoundUp(pfmnum_t *v, pfmnum_t x, long pos)
{
    long rc;
    long i;
    pfmnum_t _y, _z;
    long neg_pos;
    char tmp[PFMNUM_BUF_SIZE + 1];
    long neg_flag = 0;

    PFM_CHECK_NULL(v);

    pfm_inconv(&x);

    if (number_is_negative(&x)) {
        neg_flag = 1;
        rc = number_negate(&x);
        if (rc != ERROR_NONE) {
            goto PFM_CATCH;
        }
    }
    else {
        neg_flag = 0;
    }

    if (pfm_is_int_range(pos) == FALSE) {
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }

    neg_pos = - pos;
    
    _z = x;

    NUM_TRY( number_trunc_at(&x, (int)neg_pos) ); 

    if (number_cmp(&x, &_z) == 0) {
        if (neg_flag == 1) {
            rc = number_negate(&x);
            if (rc != ERROR_NONE) {
                goto PFM_CATCH;
            }
        }
        *v = x;
        pfm_outconv(v);
        return RC_NRM;
    }

    if (pos > 0) {
        tmp[0] = '0';
        tmp[1] = '.';
        for (i = 0; i < pos - 1; i++) {
            tmp[2 + i] = '0';
        }
        tmp[2 + i] = '1';
        tmp[3 + i] = 0x00;
    }
    else if (pos == 0) {
        strcpy(tmp, "1");
    }
    else {
        tmp[0] = '1';
        for (i = 0; i < neg_pos ; i++) {
            tmp[1 + i] = '0';
        }
        tmp[1 + i] = 0x00;
    }

    NUM_TRY( number_from_str(&_y, (int)strlen(tmp), tmp, "N"));

    NUM_TRY( number_add(&x, &_y));

    if (neg_flag == 1) {
        rc = number_negate(&x);
        if (rc != ERROR_NONE) {
            goto PFM_CATCH;
        }
    }

    *v = x;

    pfm_outconv(v);
    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(v);
    EMSG_APND_NUM(x);
    EMSG_APND_LONG(pos);
    return RC_ERR;

}

/* --------------------------------------- function body ---------------------------------------- */
static char *
pfmnum_print(pfmnum_t *x, long type)
{
    long rc ;
    static char buf[PFMNUM_MAX_BUF_CNT][PFMNUM_MAX_BUF_SIZE + 1];
    static long pos = 0;
    
    pos %= PFMNUM_MAX_BUF_CNT;

    bzero(buf[pos], PFMNUM_MAX_BUF_SIZE + 1);
    
    switch (type) {
    case PFMNUM_TO_STRN:
        rc = pfmNumToStrn(buf[pos], *x, PFMNUM_MAX_BUF_SIZE);
        break;
    case PFMNUM_TO_CMASTRN:
        rc = pfmNumToCommaStrN(buf[pos], *x, PFMNUM_MAX_BUF_SIZE);
        break;
    default:
        strncpy(buf[pos], "INTERNAL ERROR(INVALID_TYPE)", PFMNUM_MAX_BUF_SIZE);
        return buf[pos++];
    }

    if (rc != RC_NRM) {
        strncpy(buf[pos], "INVALID NUMBER", PFMNUM_MAX_BUF_SIZE);
    }
    return buf[pos++];
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumFindLocationOfLeastSignificantDigit(pfmnum_t x)
{
    if ( memcmp(&x, &g_pfmnum_zero, sizeof(pfmnum_t)) == 0) {
        return 0;
    }

    return - (number_get_exp10((&x)) + 1 - number_get_siglen10((&x)));
}

/* --------------------------------------- function body ---------------------------------------- */
static char * 
__pfmNumCalcVaList( char *fmt, va_list *p_args)
{
    long i;
    char expr[1024];
    char post[1024];
    char t_fmt[1024]; 
    char rc_str[PFMNUM_BUF_SIZE];
    char *rc_p;

#if 1 /* HKLEE 20151201 IMS.91383 */
    char g_pfmnum_emsg_tmp  [PFMNUM_EMSG_BUF_SIZE + 1];
#endif

	long inc_fmt;
    pfmnum_t t_num;
	va_list args;
	memcpy(&args, p_args, sizeof(va_list));

    bzero(expr, sizeof(expr));
    while (*fmt)
        {
            if (*fmt == ' ' || *fmt == '\t' || *fmt == '\n' || *fmt == '\r') 
                {
                    fmt++;
                    continue;
                }

			inc_fmt = 1;
            if ((*fmt == '%')
                && (*(fmt+1) != ' ' && *(fmt+1) != '\t' && *(fmt+1) != '\n' &&
                    *(fmt+1) != '\r' && *(fmt+1) != '\0' && *(fmt+1) != '%'))
                {
                    bzero(t_fmt, sizeof(t_fmt));
                    memcpy(t_fmt, fmt, strlen(fmt));
                    rc_p = strpbrk(&t_fmt[1], "dfns%");
                    if (rc_p == NULL) 
                        {
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
							memcpy(p_args, &args, sizeof(va_list));
                            return NULL;
                        }
          
					inc_fmt = (long)(rc_p - t_fmt);
                    *(rc_p+1) = '\0';
                    PRINTF("t_fmt : %s\n", t_fmt);

                    switch (t_fmt[strlen(t_fmt)-1])
                        {
                        case 'd' : 
                            sprintf(&expr[strlen(expr)], t_fmt, (long)va_arg(args, long));
                            break;
                        case 'f' :
                            sprintf(&expr[strlen(expr)], t_fmt, (double)va_arg(args, double));
                            break;
                        case 's' :
                            sprintf(&expr[strlen(expr)], t_fmt, (char *)va_arg(args, char *));
                            break;
                        case 'n' :
#ifdef _AIX
						    memcpy( &t_num , args , sizeof(pfmnum_t) );
							va_arg( args, pfmnum_t);
#else
                            t_num = va_arg(args, pfmnum_t);
#endif
                            pfmNumToStr(rc_str, t_num);
                            sprintf(&expr[strlen(expr)], "%s", rc_str);
                            break;
                        default :
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
							memcpy(p_args, &args, sizeof(va_list));
                            return NULL;
                        }
                }
            else
                {
                    expr[strlen(expr)] = *fmt;
                }
            fmt += inc_fmt;
        }

	memcpy(p_args, &args, sizeof(va_list));

	bzero( post, sizeof(post) );
    if (postfix(post, expr) == RC_ERR) return NULL;
    PRINTF("Postfix : %s\n", post);

    if (!is_legal(post))
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, expr);
            return NULL;
        }

    if ((rc_p = calc(post)) == NULL)
        {
 #if 0 /* HKLEE 20151201 IMS.91383 */
             snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:%s", __LINE__, pfmNumGetErrorMsg());
 #else
             snprintf(g_pfmnum_emsg_tmp, sizeof(g_pfmnum_emsg_tmp), "pfmnum_calc:%d:%s", __LINE__, pfmNumGetErrorMsg());
             memcpy(g_pfmnum_emsg, g_pfmnum_emsg_tmp, sizeof(g_pfmnum_emsg));
 #endif
            return NULL;
        }
    
    return rc_p;
}


#if 0
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCalc(pfmnum_t *num, char *fmt, ...)
{
    va_list args;
    char expr[1024];
    char post[1024];
    char rc_str[64];
    char t_fmt[1024]; 
    char *rc_p;
    pfmnum_t t_num;

    bzero(expr, sizeof(expr));
    va_start(args, fmt);
    while (*fmt)
        {
            if (*fmt == ' ' || *fmt == '\t' || *fmt == '\n' || *fmt == '\r') 
                {
                    fmt++;
                    continue;
                }

            if ((*fmt == '%')
                && (*(fmt+1) != ' ' && *(fmt+1) != '\t' && *(fmt+1) != '\n' &&
                    *(fmt+1) != '\r' && *(fmt+1) != '\0' && *(fmt+1) != '%'))
                {
                    bzero(t_fmt, sizeof(t_fmt));
                    memcpy(t_fmt, fmt, strlen(fmt));
                    rc_p = strpbrk(&t_fmt[1], "dfns%");
                    if (rc_p == NULL) 
                        {
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
                            return RC_ERR;
                        }
          
                    *(rc_p+1) = '\0';;

                    PRINTF("t_fmt : %s\n", t_fmt);

                    switch (t_fmt[strlen(t_fmt)-1])
                        {
                        case 'd' : 
                            sprintf(&expr[strlen(expr)], t_fmt, (long)va_arg(args, long));
                            break;
                        case 'f' :
                            sprintf(&expr[strlen(expr)], t_fmt, (double)va_arg(args, double));
                            break;
                        case 's' :
                            sprintf(&expr[strlen(expr)], t_fmt, (char *)va_arg(args, char *));
                            break;
                        case 'n' :
#ifdef _AIX
						    memcpy( &t_num , args , sizeof(pfmnum_t) );
							va_arg( args, pfmnum_t);
#else
                            t_num = va_arg(args, pfmnum_t);
#endif
                            pfmNumToStr(rc_str, t_num);
                            sprintf(&expr[strlen(expr)], "%s", rc_str);
                            break;
                        default :
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
                            return RC_ERR;
                        }
                }
            else
                {
                    expr[strlen(expr)] = *fmt;
                }
            fmt++;
        }
    va_end(args);

    if (postfix(post, expr) == RC_ERR) return RC_ERR;
    PRINTF("Postfix : %s\n", post);

    if (!is_legal(post))
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, expr);
            return RC_ERR;
        }

    if ((rc_p = calc(post)) == NULL)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:%s", __LINE__, pfmNumGetErrorMsg());
            return RC_ERR;
        }
    sprintf(rc_str, "%s", rc_p);
    PRINTF("num[%p] rc_str [%s]\n", num, rc_str);
    pfmNumGetFromStr(num, rc_str);
    PRINTF("after from str rc_str [%s]\n", rc_str);

    return RC_NRM;
}

#else

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCalcVaList(pfmnum_t *num, char *fmt, va_list *p_args)
{
	long rc = RC_NRM;
	char rc_str[PFMNUM_BUF_SIZE];
	char *rc_p;
#if 0 /* IMS. 141014 : Multi Thread Utility */
	rc_p = __pfmNumCalcVaList( fmt, p_args);
#else
	if(pfmNumThreadSupportYN() == FLAG_PFM_NUM_THREAD_USE_Y) {
		pthread_mutex_lock(&mutex);
		rc_p = __pfmNumCalcVaList( fmt, p_args);
		pthread_mutex_unlock(&mutex);
	} else {
			rc_p = __pfmNumCalcVaList( fmt, p_args);
	}
#endif	

	if ( rc_p == NULL ) return RC_ERR;

    sprintf(rc_str, "%s", rc_p);
    PRINTF("num[%p] rc_str [%s]\n", num, rc_str);
    pfmNumGetFromStr(num, rc_str);
    PRINTF("after from str rc_str [%s]\n", rc_str);

    return RC_NRM;
}


/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCalc(pfmnum_t *num, char *fmt, ...)
{
	long rc;
    va_list args;

    va_start(args, fmt);

	rc = pfmNumCalcVaList(num, fmt, &args);

	va_end(args);

	return rc;
}

#endif


#if 0
/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCalcLong(long *lval, char *fmt, ...)
{
    long i;
    va_list args;
    char expr[1024];
    char post[1024];
    char t_fmt[1024]; 
    char rc_str[64];
    char *rc_p;
    long len;
    pfmnum_t t_num;

    bzero(expr, sizeof(expr));
    va_start(args, fmt);
    while (*fmt)
        {
            if (*fmt == ' ' || *fmt == '\t' || *fmt == '\n' || *fmt == '\r') 
                {
                    fmt++;
                    continue;
                }

            if ((*fmt == '%')
                && (*(fmt+1) != ' ' && *(fmt+1) != '\t' && *(fmt+1) != '\n' &&
                    *(fmt+1) != '\r' && *(fmt+1) != '\0' && *(fmt+1) != '%'))
                {
                    bzero(t_fmt, sizeof(t_fmt));
                    memcpy(t_fmt, fmt, strlen(fmt));
                    rc_p = strpbrk(&t_fmt[1], "dfns%");
                    if (rc_p == NULL) 
                        {
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
                            return RC_ERR;
                        }
          
                    *(rc_p+1) = '\0';
                    PRINTF("t_fmt : %s\n", t_fmt);

                    switch (t_fmt[strlen(t_fmt)-1])
                        {
                        case 'd' : 
                            sprintf(&expr[strlen(expr)], t_fmt, (long)va_arg(args, long));
                            break;
                        case 'f' :
                            sprintf(&expr[strlen(expr)], t_fmt, (double)va_arg(args, double));
                            break;
                        case 's' :
                            sprintf(&expr[strlen(expr)], t_fmt, (char *)va_arg(args, char *));
                            break;
                        case 'n' :
#ifdef _AIX
						    memcpy( &t_num , args , sizeof(pfmnum_t) );
							va_arg( args, pfmnum_t);
#else
                            t_num = va_arg(args, pfmnum_t);
#endif
                            pfmNumToStr(rc_str, t_num);
                            sprintf(&expr[strlen(expr)], "%s", rc_str);
                            break;
                        default :
                            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, fmt);
                            return RC_ERR;
                        }
                }
            else
                {
                    expr[strlen(expr)] = *fmt;
                }
            fmt++;
        }
    va_end(args);

    if (postfix(post, expr) == RC_ERR) return RC_ERR;
    PRINTF("Postfix : %s\n", post);

    if (!is_legal(post))
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:잘못된 표현식[%s]", __LINE__, expr);
            return RC_ERR;
        }

    if ((rc_p = calc(post)) == NULL)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:%s", __LINE__, pfmNumGetErrorMsg());
            return RC_ERR;
        }
    
#if 0
    sprintf(str, "%s", rc_p);
#else
    len = strlen(rc_p);
    for (i = 0; i < len; i++) {
        if (rc_p[i] == '.') {
            rc_p[i] = 0x00;
            break;
        }
    }
    *lval = atol(rc_p);
#endif

    return RC_NRM;
}

#else

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumCalcLongVaList(long *lval, char *fmt, va_list *p_args)
{
	long rc = RC_NRM;
	/* char rc_str[PFMNUM_BUF_SIZE]; */
	char *rc_p;

#if 0 /* IMS. 141014 : Multi Thread Utility */
	rc_p = __pfmNumCalcVaList( fmt, p_args);
#else
	if(pfmNumThreadSupportYN() == FLAG_PFM_NUM_THREAD_USE_Y) {
		pthread_mutex_lock(&mutex);
		rc_p = __pfmNumCalcVaList( fmt, p_args);
		pthread_mutex_unlock(&mutex);
	} else {
			rc_p = __pfmNumCalcVaList( fmt, p_args);
	}
#endif	
	if ( rc_p == NULL ) return RC_ERR;

#if 0
    sprintf(str, "%s", rc_p);
#else
	{
		long i;
		long len = strlen(rc_p);
		for (i = 0; i < len; i++)
		{
			if (rc_p[i] == '.')
			{
				rc_p[i] = 0x00;
				break;
			}
		}
		*lval = atol(rc_p);
	}
#endif

    return RC_NRM;
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumCalcLong(long *lval, char *fmt, ...)
{
	long rc;
    va_list args;

    va_start(args, fmt);

	rc = pfmNumCalcLongVaList(lval, fmt, &args);

	va_end(args);

	return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumCalcDouble(double *dval, char *fmt, ...)
{
	long rc;

	PfmNumber num;
	double d_out_num = 0.0;
    char * rc_str;

    va_list args;

    va_start(args, fmt);

    rc = pfmNumCalcVaList(&num, fmt, &args);

    va_end(args);

	rc_str = pfmNumPrint(num);

    *dval = atof(rc_str);

    return rc;
}
#endif

/* --------------------------------------- function body ---------------------------------------- */
static void 
init_int_stack(void)
{
    g_pfmutil_int_top = -1;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
int_push(int int_t)
{
    if (g_pfmutil_int_top >= STACK_MAX - 1)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:stack overflow", __LINE__);
            return RC_ERR;
        }
    g_pfmutil_int_stack[++g_pfmutil_int_top] = int_t;
    return int_t;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
int_pop(void)
{
    if (g_pfmutil_int_top < 0)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:stack underflow", __LINE__);
            return RC_ERR;
        }
    return g_pfmutil_int_stack[g_pfmutil_int_top--];
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
get_int_stack_top(void)
{
    return (g_pfmutil_int_top < 0) ? -1 : g_pfmutil_int_stack[g_pfmutil_int_top];
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
is_int_stack_empty(void)
{
    return (g_pfmutil_int_top < 0);
}

/* --------------------------------------- function body ---------------------------------------- */
static void 
init_num_stack(void)
{
    g_pfmnum_top = -1;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
num_push(pfmnum_t num_t)
{
    if (g_pfmnum_top >= STACK_MAX - 1)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:stack overflow", __LINE__);
            return RC_ERR;
        }

    pfmNumAssign(&g_pfmnum_stack[++g_pfmnum_top], num_t);
    return RC_NRM;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
num_pop(pfmnum_t *num_t)
{
    if (g_pfmnum_top < 0)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:stack underflow", __LINE__);
            return RC_ERR;
        }
    pfmNumAssign(num_t, g_pfmnum_stack[g_pfmnum_top--]);
    return RC_NRM;
}

/* --------------------------------------- function body ---------------------------------------- */
#if 0
static long 
get_num_stack_top(pfmnum_t *num_t)
{
    if (g_pfmnum_top < 0)
        {
            snprintf(g_pfmnum_emsg, sizeof(g_pfmnum_emsg), "pfmnum_calc:%d:stack empty", __LINE__);
            return RC_ERR;
        }
    pfmNumAssign(num_t, g_pfmnum_stack[g_pfmnum_top]);
    return RC_NRM;
}
#endif

/* --------------------------------------- function body ---------------------------------------- */
#if 0
static long 
is_num_stack_empty(void)
{
    return (g_pfmnum_top < 0);
}
#endif

/* --------------------------------------- function body ---------------------------------------- */
static int  
is_operator(int ch)
{
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '^');
}

/* --------------------------------------- function body ---------------------------------------- */
static int  
is_legal(char *expr)
{
    int gap = 0;

    PRINTF("expr = [%s]\n", expr);
    if (*expr  == '-') expr++;

    while (*expr)
        {
            while (*expr == ' ') expr++;  /* remove space */

            if ((is_operator(*expr) && (*(expr+1) == ' '))
                || (is_operator(*expr) && (*(expr+1) == '\0' )))
                {
                    PRINTF("sub gap[%c] = %d\n", *expr, gap);
                    gap--;
                }
            else
                {
                    PRINTF("add gap[%c] = %d\n", *expr, gap);
                    gap++;
                    while (*expr != ' ' &&  *expr)
                        {
                            expr++;
                        }
                }

/*        if (*expr == NULL) break;*/
            if (gap < 1) break;

            expr++;
        }

    PRINTF("gap = %d\n", gap);
    return (gap == 1);   /* legal if valuable - operator == 1 */
}

/* --------------------------------------- function body ---------------------------------------- */
static int  
precedence(int op)
{
    if (op == '(') return 0;
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%' || op == '^') return 2;
    else return 3;
}

/* --------------------------------------- function body ---------------------------------------- */
static long  
postfix(char *dst, char *src)
{
    char ch;
    char unary = 1;

    init_int_stack();

#if 0
	/* 원래 버그는 AIX에서 char -> unsigned char 디폴트로 적용되어 생긴 문제임.          */
	/* 따라서 이 부분 루틴은 필요 없고, 근본적으로 char = signed char 임을 보장해주면 OK */
	int cnt = 0;
	char *l_src = src;

	while(*l_src)
	{
		if ( *l_src == '(' )
			cnt++;
		else if ( *l_src == ')' )
			cnt--;

		l_src++;
	}

	if (cnt != 0)
		return RC_ERR;
#endif

    while (*src)
        {
            if (*src == '(')
                {
                    if (int_push(*src) == RC_ERR) return RC_ERR;
                    src++;
                    unary = 1;
                }
            else if (*src == ')')
                {
                    while (get_int_stack_top() != '(')
                        {
                            if ((ch = (char)int_pop()) == RC_ERR) return RC_ERR;
                            *dst++ = ch;
                            *dst++ = ' ';
                        }
                    if (int_pop() == RC_ERR) return RC_ERR;
                    src++;
                    unary = 0;
                }
            else if ((*src >= '0' && *src <= '9')
                     || (unary && (*src == '-') && (*(src+1) >= '0' && *(src+1) <= '9')))
                {
                    do
                        {
                            *dst++ = *src++;
                        } while ((*src >= '0' && *src <= '9') || (*src == '.'));
                    *dst++ = ' ';
                    unary = 0;
                }
            else if (is_operator(*src))
                {
                    PRINTF("operator=[%c]\n", *src);
                    while (!is_int_stack_empty() && precedence((int)get_int_stack_top()) >= precedence((int)*src))
                        {
                            if ((ch = (char)int_pop()) == RC_ERR) return RC_ERR;
                            *dst++ = ch;
                            *dst++ = ' ';
                        }
                    if (int_push(*src) == RC_ERR) return RC_ERR;
                    src++;
                    unary = 1;
                }
            else
                {
                    src++;
                }
        }

    while (!is_int_stack_empty())
        {
            if ((ch = (char)int_pop()) == RC_ERR) return RC_ERR;
            *dst++ = ch;
            *dst++ = ' ';
        }
    dst--;
    *dst = 0;

    return 0;
}

/* --------------------------------------- function body ---------------------------------------- */
static 
char *calc(char *expr)
{
    int  ix;
    static char num_str[PFMNUM_BUF_SIZE];
    pfmnum_t num_t, num_a, num_b;

    init_num_stack();

    while (*expr)
        {
            if ((*expr >= '0' && *expr <= '9')
                || (*expr == '-' && *(expr+1) != ' ' && *(expr+1)))
                {
                    bzero(num_str, sizeof(num_str));
                    for (ix = 0; (*expr >= '0' && *expr <= '9') || (*expr == '.') || (*expr == '-'); ix++)
                        {
                            num_str[ix] = *expr;
                            expr++;
                        } 
                    PRINTF("num_str=[%s]\n", num_str);
                    pfmNumGetFromStr(&num_t, num_str);
                    if (num_push(num_t) == RC_ERR) return NULL;
                }
            else if (*expr == '+')
                {
                    if (num_pop(&num_a) == RC_ERR) return NULL;
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (pfmNumAdd(&num_t, num_a, num_b) == RC_ERR) return NULL;
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else if (*expr == '*')
                {
                    if (num_pop(&num_a) == RC_ERR) return NULL;
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (pfmNumMul(&num_t, num_a, num_b) == RC_ERR) return NULL;
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else if (*expr == '-')
                {
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (num_pop(&num_a) == RC_ERR) return NULL;
                    if (pfmNumSub(&num_t, num_a, num_b) == RC_ERR) return NULL;
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else if (*expr == '/')
                {
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (num_pop(&num_a) == RC_ERR) return NULL;
                    if (pfmNumDiv(&num_t, num_a, num_b) == RC_ERR) return NULL;
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else if (*expr == '%')
                {
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (num_pop(&num_a) == RC_ERR) return NULL;
                    if (pfmNumMod(&num_t, num_a, num_b) == RC_ERR) return NULL;
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else if (*expr == '^')
                {
                    if (num_pop(&num_b) == RC_ERR) return NULL;
                    if (num_pop(&num_a) == RC_ERR) return NULL;
#ifdef NH_API
                    if (pfmNumPowerN(&num_t, num_a, num_b) == RC_ERR) return NULL;
#else
                    if (pfmNumPower(&num_t, num_a, num_b) == RC_ERR) return NULL;
#endif
                    if (num_push(num_t) == RC_ERR) return NULL;
                    expr++;
                }
            else
                {
                    expr++;
                }
        }

    if (num_pop(&num_t) == RC_ERR) return NULL;
    pfmNumToStr(num_str, num_t);

    return num_str;
}


/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumGetIntLen( pfmnum_t x )
{
    long exp10;
    long int_len;
	
    if ( memcmp( &x, &g_pfmnum_zero, sizeof(PfmNumber) ) == 0 ) exp10 = 0;
	else                                                        exp10 = number_get_exp10((&x));

    if ( exp10 < 0 ) int_len = 1;
	else             int_len = exp10 + 1;
	
	return int_len;
}
/* --------------------------------------- function body ---------------------------------------- */
long
pfmNumToInFmtStrN_EU
(char *str, long buf_size, char *frmt, long right_align_flag, long lpad_zero_flag, pfmnum_t x)
{
    char mybuf[PFMNUM_MAX_BUF_SIZE + 1];
    long rc;

    strncpy(mybuf, frmt, PFMNUM_MAX_BUF_SIZE + 1);
    for(int i_loop=0; i_loop<strlen(mybuf); i_loop++)
    {
	if(mybuf[i_loop] == ','||mybuf[i_loop] == '.') mybuf[i_loop] = mybuf[i_loop] == ',' ? '.' : ',';
    }

    rc=pfmNumToInFmtStrN(str, buf_size, mybuf, right_align_flag, lpad_zero_flag, x);

    strncpy(mybuf, str, PFMNUM_MAX_BUF_SIZE + 1);
    for(int i_loop=0; i_loop<strlen(mybuf); i_loop++)
    {
	if(mybuf[i_loop] == ','||mybuf[i_loop] == '.') mybuf[i_loop] = mybuf[i_loop] == ',' ? '.' : ',';
    }

    memcpy(str, mybuf, PFMNUM_MAX_BUF_SIZE + 1);
    return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfmNumToInFmtStrN
(char *str, long buf_size, char *frmt, long right_align_flag, long lpad_zero_flag, pfmnum_t x)
{
    long rc ;
    char mybuf[PFMNUM_MAX_BUF_SIZE + 1];
    char mybuf2[PFMNUM_MAX_BUF_SIZE + 1];
    char align;
    char lpad_zero = 0;
    
    char	rst_str[PFMNUM_MAX_BUF_SIZE + 1];
    char	*fmt_str;

    PFM_CHECK_NULL(str);
    
    if (buf_size <= 0)
	{
        rc = ERROR_WRONG_LENGTH;
        goto PFM_CATCH;
    }

    if (buf_size > PFMNUM_MAX_BUF_SIZE)
	{
        rc = ERROR_TOO_BIG_BUFSIZE;
        goto PFM_CATCH;
    }

    if (pfm_is_int_range(buf_size) == FALSE)
	{
        rc = ERROR_TOO_BIG_LONG_VALUE;
        goto PFM_CATCH;
    }

    pfm_inconv(&x);

	memset(str, 0x00, buf_size);

#if 0
    NUM_TRY( number_to_str(&x, (int)buf_size + 1, mybuf2, "N"));
    NUM_TRY( pfm_strconv(mybuf, mybuf2, buf_size ));
#else
    NUM_TRY( number_to_str(&x, PFMNUM_MAX_BUF_SIZE + 1, mybuf2, "N"));
    NUM_TRY( pfm_strconv(mybuf, mybuf2, PFMNUM_MAX_BUF_SIZE ));
#endif
  
    if (right_align_flag) {
        align = 'R';
    }
    else {
        align = 'L';
    }
    
    if (lpad_zero_flag > 0)      lpad_zero = 1;
    else if (lpad_zero_flag < 0) lpad_zero = -1;
    else                         lpad_zero = 0;
    
#if 0   /* 크기가 1작은 현상 해결 1은 NULL이 사용함 */
    rc = pfm_format_align(str, buf_size, frmt, mybuf, align, lpad_zero);
#else
    NUM_TRY( pfm_format_align(str, buf_size + 1, frmt, mybuf, align, lpad_zero) );
#endif

#if 0 /* 20080425: MRH - 불필요한 로직때문에 정렬이 잘 안됨 */
    /* 2005.11.28 추가 */
    /* 원화, \ 표시와 금액사이의 Space 제거 */
    fmt_str = pfmnum_nrmspace(str, buf_size);

    PRINTF("pfmnum_nrmspace Call 후 buf_size[%ld] fmt_str[%s]\n", buf_size, fmt_str);    
		
    sprintf(rst_str, "%*.*s", (int)buf_size, (int)buf_size, fmt_str);
   
    if (rc != RC_NRM) {
        goto PFM_CATCH;
    }
    
    
    PRINTF("Return 전 str[%ld] str[%s]\n", buf_size, rst_str);    

    strcpy(str, rst_str);
#endif /* 20080425: MRH - 불필요한 로직때문에 정렬이 잘 안됨 */

    return RC_NRM;

  PFM_CATCH:
    EMSG_INIT();
    EMSG_APND_PTR(str);
    EMSG_APND_LONG(buf_size);
    EMSG_APND_STR(frmt);
    EMSG_APND_LONG(right_align_flag);
    EMSG_APND_LONG(lpad_zero_flag);
    EMSG_APND_NUM(x);
    return RC_ERR;
}

/* --------------------------------------- function body ---------------------------------------- */
long 
pfm_isnumber(char *str)
{
    long ix;
    long first_pos = 0;
    long last_pos  = 0;
    long point_pos = 0;

    for (ix = 0; ix < (long)strlen(str); ix++)
        {
            if (str[ix] == ' ')
                {
                    if (first_pos != 0)  last_pos = 1;
                }
            else if (isdigit(str[ix]))
                {
                    if (first_pos == 0) first_pos = 1;
                    if (last_pos) return FALSE;
                }

            else if (str[ix] == '-')
                {
                    if (first_pos) return FALSE;
                    if (first_pos == 0) first_pos = 1;
                }
            else if (str[ix] == '.')
                {
                    if (point_pos) return FALSE;
                    if (first_pos == 0) return FALSE;
                    if (point_pos == 0) point_pos = 1;
                }
            else
                {
                    return FALSE;
                }
        }

    return TRUE;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
pfm_format_align(char *dst, long dst_len, const char *in_fmt, char *src, char align, char lpad_zero)
{
    char f_str[PFMNUM_BUF_SIZE + 1];
    long f_len;
    long rc = RC_NRM;
    char *tail_ptr = NULL;
    long tail_len = 0;
    long zero_pad_cnt =0;
	char *fmt = NULL;
    long fmt_len;
	long fmt_offset = 0;
    long i;

	fmt = strdup(in_fmt);

    fmt_len = pfm_min_strlen(fmt, PFMNUM_BUF_SIZE);
    if (fmt_len >= PFMNUM_BUF_SIZE)
	{
        rc = ERROR_TOO_BIG_FRMT;
		goto PFM_CATCH;
    }

    /* TAIL CHECK */
    for (i = fmt_len - 1; i >= 0; i--)
	{
        if (fmt[i] == '#' || fmt[i] == '0')
		{
            /* save ptr */
            tail_ptr = strdup(fmt + i + 1);
            /* cut fmt data */
            fmt[i + 1] = 0x00;
            /* adjust argument dst_len */
            tail_len = (fmt_len - 1) - i;
			fmt_len  = i + 1;
            break;
        }
    }

#if 0
    printf(">>>>>>>> Dest Source [%ld][%s] \n", dst_len, dst);  
#endif
    
    /* HEAD APPEND */
    if ( fmt[0] != '#' && fmt[0] != '0')
	{
        for (  i = 0 ; i < fmt_len; i++)
		{
            if (fmt[i] == '#' || fmt[i] == '0')
			{
				if ( i > dst_len - 1 )
				{
					strncpy(dst, fmt, dst_len - 1);

					PRINTF("[i[%ld] > dst_len [%ld]\n", i, dst_len);

					rc = ERROR_BUF_NOT_ENOUGH;
					goto PFM_CATCH;
				}
                 /* set head data */
                strncpy(dst, fmt, i);
                /* cut fmt data */
                //fmt += i;
				fmt_offset = i;
                /* adjust buf ptr */
                dst += i;
                /* adjust argument dst_len */
                dst_len -= i;
                break;
            }
        }
    }

#if 0
    printf(">>>>>>>> PFM_FORMAT_ALIGN----HEAD APPEND dst_len[%ld] \n", dst_len);  
#endif   
    
    bzero(f_str, sizeof(f_str));
	rc = pfm_format_str(f_str, fmt + fmt_offset, src);
	free(fmt);
	fmt = NULL;

	if ( rc != RC_NRM)
	{
		goto PFM_CATCH;
	}
     	
#if 0
    printf(">>>>>>>> pfm_format_str Call 후 ---- f_str[%d][%s]\n", strlen(f_str), f_str); 
#endif

#if 0
	f_len = sprintf( dst, "%s", f_str );
#else
    f_len = strlen(f_str);
#endif

	if (f_len + tail_len > dst_len - 1)
	{

		if ( f_len > 0 )
		{
			if ( f_len < dst_len - 1 )
			{
				sprintf (dst, "%s%.*s", f_str, (int)(dst_len-1-f_len), tail_ptr);
			}
			else
			{
				sprintf (dst, "%.*s", dst_len - 1, f_str);
			}
		}

		PRINTF("[f_len[%ld] + tail_len[%ld] > dst_len [%ld]\n", f_len, tail_len, dst_len);

		rc = ERROR_BUF_NOT_ENOUGH;
		goto PFM_CATCH;
	}
     
#if 0
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>[f_len[%ld], dst_len [%ld]\n", f_len, dst_len);
#endif

    switch (toupper(align))
        {
        case 'L' : 
            if ( lpad_zero == -1 &&
				(zero_pad_cnt = dst_len - f_len - 1 - tail_len) > 0 )
			{
				if ( strchr( f_str, '.' ) == NULL )
				{
					if ( zero_pad_cnt > 1 )
					{
						sprintf (dst, "%s.%0*d%s", f_str, zero_pad_cnt - 1, 0, tail_ptr);
					}
					else
					{
						sprintf (dst, "%s.%s", f_str, tail_ptr);

						rc = ERROR_BUF_NOT_ENOUGH;
						goto PFM_CATCH;
					}
				}
				else
				{
					sprintf (dst, "%s%0*d%s", f_str, zero_pad_cnt, 0, tail_ptr);
				}
			}
			else
			{
				sprintf (dst, "%s%-*s", f_str, (int)(dst_len-1-f_len), tail_ptr);
			}
             
#if 0           
            printf("[디버깅중.... Dest Format String----11111111 [dst[%s] ]\n", dst);      
#endif                

            break;
        case 'R' :
            if ( lpad_zero == 1 )
			{
				zero_pad_cnt = dst_len - f_len - 1 - tail_len;

				if (f_str[0] == '-')
				{
					if (zero_pad_cnt > 0)
					{
						sprintf (dst, "%c%0*d%*s%s", f_str[0], (int)zero_pad_cnt, 0, (int)f_len-1, &f_str[1], tail_ptr);
#if 0                   
						printf("[Dest Format String----777777777 [dst[%s] ]\n", dst);      
#endif                            
					}                        
					else
					{
						sprintf (dst, "%c%*s%s", f_str[0], (int)f_len-1, &f_str[1], tail_ptr);
#if 0                    
						printf("[Dest Format String----8888888888 [dst[%s] ]\n", dst);      
#endif              
					}      
					
					
				}
				else
				{
					if (zero_pad_cnt > 0) 
						sprintf (dst, "%0*d%*s%s", (int)zero_pad_cnt, 0, (int)f_len, f_str, tail_ptr);
					else
						sprintf (dst, "%*s%s", (int)f_len, f_str, tail_ptr);
				}
#if 0                   
				printf("[Dest Format String----22222222 [dst[%s] ]\n", dst);   
#endif                         
			}
            else
			{
				/* 11.30 수정 : 앞자리 SPACE로 채움 (%*s%s--> *.*s%s) */
				sprintf (dst, "%*.*s%s", (int)(dst_len-1-tail_len), (int)(dst_len-1-tail_len), f_str, tail_ptr);
#if 0
				printf("FORMATTING >>>>>>>>>>>>>>>>>>>>> dst[%s]\n", dst);    
#endif
			}
             
            break;
        default :
            PRINTF("align [%ld]\n", align);
            rc = ERROR_NUMBER_INVALID_ARGUMENT;
			goto PFM_CATCH;
        }

	if ( fmt != NULL ) free(fmt);
	if ( tail_ptr != NULL ) free(tail_ptr);

    return RC_NRM;

PFM_CATCH:
	if ( fmt != NULL ) free(fmt);
	if ( tail_ptr != NULL ) free(tail_ptr);

	return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
pfm_format_str(char *dst, const char *fmt, char *src)
{
    long ix, jx;
    long max_precision = 0;
    long min_precision = 0;
    long val_cnt       = 0;
    long zero_cnt      = 0;
    long prev_cnt      = 0;
    long len_fmt       = 0;
    long len_prev      = 0;
    long len_post      = 0;
    long is_num        = 1;
    long last_zero     = 0;
    long last_comma    = 0;
    long prev_comma    = 0;
    long last_dot      = 0;
    long comma_gap     = 0;
    char prev_fmt[48];
    char post_fmt[48];
    char prev_str[64];
    char post_str[64];
    char prev_rtn[64];
    char post_rtn[64];
    char *cp;

    len_fmt = strlen(fmt);
    if (len_fmt == 0) return ERROR_INVALID_FRMT;

    for (ix = (len_fmt-1); ix >= 0; ix--)
        {
            switch (fmt[ix])
                {
                case '0' :
                    if (last_zero == 0) last_zero  = ix;
                    if (last_dot  == 0) max_precision++;
                    if (last_dot  == 0 && last_zero) min_precision++;
                    val_cnt++;
                    prev_cnt++;
                    zero_cnt = prev_cnt;
                    break;
                case '#' :
                    if (last_dot == 0) max_precision++;
                    if (last_dot == 0 && last_zero) min_precision++;
                    val_cnt++;
                    prev_cnt++;
                    break;
                case 'A' :
                    is_num = 0;
                    val_cnt++;
                    prev_cnt++;
                    break;
                case ',' :
                    if (last_comma == 0)
                        {
                            last_comma = ix;
                            if (last_dot == 0)
                                comma_gap = len_fmt  - last_comma - 1;
                            else
                                comma_gap = last_dot - last_comma - 1;
                        }
                    else
                        {
                            if (comma_gap != (prev_comma - ix -1)) is_num = 0;
                        }
                    prev_comma = ix;
                    break;
                case '.' :
                    zero_cnt = 0;
                    prev_cnt = 0;
                    if (last_dot == 0) last_dot = ix;
                    else               is_num = 0;
                    if (last_comma) is_num = 0;
                    break;
                case '/' :
                    is_num = 0;
                    break;
                case '-' :
                    is_num = 0;
                    break;
                default  :
                    return ERROR_INVALID_FRMT;
                }

            if ((ix == 0) && (comma_gap < prev_comma)) is_num = 0;
        }
 
    PRINTF("is_num[%d], max[%d], min[%d]\n", is_num, min_precision, max_precision);
    PRINTF("zero_cnt[%d], val_cnt[%d], comma_gap[%d]\n", zero_cnt, val_cnt, comma_gap);

    if (is_num)
        {
            bzero(prev_fmt, sizeof(prev_fmt));
            bzero(post_fmt, sizeof(post_fmt));
            bzero(prev_str, sizeof(prev_str));
            bzero(post_str, sizeof(post_str));

            if (last_dot && (last_dot != (len_fmt-1)))
                {
#if 0        		
                    printf("[%d] --------->last_dot[%ld], fmt[%s]]\n", __LINE__, last_dot, fmt);
#endif
         	
                    sprintf(prev_fmt, "%.*s", (int)last_dot, fmt);
                    sprintf(post_fmt, "%s", &fmt[last_dot+1]);
                }
            else
                {
#if 0
                    printf("[%d]--------->last_dot[%ld] len_fmt[%s], fmt[%s]]\n", __LINE__,last_dot, len_fmt, fmt);
#endif         	
                    sprintf(prev_fmt, "%.*s", (int)(last_dot ? last_dot : len_fmt), fmt);
             
                }
#if 0
            printf("[%d] --------->prev_fmt[%s], post_fmt[%s]]\n", __LINE__,prev_fmt, post_fmt);
#endif

            PRINTF("prev_fmt[%s], post_fmt[%s]]\n", prev_fmt, post_fmt);

            pfmnum_ltrim(pfmnum_rtrim(src));
            if (!pfm_isnumber(src)) return ERROR_INVALID_CONV_STR;
            sprintf(prev_str, "%s", src);
            if ((cp = strchr(prev_str, '.')) != NULL) 
                {
                    sprintf(post_str, "%s", cp+1);
                    *cp = '\0';
                }

            bzero(prev_rtn, sizeof(prev_rtn));
#if 0 /* IMS.122772 pfmNumToFmtStrN  */
            if (zero_cnt > (long)strlen(prev_str))
                {
                    sprintf(prev_str, "%0*ld", (int)zero_cnt, atol(prev_str));
                }
#else
		if(atol(prev_str) >= 1000) {
            if (zero_cnt > (long)strlen(prev_str))
                {
                    sprintf(prev_str, "%0*ld", (int)zero_cnt, atol(prev_str));
                }
		}
#endif
         
#if 0
            printf("[%d] --------->zero_cnt[%ld] prev_str[%s]\n", __LINE__, zero_cnt, prev_str);
#endif   
         

            len_prev = strlen(prev_str);
        
            for (ix = 0; ix < len_prev; ix++)
                {
                    prev_rtn[strlen(prev_rtn)] = prev_str[ix];
             
                    if ( comma_gap &&
                         (((len_prev - ix) % comma_gap) == 1) &&
                         ( ix != (len_prev-1)) && 
                         ((prev_str[ix] >= '0') && (prev_str[ix] <= '9'))) 
                        {
                            prev_rtn[strlen(prev_rtn)] = ',';
                        }
                }
         
#if 0
            printf("[%d] --------->prev_rtn [%s]\n", __LINE__,prev_rtn);
#endif            

            bzero(post_rtn, sizeof(post_rtn));
            len_post = strlen(post_str);
         
#if 0
            printf("[%d] --------->post_fmt [%s]\n", __LINE__,post_fmt);
#endif          
         
            if (post_fmt[0])
                {
#if 0
                    printf("[%d] --------->post_str [%s] min_precision[%ld] len_post[%ld]\n", __LINE__,post_str, min_precision, len_post);
                    printf("[%d] --------->post_str [%s] min_precision[%ld] \n", __LINE__,post_str, max_precision);
#endif   
                    if (len_post < min_precision)
                        {
                            sprintf(post_rtn, "%s%0*d", post_str, (int)(min_precision-len_post), 0);
                        }
                    else
                        {
                            sprintf(post_rtn, "%.*s", (int)max_precision, post_str);
                        }

#if 0
                    printf("[%d] --------->post_rtn [%s]\n", __LINE__,post_rtn);
#endif 
                    /* 2005.11.14 수정 : format이 #이면서 숫자가 0인 경우 Space를 return하기 위해서 */
                    if (post_rtn[0] == '\0' && prev_rtn[0] == '0') {
                        dst = NULL;
                    }
					/* 2008.07.08 추가 : 소수점 이하가 없을때, 소수점이 찍히는 현상을 막기 위해서 */
					else if (post_rtn[0] == '\0')
					{
						sprintf(dst, "%s", prev_rtn);
					}
                    else {
                        sprintf(dst, "%s.%s", prev_rtn, post_rtn);
                    }
                }
            else
                {
                    sprintf(dst, "%s", prev_rtn);
                }
#if 0
            printf("[%d] --------->dst [%s]\n", __LINE__,dst);
#endif           
         
        }
    else
        {
            if (val_cnt != strlen(src)) return ERROR_INVALID_FRMT;

            bzero(post_rtn, sizeof(post_rtn));
            for (ix = 0, jx = 0; ix < (long)strlen(fmt); ix++)
                {
                    if (fmt[ix] == 'A' || fmt[ix] == '#' || fmt[ix] == '0')
                        {
                            post_rtn[strlen(post_rtn)] = src[jx];
                            jx++;
                        }
                    else
                        {
                            post_rtn[strlen(post_rtn)] = fmt[ix];
                        }
                }
            sprintf(dst, "%s", post_rtn);
         
#if 0
            printf("[%d] --------->dst [%s]\n", __LINE__,dst);
#endif            
        }


    return RC_NRM;
}

/* --------------------------------------- function body ---------------------------------------- */
static long 
pfm_min_strlen(char *str, long len)
{
    long i;

    for (i = 0; i < len; i++) {
        if (str[i] == 0x00) {
            return i;
        }
    }

    return len;
}

#if 1 /* IMS. 141014 : Multi Thread Utility */
static long
pfmNumThreadSupportYN( void ) {

    static long num_thread_yn = FLAG_NUM_CALC_NOT_CHECK;

    if(num_thread_yn == FLAG_NUM_CALC_NOT_CHECK) {
        if(getenv("PFM_NUM_THREAD_USE") != NULL) {
			if(getenv("PFM_NUM_THREAD_USE")[0] == 'Y' || getenv("PFM_NUM_THREAD_USE")[0] == 'y'){
				num_thread_yn = FLAG_PFM_NUM_THREAD_USE_Y;
			}
        } else {
            num_thread_yn = FLAG_PFM_NUM_THREAD_USE_N;
        }
    }
    return num_thread_yn;
}
static long 
pfmGetThreadIdx(long ThreadId){
	static thread_idx_t * SvrNameArr=NULL;
	static long  TotThreadIdx=0;
	thread_idx_t * ThreadIdx=NULL;
	long * temp_long;
	
	ThreadIdx = bsearch(&ThreadId, (void *)SvrNameArr, TotThreadIdx, sizeof(thread_idx_t), cmp_svr);
	if(ThreadIdx != NULL) {
		return ThreadIdx->thread_idx;
	}
	else {
		SvrNameArr = realloc(SvrNameArr, sizeof(thread_idx_t)*++TotThreadIdx);                 
		temp_long = (void*)SvrNameArr+(sizeof(thread_idx_t) * (TotThreadIdx-1));               
		*temp_long = ThreadId;                                                                
		temp_long = (void*)SvrNameArr+(sizeof(thread_idx_t) * (TotThreadIdx-1)) + sizeof(long);
		*temp_long = TotThreadIdx-1;                                                             	

		qsort((void *)SvrNameArr, TotThreadIdx, sizeof(thread_idx_t), cmp_svr);
		return TotThreadIdx-1;
	}
}

#endif

