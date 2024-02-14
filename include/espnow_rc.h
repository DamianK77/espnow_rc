#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "string.h"

/**
 * @brief struct for configuration of espnow_rc
 * 
 * @param mode mode, ERC_MODE_TX if transmitter, ERC_MODE_RX if receiver
 * 
 */
typedef struct {
    uint8_t mode;
}erc_config_t;

/**
 * @brief struct for 8 channel transmitter
 * 
 * @param mode mode of this device, 0 if transmitter 1 if receiver
 * @param pairing_mode pairing mode, 1 if ready to pair, 0 if not pairing
 * 
 */
typedef struct {
    uint8_t mode;
    uint8_t pairing_mode;

    int16_t ch0;
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;
    int16_t ch5;
    int16_t ch6;
    int16_t ch7;
} erc_dataframe_t;

#define ERC_MODE_TX 0
#define ERC_MODE_RX 1

void erc_init(erc_config_t *config);
esp_err_t erc_rx_start_pairing(void);
