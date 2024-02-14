#include <stdio.h>
#include "espnow_rc.h"

const uint8_t *broadcast_address = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t peer_info = {
    .channel = 0,
    .ifidx = ESP_IF_WIFI_STA,
    .encrypt = false,
    .lmk = {0},
    .peer_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

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
 * @param sender_mac mac address of sending device
 * @param mode mode, 0 if transmitter 1 if receiver
 * 
 */
typedef struct {
    uint8_t sender_mac[6];
    uint8_t mode;

    int16_t ch0;
    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;
    int16_t ch5;
    int16_t ch6;
    int16_t ch7;
} erc_dataframe_t;

void erc_init(void)
{
    nvs_flash_init();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N|WIFI_PROTOCOL_LR) );
    ESP_ERROR_CHECK(esp_now_init());
}

void erc_rx_send_pair(void) {
    erc_dataframe_t dataPacket;
    esp_read_mac(dataPacket.sender_mac, ESP_MAC_WIFI_STA);
    dataPacket.mode = 1;
    esp_now_send(broadcast_address, (uint8_t *) &dataPacket, sizeof(dataPacket));
}