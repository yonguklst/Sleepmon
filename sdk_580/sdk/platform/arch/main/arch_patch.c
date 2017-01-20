/**
 ****************************************************************************************
 *
 * @file arch_patch.c
 *
 * @brief ROM code patches.
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/*
 * INCLUDES
 ****************************************************************************************
 */
#include "arch.h"
#include <stdlib.h>
#include <stddef.h>     // standard definitions
#include <stdint.h>     // standard integer definition
#include <stdbool.h>    // boolean definition
#include "ke_task.h"
#include "gtl_env.h"
#include "gtl_task.h"
#include "gapc_task.h"
#include "l2cc_pdu.h"

/**
 * @addtogroup PATCHES
 * @{
 */

/*
 * DEFINES
 ****************************************************************************************
 */

#if (LOG_MEM_USAGE)
    #if (!DEVELOPMENT_DEBUG)
        #error "LOG_MEM_USAGE must not be set when building for production (DEVELOPMENT_DEBUG is 0)"
    #else
        #warning "LOG_MEM_USAGE is set! Some patches may be disabled when this option is selected..."
    #endif
#endif

/*
 * FORWARD DECLARATIONS OF EXTERNAL FUNCTIONS
 ****************************************************************************************
 */

#if (LOG_MEM_USAGE)
void *log_ke_malloc(uint32_t size, uint8_t type);
void log_ke_free(void* mem_ptr);
#endif


#ifdef __DA14581__
struct l2cc_att_rd_req;
uint8_t atts_read_resp_patch(uint8_t conidx, struct l2cc_att_rd_req* req);

struct advertising_pdu_params;
struct co_buf_tx_node;
struct lld_evt_tag *lld_adv_start_patch(struct advertising_pdu_params *adv_par,
                                        struct co_buf_tx_node *adv_pdu,
                                        struct co_buf_tx_node *scan_rsp_pdu,
                                        uint8_t adv_pwr);

#if (!BLE_HOST_PRESENT)
int llm_rd_local_supp_feats_cmd_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id);
#endif //BLE_HOST_PRESENT


struct smpc_pairing_cfm;
int PATCHED_581_smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                     struct smpc_pairing_cfm *param,
                                     ke_task_id_t const dest_id, ke_task_id_t const src_id);
                                     
extern bool my_smpc_check_pairing_feat(struct gapc_pairing *pair_feat);
extern uint8_t my_smpc_check_param(struct l2cc_pdu *pdu);
#else /* DA14580 */

extern void smpc_send_pairing_req_ind(uint8_t conidx, uint8_t req_type);

struct gapc_pairing;
extern bool smpc_check_pairing_feat(struct gapc_pairing *pair_feat);

struct l2cc_pdu_recv_ind;
extern int l2cc_pdu_recv_ind_handler(ke_msg_id_t const msgid, struct l2cc_pdu_recv_ind *param,
                                        ke_task_id_t const dest_id, ke_task_id_t const src_id);
struct smpc_pairing_cfm;
extern int smpc_pairing_cfm_handler(ke_msg_id_t const msgid,
                                    struct smpc_pairing_cfm *param,
                                    ke_task_id_t const dest_id, ke_task_id_t const src_id);
 
struct llcp_con_up_req;
extern void my_llc_con_update_req_ind(uint16_t conhdl, struct llcp_con_up_req const *param);

struct llcp_channel_map_req;
extern void my_llc_ch_map_req_ind (uint16_t conhdl, struct llcp_channel_map_req const *param);

struct gapm_start_advertise_cmd;
extern uint8_t patched_gapm_adv_op_sanity(struct gapm_start_advertise_cmd *adv);

extern uint8_t my_smpc_check_param(struct l2cc_pdu *pdu);
#endif // __DA14581__

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

//The linker removes this if LOG_MEM_USAGE is 0
struct mem_usage_log mem_log[KE_MEM_BLOCK_MAX] __attribute__((section("retention_mem_area0"), zero_init));

#ifdef __DA14581__
uint32_t arch_adv_int __attribute__((section("retention_mem_area0"), zero_init));
#endif


/*
 * LOCAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#include "gtl.h"
#include "gapm_task.h"

/// KE TASK element structure
struct ke_task_elem
{
    uint8_t   type;
    struct ke_task_desc * p_desc;
};

/// KE TASK environment structure
struct ke_task_env_tag
{
    uint8_t task_cnt;
    struct ke_task_elem task_list[];
};
extern volatile struct ke_task_env_tag ke_task_env;
///////////////////////////////////
//patch LLM task
#include "llm.h"
#include "llm_task.h"
#include "llm_util.h"
#include "lld.h"
#include "reg_ble_em_rx.h"

int TASK_LLM_lld_data_ind_handler(ke_msg_id_t const msgid,
                     struct lld_data_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id);
static int
my_lld_data_ind_handler(ke_msg_id_t const msgid,
                     struct lld_data_ind const *param,
                     ke_task_id_t const dest_id,
                     ke_task_id_t const src_id);

extern struct llm_env_tag* llm_env[];
extern const struct ke_msg_handler llm_default_state[];
extern const struct ke_msg_handler llm_stopping[];

extern int TASK_LLM_llm_le_set_evt_msk_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rd_buff_size_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rd_loc_supp_feats_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_rand_add_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_adv_param_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rd_adv_ch_tx_pw_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_adv_data_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_adv_en_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_scan_rsp_data_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_scan_param_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_scan_en_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_create_con_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_create_con_cancel_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_host_ch_class_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_chnl_assess_timer_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rd_wl_size_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_wl_clr_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_wl_dev_add_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_wl_dev_rem_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_enc_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rand_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_rd_supp_states_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_test_rx_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_test_tx_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_test_end_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_le_set_host_ch_class_cmd_sto_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_reset_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_rd_bd_addr_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_rd_local_ver_info_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_rd_local_supp_cmds_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_set_cntl_host_flow_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_rd_local_supp_feats_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_set_evt_msk_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_host_buff_size_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_host_nb_comp_pkts_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_rd_buff_size_cmd_handler(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);
extern int TASK_LLM_llm_disconnect_mgt(ke_msg_id_t const msgid, void *param, ke_task_id_t const dest_id, ke_task_id_t const src_id);

/// Specifies the default message handlers of LLM task
static const struct ke_msg_handler my_llm_default_state[38] =
{
    /// low energy commands
    {LLM_LE_SET_EVT_MASK_CMD,        (ke_msg_func_t)TASK_LLM_llm_le_set_evt_msk_cmd_handler},
    {LLM_LE_RD_BUFF_SIZE_CMD,        (ke_msg_func_t)TASK_LLM_llm_le_rd_buff_size_cmd_handler},
    {LLM_LE_RD_LOCAL_SUPP_FEATS_CMD, (ke_msg_func_t)TASK_LLM_llm_le_rd_loc_supp_feats_cmd_handler},
    {LLM_LE_SET_RAND_ADDR_CMD,       (ke_msg_func_t)TASK_LLM_llm_le_set_rand_add_cmd_handler},

    #if (BLE_BROADCASTER || BLE_PERIPHERAL)
    {LLM_LE_SET_ADV_PARAM_CMD,     (ke_msg_func_t)TASK_LLM_llm_le_set_adv_param_cmd_handler},
    {LLM_LE_RD_ADV_CHNL_TX_PW_CMD, (ke_msg_func_t)TASK_LLM_llm_le_rd_adv_ch_tx_pw_cmd_handler},
    {LLM_LE_SET_ADV_DATA_CMD,      (ke_msg_func_t)TASK_LLM_llm_le_set_adv_data_cmd_handler},
    {LLM_LE_SET_ADV_EN_CMD,        (ke_msg_func_t)TASK_LLM_llm_le_set_adv_en_cmd_handler},
    {LLM_LE_SET_SCAN_RSP_DATA_CMD, (ke_msg_func_t)TASK_LLM_llm_le_set_scan_rsp_data_cmd_handler},
    #endif /* #if (BLE_BROADCASTER || BLE_PERIPHERAL) */

    #if (BLE_OBSERVER || BLE_CENTRAL)
    {LLM_LE_SET_SCAN_PARAM_CMD, (ke_msg_func_t)TASK_LLM_llm_le_set_scan_param_cmd_handler},
    {LLM_LE_SET_SCAN_EN_CMD,    (ke_msg_func_t)TASK_LLM_llm_le_set_scan_en_cmd_handler},
    #endif /* BLE_CENTRAL || BLE_OBSERVER */

    #if (BLE_CENTRAL)
    {LLM_LE_CREATE_CON_CMD,            (ke_msg_func_t)TASK_LLM_llm_le_create_con_cmd_handler},
    {LLM_LE_CREATE_CON_CANCEL_CMD,     (ke_msg_func_t)TASK_LLM_llm_le_create_con_cancel_cmd_handler},
    {LLM_LE_SET_HOST_CHNL_CLASSIF_CMD, (ke_msg_func_t)TASK_LLM_llm_le_set_host_ch_class_cmd_handler},
    #if (BLE_CHNL_ASSESS)
    {LLM_LE_CHNL_ASSESS_TIMER,         (ke_msg_func_t)TASK_LLM_llm_le_chnl_assess_timer_handler},
    #endif //(BLE_CHNL_ASSESS)
    #endif /* #if (BLE_CENTRAL) */

    {LLM_LE_RD_WLST_SIZE_CMD,      (ke_msg_func_t)TASK_LLM_llm_le_rd_wl_size_cmd_handler},
    {LLM_LE_CLEAR_WLST_CMD,        (ke_msg_func_t)TASK_LLM_llm_le_wl_clr_cmd_handler},
    {LLM_LE_ADD_DEV_TO_WLST_CMD,   (ke_msg_func_t)TASK_LLM_llm_le_wl_dev_add_cmd_handler},
    {LLM_LE_RMV_DEV_FROM_WLST_CMD, (ke_msg_func_t)TASK_LLM_llm_le_wl_dev_rem_cmd_handler},
    {LLM_LE_ENC_CMD,               (ke_msg_func_t)TASK_LLM_llm_le_enc_cmd_handler},
    {LLM_LE_RAND_CMD,              (ke_msg_func_t)TASK_LLM_llm_le_rand_cmd_handler},
    {LLM_LE_RD_SUPP_STATES_CMD,    (ke_msg_func_t)TASK_LLM_llm_le_rd_supp_states_cmd_handler},

    #if (BLE_TEST_MODE_SUPPORT)
    {LLM_LE_TEST_RX_CMD,  (ke_msg_func_t)TASK_LLM_llm_le_test_rx_cmd_handler},
    {LLM_LE_TEST_TX_CMD,  (ke_msg_func_t)TASK_LLM_llm_le_test_tx_cmd_handler},
    {LLM_LE_TEST_END_CMD, (ke_msg_func_t)TASK_LLM_llm_le_test_end_cmd_handler},
    #endif // BLE_TEST_MODE_SUPPORT

    {LLM_LE_SET_HOST_CH_CLASS_CMD_STO, (ke_msg_func_t)TASK_LLM_llm_le_set_host_ch_class_cmd_sto_handler},

    #if (BLE_OBSERVER || BLE_CENTRAL || BLE_PERIPHERAL)
    /// indication from the LLD
    {LLD_DATA_IND, (ke_msg_func_t)my_lld_data_ind_handler},
    #endif /* #if (BLE_OBSERVER || BLE_CENTRAL || BLE_PERIPHERAL) */

    /// legacy commands
    {LLM_RESET_CMD,                       (ke_msg_func_t)TASK_LLM_llm_reset_cmd_handler},
    {LLM_RD_BD_ADDR_CMD,                  (ke_msg_func_t)TASK_LLM_llm_rd_bd_addr_cmd_handler},
    {LLM_RD_LOCAL_VER_INFO_CMD,           (ke_msg_func_t)TASK_LLM_llm_rd_local_ver_info_cmd_handler},
    {LLM_RD_LOCAL_SUPP_CMDS_CMD,          (ke_msg_func_t)TASK_LLM_llm_rd_local_supp_cmds_cmd_handler},
    {LLM_SET_CNTLR_TO_HOST_FLOW_CNTL_CMD, (ke_msg_func_t)TASK_LLM_llm_set_cntl_host_flow_cmd_handler},
    {LLM_RD_LOCAL_SUPP_FEATS_CMD,         (ke_msg_func_t)TASK_LLM_llm_rd_local_supp_feats_cmd_handler},
    {LLM_SET_EVT_MASK_CMD,                (ke_msg_func_t)TASK_LLM_llm_set_evt_msk_cmd_handler},
    {LLM_HOST_BUFFER_SIZE_CMD,            (ke_msg_func_t)TASK_LLM_llm_host_buff_size_cmd_handler},
    {LLM_HOST_NB_COMPLETED_PKTS_CMD,      (ke_msg_func_t)TASK_LLM_llm_host_nb_comp_pkts_cmd_handler},
    {LLM_RD_BUFF_SIZE_CMD,                (ke_msg_func_t)TASK_LLM_llm_rd_buff_size_cmd_handler},
    {LLC_DISCO_IND,                       (ke_msg_func_t)TASK_LLM_llm_disconnect_mgt}
};

static const struct ke_state_handler my_llm_default_handler = KE_STATE_HANDLER(my_llm_default_state);

/// LLM task descriptor
static const struct ke_task_desc TASK_DESC_LLM = {llm_state_handler, &my_llm_default_handler, llm_state, LLM_STATE_MAX, LLM_IDX_MAX};

static int
my_lld_data_ind_handler(ke_msg_id_t const msgid,
                        struct lld_data_ind const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id)
{
    uint8_t rx_cnt = param->rx_cnt;
    uint8_t rx_hdl = param->rx_hdl;
    ke_state_t current_state;

    current_state = ke_state_get(TASK_LLM);

    /*
    *********************************************************************************
    ADVERTISING AND INITIATING RX MGT
    *********************************************************************************
    */
    if ((current_state != LLM_STOPPING) && (current_state != LLM_IDLE))
    {
        while(rx_cnt--)
        {
            // Get the RX descriptor corresponding to the handle
            struct co_buf_rx_desc *rxdesc = co_buf_rx_get(rx_hdl);
            // Get the type of the advertising packet received
            uint8_t pdu_type = (uint8_t)(rxdesc->rxheader & BLE_RXTYPE_MASK);
            switch (pdu_type)
            {
                case LLM_ADV_CONN_UNDIR:
                case LLM_ADV_CONN_DIR:
                case LLM_ADV_DISC_UNDIR:
                case LLM_ADV_NONCONN_UNDIR:
                case LLM_SCAN_RSP:
                    if(llm_util_rxlen_getf(rxdesc) < 6)
                    {
                        if(param->rx_cnt == 1) //if just one rx descr free it and do not call the actual handler
                        {
                            //invalid
                            //discard it
                            // Free the received buffer
                            co_buf_rx_free(rx_hdl);
                            return (KE_MSG_CONSUMED);
                        }
                        else
                        {
                            rxdesc->rxheader |= LLM_ADV_END; //alter the pdu_type so that it will not be processed
                        }
                    }
            }

            // Get the next one
            rx_hdl = co_buf_rx_next(rx_hdl);
        }
    }

    return TASK_LLM_lld_data_ind_handler(msgid, param, dest_id, src_id);
}

void patch_llm_task(void)
{
    uint8_t hdl;
    volatile struct ke_task_elem * curr_list = ke_task_env.task_list;
    uint8_t curr_nb = ke_task_env.task_cnt;

    // Search task handle
    for(hdl=0 ; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == TASK_LLM)
        {
            ke_task_env.task_list[hdl].p_desc = (struct ke_task_desc *) &TASK_DESC_LLM;
            //return;   let it search for all connections
        }
    }
}



#if ((BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1)  && BLE_HOST_PRESENT )
int gtl_sleep_to_handler(ke_msg_id_t const msgid,
                        void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);
int gtl_polling_to_handler(ke_msg_id_t const msgid,
                        void const *param,
                        ke_task_id_t const dest_id,
                        ke_task_id_t const src_id);



#define MAX_GTL_PENDING_PACKETS_ADV (50)
#define MAX_GTL_PENDING_PACKETS     (MAX_GTL_PENDING_PACKETS_ADV + 10)

/**
****************************************************************************************
* @brief Function called to send a message through UART.
*
* @param[in]  msgid   U16 message id from ke_msg.
* @param[in] *param   Pointer to parameters of the message in ke_msg.
* @param[in]  dest_id Destination task id.
* @param[in]  src_id  Source task ID.
*
* @return             Kernel message state, must be KE_MSG_NO_FREE.
*****************************************************************************************
*/
static int my_gtl_msg_send_handler (ke_msg_id_t const msgid,
                          void *param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
     //extract the ke_msg pointer from the param passed and push it in GTL queue
    struct ke_msg *msg = ke_param2msg(param);

    // Check if there is no transmission ongoing
    if (ke_state_get(TASK_GTL) != GTL_TX_IDLE)
    {
        if(gtl_env.tx_queue.tx_data_packet > MAX_GTL_PENDING_PACKETS_ADV)
        {
            if(msgid == GAPM_ADV_REPORT_IND || gtl_env.tx_queue.tx_data_packet > MAX_GTL_PENDING_PACKETS)
                return KE_MSG_CONSUMED;
        }
        co_list_push_back(&gtl_env.tx_queue, &(msg->hdr));
    }
    else
    {
        // send the message
        gtl_send_msg(msg);

        // Set GTL task to TX ONGOING state
        ke_state_set(TASK_GTL, GTL_TX_ONGOING);
    }

    //return NO_FREE always since gtl_eif_write handles the freeing
    return KE_MSG_NO_FREE;
}

const struct ke_msg_handler my_gtl_default_state[] =
{
    /** Default handler for GTL TX message, this entry has to be put first as table is
        parsed from end to start by Kernel */
    {KE_MSG_DEFAULT_HANDLER,  (ke_msg_func_t)my_gtl_msg_send_handler},

    {GTL_SLEEP_TO, (ke_msg_func_t)gtl_sleep_to_handler},
    {GTL_POLLING_TO, (ke_msg_func_t)gtl_polling_to_handler},

};

const struct ke_state_handler my_gtl_default_handler = KE_STATE_HANDLER(my_gtl_default_state);
const struct ke_task_desc TASK_DESC_GTL = {NULL, &my_gtl_default_handler, gtl_state, GTL_STATE_MAX, GTL_IDX_MAX};

void patch_gtl_task()
{
    uint8_t hdl;
    //struct ke_task_desc * p_task_desc = NULL;
    volatile struct ke_task_elem * curr_list = ke_task_env.task_list;
    uint8_t curr_nb = ke_task_env.task_cnt;

    // Search task handle
    for (hdl = 0; hdl < curr_nb; hdl++)
    {
        if(curr_list[hdl].type == TASK_GTL)
        {
            ke_task_env.task_list[hdl].p_desc = (struct ke_task_desc *) &TASK_DESC_GTL;
            return;
        }
    }
}

#endif // #if (BLE_APP_PRESENT == 0 || BLE_INTEGRATED_HOST_GTL == 1)

#ifndef __DA14581__
static bool cmp_abs_time(struct co_list_hdr const * timerA, struct co_list_hdr const * timerB)
{
    uint32_t timeA = ((struct ke_timer*)timerA)->time;
    uint32_t timeB = ((struct ke_timer*)timerB)->time;

    return (((uint32_t)( (timeA - timeB) & 0xFFFF) ) > KE_TIMER_DELAY_MAX);
}

#endif


/*
 * PATCH TABLE DEFINITION
 ****************************************************************************************
 */

static const uint32_t * const patch_table[] = 
{
#ifdef __DA14581__
    
    /* DA14581 */
    
# if (BLE_HOST_PRESENT)
    [0] = (const uint32_t *) atts_read_resp_patch,
    [2] = (const uint32_t *) my_smpc_check_pairing_feat,
    [3] = (const uint32_t *) my_smpc_check_param,
# else
    [0] = (const uint32_t *) llm_rd_local_supp_feats_cmd_handler,
# endif
    [1] = (const uint32_t *) lld_adv_start_patch,
# if (LOG_MEM_USAGE)
    [6] = (const uint32_t *) log_ke_malloc,
    [7] = (const uint32_t *) log_ke_free,
# endif

#else
    
    /* DA14580 */
    
    (const uint32_t *) cmp_abs_time,
    (const uint32_t *) l2cc_pdu_recv_ind_handler,
    (const uint32_t *) smpc_send_pairing_req_ind,
    (const uint32_t *) smpc_check_pairing_feat,
    (const uint32_t *) smpc_pairing_cfm_handler,
# if (LOG_MEM_USAGE)
    (const uint32_t *) log_ke_malloc,
    (const uint32_t *) log_ke_free,	
# else    
    (const uint32_t *) my_smpc_check_param,
    (const uint32_t *) NULL,
# endif        
    (const uint32_t *) patched_gapm_adv_op_sanity,
    
#endif //__DA14581__
};

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief SVC_Handler. Handles h/w patching mechanism IRQ
 *
 * @return void 
 ****************************************************************************************
 */
void SVC_Handler_c(unsigned int * svc_args)
{
    // Stack frame contains:
    // r0, r1, r2, r3, r12, r14, the return address and xPSR
    // - Stacked R0 = svc_args[0]
    // - Stacked R1 = svc_args[1]
    // - Stacked R2 = svc_args[2]
    // - Stacked R3 = svc_args[3]
    // - Stacked R12 = svc_args[4]
    // - Stacked LR = svc_args[5]
    // - Stacked PC = svc_args[6]
    // - Stacked xPSR= svc_args[7]

    unsigned int svc_number;
    
    svc_number = ((char *)svc_args[6])[-2];
    
    if (svc_number < (sizeof patch_table)/4)
        svc_args[6] = (uint32_t)patch_table[svc_number];
    else
        while(1);

    return;
}


/**
 ****************************************************************************************
 * @brief Set and enable h/w Patch functions
 *
 * @return void
 ****************************************************************************************
 */
void patch_func(void)
{

#ifdef __DA14581__
    
    /* DA14581 */
    
    //0x00024b1d lld_adv_start
    SetWord32(PATCH_ADDR0_REG, 0x00024b1C);
    SetWord32(PATCH_DATA0_REG, 0xb5ffdf01); //lld_adv_start svc 1 (+ enabling of interrupts)
    
    if(BLE_HOST_PRESENT)
    {        
        //0x0002a8c5 atts_read_resp
        SetWord32(PATCH_ADDR1_REG, 0x0002a8c4);
        SetWord32(PATCH_DATA1_REG, 0xb5f8df00); //atts_read_resp svc 0 (+ enabling of interrupts)
        
        // replace SMPC_PAIRING_CFM handler in smpc_default_state handler table:
        // smpc_default_state[8].func = &PATCHED_581_smpc_pairing_cfm_handler
        SetWord32(PATCH_ADDR2_REG, 0x00034538 + 8 * 8 + 4);
        SetWord32(PATCH_DATA2_REG, (uint32_t)&PATCHED_581_smpc_pairing_cfm_handler); //originally contained 0x0002d231 = the address of ROM function smpc_pairing_cfm_handler
        //0x0002c8e7 smpc_check_pairing_feat
        SetWord32(PATCH_ADDR3_REG, 0x0002c8e4);
        SetWord32(PATCH_DATA3_REG, 0xdf02e7f5); //smpc_check_pairing_feat svc 2 
        //0x00031949 smpc_check_param
        SetWord32(PATCH_ADDR4_REG, 0x00031948);
        SetWord32(PATCH_DATA4_REG, 0x4601df03); //smpc_check_param svc 3 
    }
    else
    {
        //0x000278f9 llm_rd_local_supp_feats_cmd_handler
        SetWord32(PATCH_ADDR1_REG, 0x000278f8);
        SetWord32(PATCH_DATA1_REG, 0x4619df00); //llm_rd_local_supp_feats_cmd_handler svc 0 

        SetWord32(PATCH_ADDR2_REG, 0x000278c0); // llm_rd_local_ver_info_cmd_handler
        SetWord32(PATCH_DATA2_REG, 0x80c121d2); // patch manufacturer ID

        SetWord32(PATCH_ADDR3_REG, 0x0002299c); // llc_version_ind_pdu_send
        SetWord32(PATCH_DATA3_REG, 0x80d020d2); // patch manufacturer ID

    SetWord32(PATCH_ADDR4_REG, 0x00023b40); //  llc_data_req_handler patch for bluez. change the compare mask for llid get from 3 to 1. 
    SetWord32(PATCH_DATA4_REG, 0xD02007C9); //
    }

    if (LOG_MEM_USAGE)
    {
        //0x00031fcd  ke_malloc
        SetWord32(PATCH_ADDR6_REG, 0x00031fcc);
        SetWord32(PATCH_DATA6_REG, 0x468edf06); //ke_malloc svc 6

        //0x0003208f  ke_free
        SetWord32(PATCH_ADDR7_REG, 0x0003208c);
        SetWord32(PATCH_DATA7_REG, 0xdf07bdf8); //ke_free svc 7
    }


#else /* DA14580 */
    
    //0x00032795 cmp_abs_time
    SetWord32(PATCH_ADDR0_REG, 0x00032794);
    SetWord32(PATCH_DATA0_REG, 0xdf00b662); //cmp_abs_time svc 0 (+ enabling of interrupts)

    //0x0002a32b l2cc_pdu_recv_ind_handler (atts)
    SetWord32(PATCH_ADDR1_REG, 0x0002a328);
    SetWord32(PATCH_DATA1_REG, 0xdf014770); //l2cc_pdu_recv_ind_handler svc 1

    //0x0002ca1f  smpc_send_pairing_req_ind
    SetWord32(PATCH_ADDR2_REG, 0x0002ca1c);
    SetWord32(PATCH_DATA2_REG, 0xdf02bdf8); //smpc_send_pairing_req_ind svc 2

    //0x0002cb43  smpc_check_pairing_feat
    SetWord32(PATCH_ADDR3_REG, 0x0002cb40);
    SetWord32(PATCH_DATA3_REG, 0xdf03e7f5); //smpc_check_pairing_feat svc 3

    //0x0002d485  smpc_pairing_cfm_handler
    SetWord32(PATCH_ADDR4_REG, 0x0002d484);
    SetWord32(PATCH_DATA4_REG, 0xb089df04); //smpc_pairing_cfm_handler svc 4

    if(LOG_MEM_USAGE)
    {
        //0x00032215  ke_malloc
        SetWord32(PATCH_ADDR5_REG, 0x00032214);
        SetWord32(PATCH_DATA5_REG, 0x468edf05); //ke_malloc svc 5

        //0x000322d7  ke_free
        SetWord32(PATCH_ADDR6_REG, 0x000322d4);
        SetWord32(PATCH_DATA6_REG, 0xdf06bdf8); //ke_free svc 6
    }
    else
    {
        //0x00031b95  smpc_check_param
        SetWord32(PATCH_ADDR5_REG, 0x00031b94);
        SetWord32(PATCH_DATA5_REG, 0xb500df05); //smpc_check_param svc 5
    }

    //0x00030cef gapm_adv_op_sanity
    SetWord32(PATCH_ADDR7_REG, 0x00030cec);
    SetWord32(PATCH_DATA7_REG, 0xdf07bd70); //gapm_adv_op_sanity svc 7

    NVIC_DisableIRQ(SVCall_IRQn);
    NVIC_SetPriority(SVCall_IRQn, 0);
    NVIC_EnableIRQ(SVCall_IRQn);
#endif //__DA14581__
}

/// @} PATCHES
