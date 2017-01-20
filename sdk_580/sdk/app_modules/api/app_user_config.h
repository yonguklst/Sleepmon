/**
 ****************************************************************************************
 *
 * @file app_user_config.h
 *
 * @brief Compile configuration file.
 *
 * Copyright (C) 2015. Dialog Semiconductor Ltd, unpublished work. This computer
 * program includes Confidential, Proprietary Information and is a Trade Secret of
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

#ifndef _APP_USER_CONFIG_H_
#define _APP_USER_CONFIG_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup
 *
 * @brief
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "co_bt.h"
#include "gap.h"
#include "gapm.h"
#include "gapm_task.h"
#include "gapc_task.h"
#include <stdint.h>

/*
 * DEFINES
 ****************************************************************************************
 */

#define MS_TO_BLESLOTS(x)    ((int)(x/0.625))
#define MS_TO_DOUBLESLOTS(x) ((int)(x/1.25))
#define MS_TO_TIMERUNITS(x)  ((int)(x/10))

#define US_TO_BLESLOTS(x)    ((int)(x/625))
#define US_TO_DOUBLESLOTS(x) ((int)(x/1250))
#define US_TO_TIMERUNITS(x)  ((int)(x/10000))

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct security_configuration
{
    /// IO capabilities (@see gap_io_cap)
    enum gap_io_cap iocap;
    /// OOB information (@see gap_oob)
    enum gap_oob oob;
    /// Authentication (@see gap_auth)
    enum gap_auth auth;
    /// Encryption key size (7 to 16)
    uint8_t key_size;
    ///Initiator key distribution (@see gap_kdist)
    enum gap_kdist ikey_dist;
    ///Responder key distribution (@see gap_kdist)
    enum gap_kdist rkey_dist;
    /// Device security requirements (minimum security level). (@see gap_sec_req)
    enum gap_sec_req sec_req;
    struct gap_sec_key tk;
    struct gap_sec_key csrk;
};

/*
 ****************************************************************************************
 *
 * Advertising related configuration
 *
 ****************************************************************************************
 */
struct advertise_configuration {
     /// Own BD address source of the device:
     /// - GAPM_PUBLIC_ADDR:         Public Address
     /// - GAPM_PROVIDED_RND_ADDR:   Provided random address
     /// - GAPM_GEN_STATIC_RND_ADDR: Generated static random address
     /// - GAPM_GEN_RSLV_ADDR:       Generated resolvable private random address
     /// - GAPM_GEN_NON_RSLV_ADDR:   Generated non-resolvable private random address
     /// - GAPM_PROVIDED_RECON_ADDR: Provided Reconnection address (only for GAPM_ADV_DIRECT)
    enum gapm_own_addr_src addr_src;

    /// Duration of resolvable address before regenerate it.
    uint16_t renew_dur;

    /// Provided own static private random address (addr_src = GAPM_PROVIDED_RND_ADDR or GAPM_PROVIDED_RECON_ADDR)
    uint8_t addr[BD_ADDR_LEN];

    /// Minimum interval for advertising
    uint16_t intv_min;

    /// Maximum interval for advertising
    uint16_t intv_max;

    /// Advertising channel map
    uint8_t channel_map;

    /*************************
     * Advertising information
     *************************
     */

    /// Host information advertising data (GAPM_ADV_NON_CONN and GAPM_ADV_UNDIRECT)
    /// Advertising mode :
    /// - GAP_NON_DISCOVERABLE: Non discoverable mode
    /// - GAP_GEN_DISCOVERABLE: General discoverable mode
    /// - GAP_LIM_DISCOVERABLE: Limited discoverable mode
    /// - GAP_BROADCASTER_MODE: Broadcaster mode
    uint8_t mode;

    /// Host information advertising data (GAPM_ADV_NON_CONN and GAPM_ADV_UNDIRECT)
    /// Advertising filter policy:
    /// - ADV_ALLOW_SCAN_ANY_CON_ANY: Allow both scan and connection requests from anyone
    /// - ADV_ALLOW_SCAN_WLST_CON_ANY: Allow both scan req from White List devices only and
    ///   connection req from anyone
    /// - ADV_ALLOW_SCAN_ANY_CON_WLST: Allow both scan req from anyone and connection req
    ///   from White List devices only
    /// - ADV_ALLOW_SCAN_WLST_CON_WLST: Allow scan and connection requests from White List
    ///   devices only
    uint8_t adv_filt_policy;

    ///  Direct address information (GAPM_ADV_DIRECT)
    /// (used only if reconnection address isn't set or privacy disabled)
    /// BD Address of device
    uint8_t peer_addr[BD_ADDR_LEN];

    ///  Direct address information (GAPM_ADV_DIRECT)
    /// (used only if reconnection address isn't set or privacy disabled)
    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_type;
};

/*
 ****************************************************************************************
 *
 * Parameter update configuration
 *
 ****************************************************************************************
 */
struct connection_param_configuration {
    /// Connection interval minimum
    uint16_t intv_min;

    /// Connection interval maximum
    uint16_t intv_max;

    /// Latency
    uint16_t latency;

    /// Supervision timeout
    uint16_t time_out;

    /// Minimum Connection Event Duration
    uint16_t ce_len_min;

    /// Maximum Connection Event Duration
    uint16_t ce_len_max;
};

/*
 ****************************************************************************************
 *
 * GAPM configuration
 *
 ****************************************************************************************
 */
struct gapm_configuration {
    /// Device Role: Central, Peripheral, Observer or Broadcaster
    enum gap_role role;

    /// Device IRK used for resolvable random BD address generation (LSB first)
    uint8_t irk[KEY_LEN];

    /// Device Appearance (0x0000 - Unknown appearance)
    /// Fill in according to https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.gap.appearance.xml
    uint16_t appearance;

    /// Device Appearance write permission requirements for peer device (@see gapm_write_att_perm)
    enum gapm_write_att_perm appearance_write_perm;

    /// Device Name write permission requirements for peer device (@see gapm_write_att_perm)
    enum gapm_write_att_perm name_write_perm;

    /// Maximal MTU
    uint16_t max_mtu;

    /// Peripheral only: *****************************************************************
    /// Slave preferred Minimum of connection interval
    uint16_t con_intv_min;

    /// Slave preferred Maximum of connection interval
    uint16_t con_intv_max;

    /// Slave preferred Connection latency
    uint16_t con_latency;

    /// Slave preferred Link supervision timeout
    uint16_t superv_to;

    /// Privacy settings bit field (0b1 = enabled, 0b0 = disabled)
    ///  - [bit 0]: Privacy Support
    ///  - [bit 1]: Multiple Bond Support (Peripheral only); If enabled, privacy flag is
    ///             read only.
    ///  - [bit 2]: Reconnection address visible.
    uint8_t flags;
};

/*
 ****************************************************************************************
 *
 * Central configuration
 *
 ****************************************************************************************
 */
struct central_configuration {
    /// GAPM requested operation:
    /// - GAPM_CONNECTION_DIRECT: Direct connection operation
    /// - GAPM_CONNECTION_AUTO: Automatic connection operation
    /// - GAPM_CONNECTION_SELECTIVE: Selective connection operation
    /// - GAPM_CONNECTION_NAME_REQUEST: Name Request operation (requires to start a direct
    ///   connection)
    uint8_t code;

    /// Own BD address source of the device:
    /// - GAPM_PUBLIC_ADDR: Public Address
    /// - GAPM_PROVIDED_RND_ADDR: Provided random address
    /// - GAPM_GEN_STATIC_RND_ADDR: Generated static random address
    /// - GAPM_GEN_RSLV_ADDR: Generated resolvable private random address
    /// - GAPM_GEN_NON_RSLV_ADDR: Generated non-resolvable private random address
    /// - GAPM_PROVIDED_RECON_ADDR: Provided Reconnection address (only for GAPM_ADV_DIRECT)
    uint8_t addr_src;

    /// Duration of resolvable address before regenerate it.
    uint16_t renew_dur;

    /// Provided own static private random address (addr_src = GAPM_PROVIDED_RND_ADDR or GAPM_PROVIDED_RECON_ADDR)
    uint8_t addr[BD_ADDR_LEN];

    /// Scan interval
    uint16_t scan_interval;

    /// Scan window size
    uint16_t scan_window;

    /// Minimum of connection interval
    uint16_t con_intv_min;

    /// Maximum of connection interval
    uint16_t con_intv_max;

    /// Connection latency
    uint16_t con_latency;

    /// Link supervision timeout
    uint16_t superv_to;

     /// Minimum CE length
    uint16_t ce_len_min;

    /// Maximum CE length
    uint16_t ce_len_max;

    /**************************************************************************************
     * Peer device information, only for:
     *
     * - GAPM_CONNECTION_AUTO
     * - GAPM_CONNECTION_SELECTIVE
     *
     * White list with peer addresses and the respective peer address type
     **************************************************************************************
     */

    /// BD Address of device
    uint8_t peer_addr_0[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_0_type;

    /// BD Address of device
    uint8_t peer_addr_1[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_1_type;

    /// BD Address of device
    uint8_t peer_addr_2[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_2_type;

    /// BD Address of device
    uint8_t peer_addr_3[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_3_type;

    /// BD Address of device
    uint8_t peer_addr_4[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_4_type;

    /// BD Address of device
    uint8_t peer_addr_5[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_5_type;

    /// BD Address of device
    uint8_t peer_addr_6[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_6_type;

    /// BD Address of device
    uint8_t peer_addr_7[BD_ADDR_LEN];

    /// Address type of the device 0=public/1=private random
    uint8_t peer_addr_7_type;
};

/// @} APP

#endif // _APP_USER_CONFIG_H_
