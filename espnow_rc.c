#include <stdio.h>
#include "espnow_rc.h"

const uint8_t mac_broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

esp_now_peer_info_t erc_peer_info = {
    .channel = 0,
    .ifidx = ESP_IF_WIFI_STA,
    .encrypt = false,
    .lmk = {0},
    .peer_addr = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};


erc_dataframe_t incomingData; //buffer for incoming espnow messages
erc_config_t erc_config; //config for espnow_rc
bool erc_paired_flag = false; //flag for pairing status
uint8_t mac[6]; //mac address of this device

//********************************************************************************


static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    //receive from sender
    incomingData = *(erc_dataframe_t*)data; 
}

void erc_rx_send_broadcast(void) {
    erc_dataframe_t dataPacket;
    esp_read_mac(dataPacket.sender_mac, ESP_MAC_WIFI_STA);
    dataPacket.mode = ERC_MODE_RX;
    dataPacket.tx_pairing_mode = 1;
    esp_now_send(&mac_broadcast, (uint8_t *) &dataPacket, sizeof(dataPacket));
}

//task for periodic sending of broadcast pairing data with mac address
void erc_rx_pairing_task(void *arg) {
    erc_paired_flag = false;
    while (!erc_paired_flag) {
        erc_rx_send_broadcast();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

/**
 * @brief starts pairing for receiver (broadcast mac address)
 * 
 * @return esp_err_t 
 */
esp_err_t erc_rx_start_pairing(void) 
{
    if (erc_config.mode != ERC_MODE_RX) {
        return ESP_FAIL;
    }
    BaseType_t err = xTaskCreate(erc_rx_pairing_task, "erc_rx_pairing_task", 2048, NULL, configMAX_PRIORITIES-4, NULL);
    if (err != pdPASS) {
        return ESP_FAIL;
    } else {
        return ESP_OK;
    }
}

/**
 * @brief setup wifi for esp-now long range communication and read config
 * 
 * @param config config for espnow_rc
 */
void erc_init(erc_config_t *config)
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
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    erc_config = *config;
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
}

