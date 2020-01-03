/*
 * @file           spfmippr.c
 * @brief          Input PrePRocessing ����
 * @library
 *
 * @dep-header      pfmcom.h
 *                  pfmconst.h
 *                  pfmutil.h
 *                  pfmcbuf.h
 *                  pfmtpapi.h
 *                  pfmiofrmt.h
 *                  mpfmparm.h
 *                  mpfmerrmsg.h
 *                  pfmdbio.h
 *                  pfmcomm.h
 *                  pfminpt.h
 *                  mpfmilog.h
 *                  pdb_sel_pfm_tran.h
 *
 * @dep-module      mpfmilog
 *
 * @dep-table       pfm_tran
 *
 * @dep-infile
 * @dep-outfile
 *
 * @history
 *  ��    �� : ��  ��  :  ��  ��    :  �ٰ�  �ڷ�    :          ��       ��        ��       ��
 *  --------   ------     --------     -----------     ---------------------------------------------
 *   V1.00   : ������  :  20050316  : New Bank ����  :  �ű� ����
 *   V2.00   : �輼ȯ  :  20050501  : New Bank ����  :  ����߰�
 *   V4.00   : �輼ȯ  :  20050714  : New Bank ����  :  Configurable
 *   V5.00   : �ſ�ö  :  20050906  : New Bank ����  :
 *   V6.00   : �ſ�ö  :  20060606  : New Bank ����  :  e000->tx_info ��� ���� �� OPPR�� �б� ����
 *
 */
/*
 * @pcode
 *
 *  spfmippr. main
 *      a000. init / validation
 *      b000. set pfm_tran
 *      c000. commbuff ����
 *      d000. �ŷ� ����/����
 *      e000. image logging
 *      f000. ������� �缳��(������ Async)
 *      g000. �ŷ� �б�
 *
 *	a000. init / validation
 *      1. TP msg header �� msg length ���� ����
 *      2. ���� ���ۺ��� ���� ���� ����
 *      3. CommBuff �ʱ�ȭ
 *      4. g_pfm_arch �ʱ�ȭ
 *      5. ���������� -> PFMIHEAD ��ȯ & CommBuff setting
 *
 *  b000. set pfm_tran
 *      1. �ŷ� �Ķ���� loading & CommBuff setting
 *
 *  c000. commbuff ����
 *      1. ���������ͺ� -> PFMINPT, XXXINPT, XXXINPT_1 ��ȯ & CommBuff setting
 *      2. PFMCOMM CommBuff setting
 *          1) System Date
 *          2) System Time
 *          3) Service Name
 *          4) ���� Ƚ��
 *          5) Caller Service Name
 *          6) �ǿ��� ����
 *          7) å���� ���� �Ǽ�
 *      3. PFMILOG CommBuff setting
 *      4. �ý��۰���, ��������, ���������� ��ȯ
 *
 *  d000. �ŷ� ����/����
 *      1. TTL ����
 *
 *  e000. image logging
 *      1. �Է� ���� ������ image log�� insert (image log ��� ȣ��)
 *      2. a000 - c000 �߿� ���� �߻��� ���� �ڵ带 �����Ͽ� insert
 *
 *  f000. ������� �缳��(������ Async)
 *      1.
 *
 *  g000. �ŷ� �б�
 *      1. �ŷ� �Ķ������ ���� �̸����� tpforward
 *
 */
/* --------------------------------------- include files ---------------------------------------- */
#include <pfmcom.h>
#include <pfmcomm.h>
#include <pfmerr.h>
#include <pfmutil.h>
#include <pfmcbuf.h>
#include <pfmtpapi.h>
#include <pfmiofrmt.h>
#include <pfmioframe.h>
#include <pfmsframe.h>
#include <pfmlframe.h>
#include <pfmdbio.h>
#include <pfmxframe.h>
#include <mpfmltran.h>
#include <mpfmchdr.h>
#include <mpfmthdr.h>
#include <mpfmvldmsg.h>
#include <mpfmvldpfmihead.h>
#include <mpfmcvtstdindata.h>
#include <mpfmdefstdhdr.h>
#include <mpfmtxcntl.h>
#include <mpfmimglog.h>
#include <pfmfault.h>

/* ---------------------------------- constant, macro definitions ------------------------------- */
/* Oracle : -1003  : no statement parsed
 * Oracle : -1012  : not logged on
 * Oracle : -3113  : end-of-file on communication channel
 * Oracle : -3114  : not connected to ORACLE
 * Oracle : -24909 : call in progress. Current operation cancelled
*/
#define CHK_RESTART_FLAG(ecode) do {            \
        if ( (ecode == -1003)                   \
            || (ecode == -1012)                 \
            || (ecode == -3113)                 \
            || (ecode == -3114)                 \
            || (ecode == -24909) ) {            \
            g_pfm_arch.restart_flag = 1;        \
            PFM_DSP("IPPR Reboot Flag Set....[%ld]", g_pfm_arch.restart_flag);  \
        }                                       \
} while (0)
/* ------------------------------------ structure definitions ----------------------------------- */
typedef struct spfmippr_ctx_s spfmippr_ctx_t;
struct spfmippr_ctx_s {
    char svc_name[MAX_LEN_SVC_NAME + 1];    /* IPPR ���� ��                  */
    long pfmihead_flag;                     /* PFMIHEAD setting ����           */
    long ilog_flag;                         /* �̹��� �α� ����                */
    long timer_flag;                        /* TimerDB read ����               */

    char global_id[LEN_GLOBAL_ID + 1];      /* GLOBAL_ID                       */
    char tx_code[LEN_SFG_TX_CODE + 1];      /* TX_CODE                         */
    char inst_no[LEN_INST_NO + 1];          /* INST_NO                         */
    long snd_infc_g;                        /* snd_infc_g                      */

    long mytxflag;                          /* not used                        */
    TPSVCINFO   *msg;                       /*                                 */
    TXINFO      txstat;                     /* not used                        */

};
/* -------------------------------------- global variables -------------------------------------- */
/* ------------------------------------ function prototypes ------------------------------------- */
static long a000_init_proc              (spfmippr_ctx_t *ctx, TPSVCINFO *msg);
static long b000_set_commbuff           (spfmippr_ctx_t *ctx);
static long c000_tx_validation          (spfmippr_ctx_t *ctx);
static long d000_image_logging          (spfmippr_ctx_t *ctx);
static long e000_reset_header           (spfmippr_ctx_t *ctx);
static long f000_forward_svc            (spfmippr_ctx_t *ctx);
static long z999_err_exit_proc          (spfmippr_ctx_t *ctx);

/* --------------------------------------- function body ---------------------------------------- */
int
tpsvrinit(int argc, char *argv[])
{
    long rc = RC_NRM;

	bzero(&g_svr_ctx, sizeof(svr_ctx_t));

    /* NonXA */
	g_svr_ctx.db_flag = 1;
	g_svr_ctx.xa_flag = 0;

	rc = mpfm_tpsvrinit();
	if( rc != RC_NRM ) {
		PFM_DBG("failed mpfm_tpsvrinit ...");
		return -1;
	}

	return 0;
}
/* --------------------------------------- function body ---------------------------------------- */
int
tpsvrdone(void)
{
    long rc = RC_NRM;

    /* NonXA */
	rc = mpfm_tpsvrdone();
	if( rc != RC_NRM ) {
		PFM_DBG("failed mpfm_tpsvrdone ...");
		return -1;
	}

	return 0;
}
/* --------------------------------------- function body ---------------------------------------- */
void
tpsvctimeout(TPSVCINFO *msg)
{
    mpfm_tpsvctimeout(msg);
}
/* --------------------------------------- function body ---------------------------------------- */
void
SPFMIPPR(TPSVCINFO *msg)
{
    long rc = RC_NRM;

    spfmippr_ctx_t  s_ctx;
    spfmippr_ctx_t  *ctx = &s_ctx;

    /* arch �ʱ�ȭ */
    mpfm_pgm_init(PT_ONLINE);
    mpfm_mem_log_dump_init();

    /* 20060518 - analyzer log ������ ���� */
    g_pfm_process.log_analyzer_yn = 0;

    /* --- AMS LOG --- */
    PFM_TRACE_PGM_S("=-=-=-=-=- START OF SPFMIPPR (Before setting Global_ID)=-=-=-=-=- ");
    /* --- AMS LOG --- */

    /* �ʱ�ȭ �� �������� */
    PFM_TRY(a000_init_proc(ctx, msg));
    mpfm_set_proc_step("a000 - �ʱ�ȭ & �������� �Ϸ�");

    /* commbuff ���� */
    PFM_TRY(b000_set_commbuff(ctx));
    mpfm_set_proc_step("b000 - Commbuff SET �Ϸ�");

    /* for sysmaster */
    if (g_pfm_ams.ams_use_flag == 2 && PFM_TRAN->ams_lging_g > 0) {
        mpfm_tpgetorgclh(&g_pfm_ams.clid, (int)*(ctx->msg->cltid.clientdata));
        PFMCOMM->caller_spr_num  = g_pfm_ams.spr_num;
        mpfm_ams_set_trace_level(1, atol(PFM_TRAN->upmu_use_list_nm10) );
    }
    else {
        mpfm_ams_set_trace_level(PFM_TRAN->ams_lging_g, atol(PFM_TRAN->upmu_use_list_nm10) );
    }

    /* --- AMS LOG --- */ /* ȭ��ǽ� ���� */
    if( PFMIHEAD != NULL && PFM_TRAN != NULL && PFMIHEAD->scr_prctc_mode[0] != '1' ) {
        mpfm_ams_log_svc( LOG_TYPE_SVC_S,
                          __FILE__, __func__, __LINE__,
                          PFM_TRAN->trx_c,
                          PFMIHEAD->chan_u, PFMIHEAD->trxbrno, PFMIHEAD->trx_trmno);
    }
    else {
        PFM_DBG("PFMIHEAD == NULL OR PFM_TRAN == NULL");
    }
    /* --- AMS LOG --- */

    /* �ŷ�����(���հŷ�����) ��� */
    PFM_TRY(c000_tx_validation(ctx));
    mpfm_set_proc_step("c000 - �ŷ����� �Ϸ�");

    /* �̹��� �α� */
    PFM_TRY(d000_image_logging(ctx));
    mpfm_set_proc_step("d000 - �̹����α� �Ϸ�");

    /* PFMIHEAD with the Timer DB the Reset */
    PFM_TRY(e000_reset_header(ctx));
    mpfm_set_proc_step("e000 - TimerDB Select �Ϸ�");

    /* --- AMS LOG --- */ /* ȭ��ǽ� ���� */
    if( PFMIHEAD != NULL && PFM_TRAN != NULL && PFMIHEAD->scr_prctc_mode[0] != '1' ) {
        mpfm_ams_log_svc( LOG_TYPE_SVC_E,
                          __FILE__, __func__, __LINE__,
                          PFM_TRAN->trx_c,
                          PFMIHEAD->chan_u, PFMIHEAD->trxbrno, PFMIHEAD->trx_trmno);
    }
    else {
        PFM_DBG("PFMIHEAD == NULL OR PFM_TRAN == NULL");
    }

    /* �ŷ��б� */
    PFM_TRY(f000_forward_svc(ctx));
    mpfm_set_proc_step("f000 - forward �Ϸ�");

PFM_CATCH:

    mpfm_set_proc_step("z999 - IPPR ����ó�� ����");

    z999_err_exit_proc(ctx);

    mpfm_set_proc_step("z999 - IPPR ����ó�� �Ϸ�");
}

/* --------------------------------------- function body ---------------------------------------- */
/*
 * �ʱ�ȭ & �������� 
 */
static long
a000_init_proc(spfmippr_ctx_t *ctx, TPSVCINFO *msg)
{
    long rc = RC_NRM;
    long snd_node_ser;

    /* context �ʱ�ȭ */
    memset(ctx, 0x00, sizeof(spfmippr_ctx_t));

    /* initialize TPSVCINFO */
    ctx->msg      = msg;
    /* �Է� ������ ���� */
    PFM_DBG( "*************** �ܸ� �Է� ������[%ld] **************", ctx->msg->len );
    PFM_HEXDUMP(ctx->msg->data, ctx->msg->len);
    PFM_DBG( "*************** �ܸ� �Է� ������[%ld] **************", ctx->msg->len );

    strncpy(ctx->svc_name, ctx->msg->name, MAX_LEN_SVC_NAME);
    /* 20060518 - arch ���� ���� */
    /* set arch svc_name */
    mpfm_set_pgm_name(ctx->svc_name);

    /* get global_id from msg*/
    rc = mpfm_get_global_id(&g_pfm_cfg.icfg, (char *)ctx->msg->data, ctx->global_id);
    if (rc != RC_NRM) {
        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5149, "�Է����� ����!! �Է��������� global id ���ϴ��� ���� �߻� �߽��ϴ�");
        return RC_ERR;
    }
    PFM_DBG("GLOB_ID from TPSVCINFO msg: [%s]", ctx->global_id);

    /* set arch global_id */
    strncpy(g_pfm_arch.global_id, ctx->global_id, sizeof(g_pfm_arch.global_id)-1 );

    PFM_DSP("=-=-=-=-=- START OF SPFMIPPR (After setting Global_ID)=-=-=-=-=- ");

    /* get tx_code from msg*/
    rc = mpfm_get_tx_code(&g_pfm_cfg.icfg, (char *)ctx->msg->data, ctx->tx_code);
    if (rc != RC_NRM) {
        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5150, "�Է����� ����!! �Է��������� �ŷ��ڵ� ���ϴ��� ���� �߻� �߽��ϴ�");
        return RC_ERR;
    }
    PFM_DBG("TX_CODE from TPSVCINFO msg: [%s]", ctx->tx_code);

    /* get inst_no from msg*/
    rc = mpfm_get_inst_no(&g_pfm_cfg.icfg, (char *)ctx->msg->data, ctx->inst_no);
    if (rc != RC_NRM) {
        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5151, "�Է����� ����!! �Է��������� �׷���ڵ� ���ϴ��� ���� �߻� �߽��ϴ�");
        return RC_ERR;
    }
    PFM_DBG("INST_NO from TPSVCINFO msg: [%s]", ctx->inst_no);

    /* get snd_infc_g from msg*/
    rc = mpfm_get_snd_infc_g(&g_pfm_cfg.icfg, (char *)ctx->msg->data, &ctx->snd_infc_g);
    if (rc != RC_NRM) {
        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5152, "�Է����� ����!! �Է��������� �۽��������̽� ���ϴ��� ���� �߻� �߽��ϴ�");
        return RC_ERR;
    }

    /* ���� �⺻ ���� */
    PFM_TRY( mpfmvldmsg(msg) );

    return RC_NRM;

PFM_CATCH:
    return rc;
}


/* --------------------------------------- function body ---------------------------------------- */
/*
 * CommBuff ���� - PFMIHEAD, PFMCOMM, PFM_TRAN, PFMILOG, PFMINPT
 */
static long
b000_set_commbuff(spfmippr_ctx_t *ctx)
{
    long rc = RC_NRM;
    pdb_pfm_tran_t  pfm_tran;

    /* set PFMIHEAD */
    PFM_TRY( mpfmchdr(ctx->msg) );
    ctx->pfmihead_flag = 1;

    /* set service PFMCOMM */
    PFM_TRY( mpfm_set_pfmcomm(ctx->svc_name, 0) );

    PFMCOMM->resp_snd_infc_g = ctx->snd_infc_g;
    PFM_DBG("SND_INFC_G from TPSVCINFO msg: [%ld]", ctx->snd_infc_g);

    /* save original gid */
    if( PFMIHEAD->rqst_resp_g[0] == 'S'){
        strncpy(PFMCOMM->orgn_glob_id, PFMIHEAD->glob_id, LEN_GLOB_ID);
    }    

    strncpy(PFMCOMM->original_gid, PFMIHEAD->glob_id, LEN_PFMIHEAD_GLOB_ID);
        
    /* validate custom header */
    PFM_TRY( mpfmvldpfmihead(PFMIHEAD, ctx->msg) );

    /* call pfm_tran setup-module */
    bzero(&pfm_tran, sizeof(pdb_pfm_tran_t));

    memcpy(pfm_tran.grpco_c, ctx->inst_no, strlen(ctx->inst_no));
    memcpy(pfm_tran.trx_c,   ctx->tx_code, strlen(ctx->tx_code));
    PFM_TRY( mpfmltran(&pfm_tran) );

    /* set PFMILOG */
    PFM_TRY( mpfm_set_item(IDX_PFMILOG, ctx->msg->data, ctx->msg->len) );

    /* convert message body */
    PFM_TRY( mpfmcvtstdindata(PFMILOG, 1) );

    return RC_NRM;

PFM_CATCH:
    CHK_RESTART_FLAG(pdb_errno());
    return rc;
}

/* --------------------------------------- function body ---------------------------------------- */
/*
 * �ŷ�����(���հŷ�����) ���
 */
static long
c000_tx_validation(spfmippr_ctx_t *ctx)
{
    long rc = RC_NRM;

    /* �ŷ�����(���հŷ�����) ��� */
    PFM_TRY( mpfmtxcntl(ctx->msg) );

    /* ���հŷ������� ȭ���Է½ǽ� */
    if(PFMCOMM->trm_prctc_skip_flag == FALSE) {
        if(PFMIHEAD != NULL && PFMIHEAD->scr_prctc_mode[0] == '1') {
            mpfm_tpforward2("SCPM9901F", ctx->msg->data, ctx->msg->len, 0);
        }
    }
    else {
        if(PFMIHEAD != NULL && PFMIHEAD->scr_prctc_mode[0] == '1') {
            /* ���� �ǽ��ŷ��� ������ �����Դϴ� */
            PFM_ERR("PFM30002", NULL);
            return RC_ERR;
        }
    }

    return RC_NRM;

PFM_CATCH:
    return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
/*
 * �̹��� �α�
 */
static long
d000_image_logging(spfmippr_ctx_t *ctx)
{
    long rc = RC_NRM;
    long _rc = RC_NRM;

    if( strncmp(ctx->tx_code, "HEALTHCHK000", LEN_SFG_TX_CODE) == 0) {
        PFM_DBG("HEALTHCHK ���� �̹��� �α� ���� [�ŷ��ڵ�:%s]", ctx->tx_code);
        return RC_NRM;
    }

    if( PFMIHEAD->scr_prctc_mode[0] == '1' ) {
        PFM_DSP("ȭ���Է½ǽ��ŷ� �̹��� �α� ���� [�ŷ��ڵ�:%s]", ctx->tx_code);
        return RC_NRM;
    }

    if( PFMILOG == NULL ) {
        PFM_DBG("msg->len: [%ld], msg->data: [%p]", ctx->msg->len, ctx->msg->data);
        PFM_TRY( mpfm_set_item(IDX_PFMILOG, ctx->msg->data, ctx->msg->len) );
    }

    if( PFMCOMM->ilog_skip_flag == TRUE ) {
        PFM_DSP("���հŷ�����->�̹����α� SKIP [���հŷ�����:%ld][�ŷ��ڵ�:%s]", 
                 PFMCOMM->ilog_skip_flag, ctx->tx_code);
        return RC_NRM;
    }

    /* mpfmimglog call */
    mpfmimglog_ctx_t   mpfmimglog_ctx;
    bzero(&mpfmimglog_ctx, sizeof(mpfmimglog_ctx_t));

    rc = mpfmimglog(&mpfmimglog_ctx);
    if( rc == RC_NRM || mpfmimglog_ctx.ilog_flag == 1 ) { 
        PFM_DBG("�̹����α� �ߺ� ���� [%ld]", mpfmimglog_ctx.ilog_flag);
        _rc = mpfmdbio_commit();
        if( _rc == RC_NRM ) {
            ctx->ilog_flag = 1;
        }
        else {
            PFM_DSP("IPPR commit ó���� ���� �߻� !!! [%ld]", _rc);
            goto PFM_CATCH;
        }
    }
    else {
        PFM_DBG("�̹����α� �ߺ� ���� [%ld]", mpfmimglog_ctx.ilog_flag);
        _rc = mpfmdbio_rollback();
        if( _rc != RC_NRM ) {
            PFM_DSP("IPPR rollback ó���� ���� �߻� !!! [%ld]", _rc);
        }
        goto PFM_CATCH;
    }

    PFM_DBG("�̹��� �α� process successfully completed");

    /* 20060812 - 0811�� ȸ�� : dup glob_id �߻��� �ű�ä���� gid�� �̹����α��ϰ�, 
                  ��gid + 1 �� ������� �����Ѵ� */
    if( rc != RC_NRM ) {
        strncpy(PFMIHEAD->glob_id, ctx->global_id, LEN_GLOBAL_ID);
    }
    /* RC_NRM OR RC_ERR(ilog dup error) */
    return rc;

PFM_CATCH:
    PFM_DBG("�̹��� �α� ����: [%ld]", rc);

    CHK_RESTART_FLAG(pdb_errno());

    PFM_DSP("Last SQL: [%s]", mpfmdbio_lastsql());
    mpfm_info_cb();

    return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
/*
 * ������ Async �� ��� PFMIHEAD with the Timer DB the Reset
 */
static long
e000_reset_header(spfmippr_ctx_t *ctx)
{

    long rc = RC_NRM;

    mpfmthdr_io_t mpfmthdr_io;
    
    bzero(&mpfmthdr_io, sizeof(mpfmthdr_io_t));
    
    if( PFMIHEAD->rqst_resp_g[0] == 'R' && PFMIHEAD->sync_g[0] == 'A' ) {        
        PFM_TRY( mpfmthdr(&mpfmthdr_io) );
    }
    
    if( mpfm_isspnull( PFMCOMM->timer_recv_trx_c ) == FALSE ){
    	ctx->timer_flag = TRUE;
	}
	
    return RC_NRM;

PFM_CATCH:
    CHK_RESTART_FLAG(pdb_errno());
    return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
/*
 * �ŷ� �Ķ���Ϳ� ��ϵ� ���񽺷� tpforward() ����
 */
static long
f000_forward_svc(spfmippr_ctx_t *ctx)
{
    long rc = RC_NRM;

    pfm_dummy_info_t pfm_dummy_info;
    
    bzero(&pfm_dummy_info, sizeof(pfm_dummy_info_t));
  
    strncpy(PFMCOMM->caller_svc_name, ctx->svc_name, MAX_LEN_SVC_NAME);
 
    PFM_TRY( mpfm_get_dummy_svc_nm(&pfm_dummy_info));
   
    /* --- AMS LOG --- */
    PFM_TRACE_PGM_E("=-=-=-=-=- NORMAL END OF SPFMIPPR [Forwarding to (%s)]-=-=-=-=- ", pfm_dummy_info.svc_nm);
    /* --- AMS LOG --- */
 
    PFM_TRY( mpfm_tpforward(mpfm_trim(pfm_dummy_info.svc_nm), 0) );


#if 0
    PFM_DSP("Forwarding to [%s]", PFM_TRAN->svc_nm);
    strncpy(PFMCOMM->caller_svc_name, ctx->svc_name, MAX_LEN_SVC_NAME);
    PFM_TRY( mpfm_tpforward(mpfm_trim(PFM_TRAN->svc_nm), 0) );
#endif

    return RC_NRM;

PFM_CATCH:
    return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
static long
z999_err_exit_proc(spfmippr_ctx_t *ctx)
{
    long rc = RC_NRM;

    long i = 0;
    long cd = 0;
    long defhead_flag = 0;
    char *sndbuf = NULL;
    long sndlen = 0;

    char src_sys_id[LEN_SYS_ID + 1];
    long src_node_ser;
    char target_sys_id[LEN_SYS_ID + 1];
    long target_node_ser_in;
    long target_node_ser_out;
    char svc_name[MAX_LEN_SVC_NAME + 1];
    bzero(src_sys_id, sizeof(src_sys_id));
    bzero(target_sys_id, sizeof(target_sys_id));
    bzero(svc_name, sizeof(svc_name));

    /* ����ó���� �����߻��� return RC_ERR �ʿ�ġ ���� */
    /* �̹��� �α� ���� ������ ��쿡�� �̹��� �α� �� tpreturn */
    if (ctx->ilog_flag == FALSE) {
        if( !ctx->pfmihead_flag ) {
            PFM_DBG("PFMIHEAD ���� �� ���� �߻� : [%s]", mpfm_err_code());
            rc = mpfmdefstdhdr(ctx->msg, mpfm_err_code());
            if( rc != RC_NRM ) {
                PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5004, "SPFMIPPR ����Ʈ ��� ������ ���� �߻� �߽��ϴ� [%s] [%s]",
                          mpfm_err_code(), mpfm_err_msg());
                goto output_proc;
            }
            ctx->pfmihead_flag = TRUE;
            defhead_flag = TRUE;
        }

        /* set default pfmcomm if null */
        if( PFMCOMM == NULL ) {
            PFM_TRY( mpfm_set_pfmcomm("SPFMIPPR", 1) );
        }

        rc = d000_image_logging(ctx);
        if( rc != RC_NRM && ctx->ilog_flag != 1 ) {
            PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5005, "SPFMIPPR���� �̹��� �α� ���� �߽��ϴ�");
        }
    }

output_proc:
    /* �����޽��� ���� */
    PFM_DBG("err_code: [%s], err_msg: [%s]", mpfm_err_code(), mpfm_err_msg());

    PFM_SET_DEFAULT_ERR();

    if( defhead_flag ) {
        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5140, "������� ��ȯ ���� ���� �߻��Ͽ� ����Ʈ ����� ����մϴ�");
    }

    /* �������� ���� */
    if( PFMIHEAD != NULL && PFMCOMM != NULL ) {
        /* gid + 1 */
        rc = mpfm_stdout_msg(1);
        if (rc != RC_NRM) {
            PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5139, "SPFMIPPR �������� ������ ���� �߻� �߽��ϴ�.");
        }
    }

    /* --- AMS LOG --- */ /* ȭ��ǽ� ���� */
    if( PFMIHEAD != NULL && PFM_TRAN != NULL && PFMIHEAD->scr_prctc_mode[0] != '1' ) {
        mpfm_ams_log_svc( LOG_TYPE_SVC_E,
                          __FILE__, __func__, __LINE__,
                          PFM_TRAN->trx_c,
                          PFMIHEAD->chan_u, PFMIHEAD->trxbrno, PFMIHEAD->trx_trmno);
    }
    else {
        PFM_DBG("PFMIHEAD == NULL OR PFM_TRAN == NULL");
    }
    PFM_TRACE_PGM_E("=-=-=-=-=- ABNORMAL END OF SPFMIPPR =-=-=-=-=- ");
    /* --- AMS LOG --- */

    /* ------------------------------------------------------------------------ */
    /* 1. Async                                                                 */
    /*    - ��û���䱸��==��û(S)                                               */
    /*    - Async�Ӽ�==�����(1)                                    : tpacall   */
    /* 2. Sync, Async-�ܹ���||����, ���� parsing ����                           */
    /*    - Sync                                                    : clireturn */
    /*    - Async�Ӽ�==�ܹ���(0, space) / ��û���䱸��(R)           : clireturn */
    /*    - �Է����� parsing ����                                   : clireturn */
    /* ------------------------------------------------------------------------ */

    if( PFMIHEAD != NULL && PFMCOMM != NULL && PFMOUTQ != NULL ) {
   		
   		/* 1. Async                                                                 */
        /*    - ��û���䱸��==��û(S)                                               */
        /*    - Async�Ӽ�==�����(1)                                    : tpacall   */
        if( PFMIHEAD->rqst_resp_g[0] == 'S' && 
            PFMIHEAD->sync_g[0] == 'A' && PFMIHEAD->async_atrb[0] == '1' ) {
     
            sndlen = mpfm_get_item_size(IDX_PFMOUTQ);
            sndbuf = mpfm_tpalloc("CARRAY", sndlen);
            if( sndbuf == NULL ) {
                PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5002, "SPFMIPPR���� tpalloc ���� �߽��ϴ�");
            }
 
            memcpy(sndbuf, PFMOUTQ, sndlen);
 
            PFM_HEXDUMP(sndbuf, sndlen);
 
            /* ä��MCA, EAI, ��ܰ� - EAI load balancing ���� ���� */
            if( PFMIHEAD->snd_infc_g == 1 || PFMIHEAD->snd_infc_g == 6 || PFMIHEAD->snd_infc_g == 2 || /* EA2 */
                PFMIHEAD->snd_infc_g == 3 || PFMIHEAD->snd_infc_g == 4) {
                rc = pfm_get_node_info(src_sys_id, &src_node_ser);
                if (rc == RC_NRM) {
                    if (PFMIHEAD->snd_infc_g == 1 || 
                        PFMIHEAD->snd_infc_g == 3 ||
                        PFMIHEAD->snd_infc_g == 4) {
                        strncpy(target_sys_id, PFMIHEAD->snd_node_ctnt, LEN_SYS_ID);
                    }
                    else if (PFMIHEAD->snd_infc_g == 6) {  /* EA2 */
                        strncpy(target_sys_id, SYS_ID_EA2, LEN_SYS_ID - 1);
                    }
                    else if (PFMIHEAD->snd_infc_g == 2) {
                        strncpy(target_sys_id, SYS_ID_EAI, LEN_SYS_ID - 1);
                    }
                    else {
                        PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5119, "ó�� �Ұ��� �۽��������̽�����[%ld] �Դϴ�", PFMIHEAD->snd_infc_g);
                    }
  
                    if ( (PFMIHEAD->snd_infc_g != 6 && PFMIHEAD->snd_infc_g != 2) && /* EA2 */
                         (PFMCOMM->snd_node_ser > 0 && PFMCOMM->snd_node_ser <= 9) ) {
                        target_node_ser_in = PFMCOMM->snd_node_ser;
                    }
                    else {
                        target_node_ser_in = 1;
                    }
  
                    rc = pfm_load_balance(src_sys_id, src_node_ser, target_sys_id, target_node_ser_in, &target_node_ser_out, svc_name);
                    if (rc != RC_NRM) {
                        PFM_DBG("target system�� pfm_obst ���̺� ��ϵ��� �ʾҰų�, ������ ��带 ã�� �� �����ϴ�");
                    }
                    else {
                        PFM_DBG("src_sys_id          [%s]", src_sys_id);
                        PFM_DBG("src_node_ser        [%ld]", src_node_ser);
                        PFM_DBG("target_sys_id       [%s]", target_sys_id);
                        PFM_DBG("target_node_ser_in  [%ld]", target_node_ser_in);
                        PFM_DBG("target_node_ser_out [%ld]", target_node_ser_out);
                        PFM_DBG("svc_name            [%s]", svc_name);
  
                        rc = mpfm_tpacall2(&cd, svc_name, sndbuf, sndlen, TPNOTRAN|TPBLOCK|TPNOREPLY);
                        if (rc != RC_NRM) {
                            PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5142, "SPFMIPPR tpacall2 ������ ���� �߻� �߽��ϴ�.");
                        }
                    }
                }
                else {
                    PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5191, "ȣ��Ʈ�̸��� [%s] ��带 ���ϴµ� ���� �߽��ϴ�", g_pfm_process.hostname);
                }
            }
            else if (PFMIHEAD->snd_infc_g == 5 || PFMIHEAD->snd_infc_g == 0) {
                PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5192, "���������� �ھ�[�۽��������̽�����(%ld)]�� �������� �ʽ��ϴ�.", PFMIHEAD->snd_infc_g);
            }
            else {
                PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5119, "ó�� �Ұ��� �۽��������̽�����[%ld] �Դϴ�", PFMIHEAD->snd_infc_g);
            }
 
            if(sndbuf) {
                mpfm_tpfree(sndbuf);
            }
 
            if(g_pfm_arch.restart_flag == 1){
                PFM_DBG("=- CLIRETURN FAIL RESTART -=");
                mpfm_clireturn(TPEXIT);
            }
            else {
                mpfm_clireturn(TPFAIL);
            }
 
        }
        /* 2. Sync, Async, ���� parsing ����                                        */
        /*    - Sync                                                    : clireturn */
        /*    - Async�Ӽ�==�ܹ���(0, space) / ��û���䱸��(R)           : clireturn */
        /*    - �Է����� parsing ����                                   : clireturn */
        else {
            if( PFMIHEAD->rqst_resp_g[0] == 'R' && PFMIHEAD->sync_g[0] == 'A' ) {
                PFM_FAULT(ERR_PF, PFM_LNS, PFM_E5195, "Async-�������� �����߻��� �������� ���� ���� �ʽ��ϴ�"); 
                for (i = 0; i < g_pfm_err.err_msg_cnt ; i++) {
                    PFM_DSP("Async-�������� �������� : [%ld-%s]", i, g_pfm_err.err_msg[i]);
                }
            }

            if(g_pfm_arch.restart_flag == 1) {
                PFM_DBG("=- CLIRETURN FAIL RESTART -=");
                mpfm_clireturn(TPEXIT);
            }
            else {
                mpfm_clireturn(TPFAIL);
            }
        }
    }
    else {
        if(g_pfm_arch.restart_flag == 1) {
            PFM_DBG("=- CLIRETURN FAIL RESTART -=");
            mpfm_clireturn(TPEXIT);
        }
        else {
            mpfm_clireturn(TPFAIL);
        }
    }

    return RC_NRM;

PFM_CATCH:

    return rc;
}
/* --------------------------------------- function body ---------------------------------------- */
