/**
 ****************************************************************************************
 *
 * @file system_start.h
 *
 * @brief Definition for HW initialize, Wi-Fi and applications
 *
 * Copyright (c) 2016-2024 Renesas Electronics. All rights reserved.
 *
 * This software ("Software") is owned by Renesas Electronics.
 *
 * By using this Software you agree that Renesas Electronics retains all
 * intellectual property and proprietary rights in and to this Software and any
 * use, reproduction, disclosure or distribution of the Software without express
 * written permission or a license agreement from Renesas Electronics is
 * strictly prohibited. This Software is solely for use on or in conjunction
 * with Renesas Electronics products.
 *
 * EXCEPT AS OTHERWISE PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, THE
 * SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. EXCEPT AS OTHERWISE
 * PROVIDED IN A LICENSE AGREEMENT BETWEEN THE PARTIES, IN NO EVENT SHALL
 * RENESAS ELECTRONICS BE LIABLE FOR ANY DIRECT, SPECIAL, INDIRECT, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THE SOFTWARE.
 *
 ****************************************************************************************
 */

#include "da16x_network_common.h"
#include "common_def.h"
#include "da16x_system.h"

/*
 * External global variables
 */
extern int factory_rst_btn;
#if defined ( __SUPPORT_EVK_LED__ )
extern int   factory_rst_led;
#endif    // __SUPPORT_EVK_LED__
extern int factory_rst_btn_chk_time;

extern int wps_btn;
#if defined ( __SUPPORT_EVK_LED__ )
extern int   wps_led;
#endif    // __SUPPORT_EVK_LED__
extern int wps_btn_chk_time;

#if defined (__SUPPORT_NOTIFY_RTC_WAKEUP__)
extern int   rtc_wakeup_intr_chk_flag;
#endif // __SUPPORT_NOTIFY_RTC_WAKEUP__

extern short dpm_app_register_timeout;
extern short dpm_app_data_rcv_ready_set_timeout;

#if defined ( __SET_BOR_CIRCUIT__ )
extern unsigned char br_ctrl;
#endif // __SET_BOR_CIRCUIT__

/*
 * For Customer's configuration
 *
 * #define MAX_SSID_PREFIX_LEN        64
 * #define MAX_PASSKEY_LEN            64
 * #define MAX_IP_ADDR_LEN            16
 *
 * #define AP_OPEN_MODE               0
 * #define AP_SECURITY_MODE           1
 *
 * #define IPADDR_DEFAULT             0
 * #define IPADDR_CUSTOMER            1
 *
 * #define DHCPD_DEFAULT              0
 * #define DHCPD_CUSTOMER             1
 *
 *
 *    typedef struct _softap_config {
 *        int    customer_cfg_flag;    // MODE_ENABLE, MODE_DISABLE
 *
 *        char    ssid_name[MAX_SSID_LEN];
 *        char    psk[MAX_PASSKEY_LEN];
 *        char    auth_type;        // AP_OPEN_MODE, AP_SECURITY_MODE
 *
 *        // AD  AE  AF  AI  AL  AM  AR  AS  AT  AU  AW  AZ  BA  BB  BD  BE  BF  BG  BH  BL
 *        // BM  BN  BO  BR  BS  BT  BY  BZ  CA  CF  CH  CI  CL  CN  CO  CR  CU  CX  CY  CZ
 *        // DE  DK  DM  DO  DZ  EC  EE  EG  ES  ET  EU  FI  FM  FR  GA  GB  GD  GE  GF  GH
 *        // GL  GP  GR  GT  GU  GY  HK  HN  HR  HT  HU  ID  IE  IL  IN  IR  IS  IT  JM  JO
 *        // JP  KE  KH  KN  KP  KR  KW  KY  KZ  LB  LC  LI  LK  LS  LT  LU  LV  MA  MC  MD
 *        // ME  MF  MH  MK  MN  MO  MP  MQ  MR  MT  MU  MV  MW  MX  MY  NG  NI  NL  NO  NP
 *        // NZ  OM  PA  PE  PF  PG  PH  PK  PL  PM  PR  PT  PW  PY  QA  RE  RO  RS  RU  RW
 *        // SA  SE  SG  SI  SK  SN  SR  SV  SY  TC  TD  TG  TH  TN  TR  TT  TW  TZ  UA  UG
 *        // UK  US  UY  UZ  VA  VC  VE  VI  VN  VU  WF  WS  YE  YT  ZA  ZW
 *        char    country_code[4];
 *
 *         int    customer_ip_address;    // IPADDR_DEFAULT, IPADDR_CUSTOMER
 *        char    ip_addr[MAX_IP_ADDR_LEN];
 *        char    subnet_mask[MAX_IP_ADDR_LEN];
 *        char    default_gw[MAX_IP_ADDR_LEN];
 *        char    dns_ip_addr[MAX_IP_ADDR_LEN];
 *
 *        int    customer_dhcpd_flag;    // DHCPD_DEFAULT, DHCPD_CUSTOMER
 *        int    dhcpd_lease_time;
 *        char    dhcpd_start_ip[MAX_IP_ADDR_LEN];
 *        char    dhcpd_end_ip[MAX_IP_ADDR_LEN];
 *        char    dhcpd_dns_ip_addr[MAX_IP_ADDR_LEN];
 *
 *    } softap_config_t;
 */
extern softap_config_t *ap_config_param;

extern int customer_console_baudrate;
#if defined ( __USER_DPM_ABNORM_WU_INTERVAL__ )
extern unsigned long long *dpm_abnorm_user_wakeup_interval;
#endif // __USER_DPM_ABNORM_WU_INTERVAL__

/*
 * External global functions
 */
extern UINT _da16x_io_pinmux(UINT mux, UINT config);
extern UINT factory_reset_default(int reboot_flag);
extern int compare_NVRAM(void);
extern void reg_n_start_DPM_manager(void);

extern UINT wps_setup(char *macaddr);
extern int gpio_handle_create_event(void);
extern void start_gpio_thread(void);
extern int set_gpio_interrupt(void);

extern int (*button1_one_touch_cb)(void);
extern void regist_wifi_notify_cb(void);

#if defined ( __SET_BOR_CIRCUIT__ )
extern void da16x_environ_lock(UINT32 flag);
#endif // __SET_BOR_CIRCUIT__

#if defined ( __SUPPORT_WIFI_CONCURRENT__ )
extern int factory_reset_btn_onetouch(void);
#endif // __SUPPORT_WIFI_CONCURRENT__

/*
 ****************************************************************************************
 * @brief        Set paramters for system running
 * @param[in]    None
 * @return       None
 ****************************************************************************************
 */
extern void set_sys_config(void);

/*
 ****************************************************************************************
 * @brief        Set paramters for system running. Initialize WLAN interface
 * @param[in]    None
 * @return       None
 ****************************************************************************************
 */
extern UINT wlaninit(void);

/*
 ****************************************************************************************
 * @brief        Config Pin-mux to control Wi-Fi & BT co-existence
 * @return       None
 ****************************************************************************************
 */
extern void initialize_bt_coex(void);

#if defined ( __SUPPORT_EVK_LED__ )
/*
 ****************************************************************************************
 * @brief        Set Factory-Reset button configuration
 * @param[in]    reset_gpio  GPIO pin number for Factory-Reset button
 * @param[in]    led_gpio    GPIO pin number for Factory-Reset LED
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern void config_factory_reset_button(int reset_gpio, int led_gpio, int seconds);

/*
 ****************************************************************************************
 * @brief        Set Wi-Fi WPS button configuration
 * @param[in]    reset_gpio  GPIO pin number for Factory-Reset button
 * @param[in]    led_gpio    GPIO pin number for Factory-Reset LED
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern void config_wps_button(int wps_gpio, int led_gpio, int seconds);

/*
 ****************************************************************************************
 * @brief        Check button press status for Factory-Reset button
 * @param[in]    reset_gpio  GPIO pin number for Factory-Reset button
 * @param[in]    led_gpio    GPIO pin number for Factory-Reset LED
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern UINT check_factory_button(int btn_gpio_num, int led_gpio_num, int check_time);

/*
 ****************************************************************************************
 * @brief        Check button press status for Wi-Fi WPS button
 * @param[in]    wps_gpio    GPIO pin number for Wi-FI WPS button
 * @param[in]    led_gpio    GPIO pin number for Wi-FI WPS LED
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern UINT    check_wps_button(int btn_gpio_num, int led_gpio_num, int check_time);

#else

/*
 ****************************************************************************************
 * @brief        Set Factory-Reset button configuration
 * @param[in]    reset_gpio  GPIO pin number for Factory-Reset button
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern void config_factory_reset_button(int reset_gpio, int seconds);

/*
 ****************************************************************************************
 * @brief        Set Wi-Fi WPS button configuration
 * @param[in]    reset_gpio    GPIO pin number for Factory-Reset button
 * @param[in]    seconds       Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern void config_wps_button(int wps_gpio, int seconds);

/*
 ****************************************************************************************
 * @brief        Check button press status for Factory-Reset button
 * @param[in]    reset_gpio  GPIO pin number for Factory-Reset button
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern UINT check_factory_button(int btn_gpio_num, int check_time);

/*
 ****************************************************************************************
 * @brief        Check button press status for Wi-Fi WPS button
 * @param[in]    wps_gpio    GPIO pin number for Wi-FI WPS button
 * @param[in]    seconds     Checking time for button press ( seconds )
 * @return       None
 ****************************************************************************************
 */
extern UINT check_wps_button(int btn_gpio_num, int check_time);

#endif    // __SUPPORT_EVK_LED__

/*
 ****************************************************************************************
 * @brief        Start system applications to run DA16200 ( Registered in sys_apps_tablep[] )
 * @return       None
 ****************************************************************************************
 */
extern void start_sys_apps(void);

/*
 ****************************************************************************************
 * @brief        Start system applications to run DA16200 ( Registered in user_apps_tablep[] )
 * @return       None
 ****************************************************************************************
 */
extern void start_user_apps(void);

/*
 ****************************************************************************************
 * @brief        
 * @input[in]
 * @return        None
 ****************************************************************************************
 */
extern void gpio_callback(void *param);

/**
 *********************************************************************************
 * @brief      Configurate pin-mux for DA16200
 * @param      None
 * @return     Returns always TRUE
 *********************************************************************************
 */
int config_pin_mux(void);

/**
 *********************************************************************************
 * @brief      Register one-touch press call-back function for Factory-Reset button
 * @return     None
 *********************************************************************************
 */
void set_customer_softap_config(void);

/**
 *********************************************************************************
 * @brief      Register user application to handle DPM opeartion on DPM manager
 * @return     None
 *********************************************************************************
 */
void regist_user_apps_to_DPM_manager(void);

/**
 *********************************************************************************
 * @brief      Start HW configuration, Wi-Fi and all kind of applications
 * @input[in]  None
 * @return     None
 *********************************************************************************
 */
void system_start(void);

/**
 *********************************************************************************
 * @brief      Set gpio interrupt configuration for Factory_Reset and WPS button
 * @return     None
 *********************************************************************************
 */
int set_gpio_interrupt(void);

/* EOF */
