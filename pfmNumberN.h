/*
 * @file            pfmutil_num.h
 * @brief		
 *
 * @dep-header		
 *
 * @history
 *  Version  :  Name   :    Date    :  Basis        :  Description
 *  --------   ------     --------     -----------     ---------------------------------------------
 *  Ver 1.00   HJS        20050422     NF3.0           CREATE
 *  Ver 2.00   SSW        20050522     NF3.0           redevelopment wrapper version
 *
 */
#ifndef _PFMNUMBER_N_H_
#define _PFMNUMBER_N_H_
/* --------------------------------------- include files ---------------------------------------- */
#include "pfmNumberType.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif
/* ---------------------------------- constant, macro definitions ------------------------------- */
#define PFMNUM_ZERO         g_pfmnum_zero
#define PFMNUM_ONE          g_pfmnum_one
/* ------------------------------------ structure definitions ----------------------------------- */
typedef     number_t    pfmnum_t;
typedef     number_t    PfmNumber;

/* --------------------------- exported global variables declarations --------------------------- */
extern const PfmNumber g_pfmnum_zero;
extern const PfmNumber g_pfmnum_one;
extern const PfmNumber g_pfmnum_e;      /* the base of natural logarithms : 2.7182818284590452354 */
/* ------------------------------- exported function declarations ------------------------------- */
long pfmNumAssign(PfmNumber *x, PfmNumber y) ;
long pfmNumToLong(long *x, PfmNumber y);
long pfmNumGetFromLong(PfmNumber *x, long y);
long pfmNumToDouble(double *x, PfmNumber y);
long pfmNumGetFromDouble(PfmNumber *x, double y);
long pfmNumGetFromLongDouble(PfmNumber *x, long double y);
long pfmNumGetFromDouble2(pfmnum_t *x, double y, long precision);
long pfmNumGetFromLongDouble2(pfmnum_t *x, long double y, long precision);
long pfmNumGetFromStr(PfmNumber *x, char *str);
long pfmNumGetFromStrn(PfmNumber *x, char *str, long len);
long pfmNumGetStrLPadZeroN(char *str, PfmNumber x, long buf_size);
long pfmNumInitAllowOutputTruncated( void );
long pfmNumSetAllowOutputTruncated( long flag );
long pfmNumToStrn(char *str, PfmNumber x, long buf_size);
long pfmNumToStr(char *str, PfmNumber x);
long pfmNumCmp(PfmNumber x, PfmNumber y);
long pfmNumCmpAbs(PfmNumber x, PfmNumber y) ;
long pfmNumIsInteger(PfmNumber x);
long pfmNumCmpLong(PfmNumber x, long y);
long pfmNumCmpStr(PfmNumber x, char *str);
long pfmNumCmpStrn(PfmNumber x, char *str, long len);
long pfmNumNegate(PfmNumber *v, PfmNumber x);
long pfmNumAbs(PfmNumber *v, PfmNumber x);
long pfmNumAdd(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumSub(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumMul(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumDiv(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumIntDiv(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumMod(PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumModDiv(PfmNumber *v, PfmNumber *mod, PfmNumber x, PfmNumber y);
long pfmNumTrunc(PfmNumber *v, PfmNumber x);
long pfmNumRound(PfmNumber *v, PfmNumber x);
long pfmNumSqr  (PfmNumber *v, PfmNumber x);
long pfmNumSqrt (PfmNumber *v, PfmNumber x);
long pfmNumPowerN (PfmNumber *v, PfmNumber x, PfmNumber y);
long pfmNumExpN   (PfmNumber *v, PfmNumber x);
long pfmNumLogEN (pfmnum_t *v, pfmnum_t x);
long pfmNumLog10N(pfmnum_t *v, pfmnum_t x);
long pfmNumLogN  (pfmnum_t *v, pfmnum_t x, pfmnum_t y);
long pfmNumInc(PfmNumber *x, PfmNumber y);
long pfmNumDec(PfmNumber *x, PfmNumber y);
long pfmNumAddStr(PfmNumber *v, PfmNumber x, char *str);
long pfmNumSubStr(PfmNumber *v, PfmNumber x, char *str);
long pfmNumMulStr(PfmNumber *v, PfmNumber x, char *str);
long pfmNumDivString(PfmNumber *v, PfmNumber x, char *str);
long pfmNumAddStrn(PfmNumber *v, PfmNumber x, char *str, long len);
long pfmNumSubStrn(PfmNumber *v, PfmNumber x, char *str, long len);
long pfmNumMulStrn(PfmNumber *v, PfmNumber x, char *str, long len);
long pfmNumDivStrn(PfmNumber *v, PfmNumber x, char *str, long len);
long pfmNumAddLong(PfmNumber *v, PfmNumber x, long y);
long pfmNumSubLong(PfmNumber *v, PfmNumber x, long y);
long pfmNumMulLong(PfmNumber *v, PfmNumber x, long y);
long pfmNumDivLong(PfmNumber *v, PfmNumber x, long y);
long pfmNumModDivLong(PfmNumber *v, PfmNumber *mod, PfmNumber x, long y);
long pfmNumAddNVaList(long cnt, pfmnum_t *v, pfmnum_t x, va_list *parg);
long pfmNumMulNVaList(long cnt, pfmnum_t *v, pfmnum_t x, va_list *parg);
long pfmNumAddN(long cnt, PfmNumber *v, PfmNumber x, ...);
long pfmNumMulN(long cnt, PfmNumber *v, PfmNumber x, ...);
long pfmNumPowLong(PfmNumber *v, PfmNumber x, long y);
long pfmNumRoundAt(PfmNumber *v, PfmNumber x, long pos);
long pfmNumTruncAt(PfmNumber *v, PfmNumber x, long pos);
long pfmNumRoundUp(PfmNumber *v, PfmNumber x, long pos);
long pfmNumCalcVaList(PfmNumber *num, char *fmt, va_list *p_args);
long pfmNumCalc(PfmNumber *num, char *fmt, ...);
long pfmNumCalcLongVaList(long *lval, char *fmt, va_list *p_args);
long pfmNumCalcLong(long *lval, char *fmt, ...);
long pfmNumToCommaStrN(char *str, PfmNumber x, long buf_size);
long pfmNumToInFmtStrN(char *str, long buf_size, char *frmt, long right_align_flag, long lpad_zero_flag, PfmNumber x);
long pfmNumGetIntLen( pfmnum_t x );
char * pfmNumGetErrorMsg(void);
char * pfmNumPrint(PfmNumber x);
char * pfmNumPrintComma(PfmNumber x);
long pfmNumFindLocationOfLeastSignificantDigit(PfmNumber x);

void pfmNumDump(char *buf, PfmNumber *x);

#ifdef __cplusplus
}
#endif

#endif /* _PFMNUMBER_N_H_ */
