#include <stdio.h>
#include "espnow_rc.h"

esp_now_peer_info_t erc_peer_info = {
    .channel = 0,
    .ifidx = ESP_IF_WIFI_STA,
    .encrypt = false,
    .lmk = {0},
    .peer_addr = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};


erc_dataframe_t incomingData; //buffer for incoming espnow messages
erc_config_t erc_config; //config for espnow_rc
bool erc_paired_flag = false; //flag for pairing status
bool erc_configured = false; //flag if component is configured
uint8_t mac[6]; //mac address of this device

//********************************************************************************

//callback for received data over ESP-NOW
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    //receive from sender
    incomingData = *(erc_dataframe_t*)data;

    if (!erc_paired_flag) {
        //debug print incoming data
        printf("Received data: mode: %d, pairing_mode: %d, mac addr %02X:%02X:%02X:%02X:%02X:%02X\n", incomingData.mode, incomingData.pairing_mode, recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2], recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5]);
    }

    //procedure for RX mode of erc
    if (erc_config.mode == ERC_MODE_RX && erc_configured)
    {
        //if device is not paired and received packet is in pairing mode and packet comes from a transmitter
        if (!erc_paired_flag && incomingData.pairing_mode == 1 && incomingData.mode == ERC_MODE_TX)
        {
            uint8_t *peer_addr_temp = recv_info->src_addr;
            esp_now_del_peer(erc_peer_info.peer_addr);
            memcpy(erc_peer_info.peer_addr, peer_addr_temp, sizeof(erc_peer_info.peer_addr));
            printf("RX paired to TX with mac addr: %02X:%02X:%02X:%02X:%02X:%02X\n", erc_peer_info.peer_addr[0], erc_peer_info.peer_addr[1], erc_peer_info.peer_addr[2], erc_peer_info.peer_addr[3], erc_peer_info.peer_addr[4], erc_peer_info.peer_addr[5]);
            esp_now_add_peer(&erc_peer_info);
            erc_paired_flag = true;
        }
    }

    //procedure for TX mode of erc
    if (erc_config.mode == ERC_MODE_TX && erc_configured)
    {
        //if device is not paired and received packet is in pairing mode and if packet comes from a receiver
        if (!erc_paired_flag && incomingData.pairing_mode == 1 && incomingData.mode == ERC_MODE_RX)
        {
            esp_now_del_peer(erc_peer_info.peer_addr);
            uint8_t *peer_addr_temp = recv_info->src_addr;
            memcpy(erc_peer_info.peer_addr, peer_addr_temp, sizeof(erc_peer_info.peer_addr));
            esp_now_add_peer(&erc_peer_info);
            erc_dataframe_t sendback_frame;
            sendback_frame.mode = erc_config.mode;
            sendback_frame.pairing_mode = 1;
            //send back a packet with mac addr to receiver
            esp_now_send(erc_peer_info.peer_addr, (uint8_t *) &sendback_frame, sizeof(sendback_frame));
            printf("TX paired to RX with mac addr: %02X:%02X:%02X:%02X:%02X:%02X\n", erc_peer_info.peer_addr[0], erc_peer_info.peer_addr[1], erc_peer_info.peer_addr[2], erc_peer_info.peer_addr[3], erc_peer_info.peer_addr[4], erc_peer_info.peer_addr[5]);
            erc_paired_flag = true;
        }
    }
}

//RECEIVER PAIRING FUNCTIONS

void erc_rx_send_broadcast(void) {
    erc_dataframe_t dataPacket;
    dataPacket.mode = ERC_MODE_RX;
    dataPacket.pairing_mode = 1;
    esp_now_send(erc_peer_info.peer_addr, (uint8_t *) &dataPacket, sizeof(dataPacket));
}

//task for periodic sending of broadcast pairing data with mac address
void erc_rx_pairing_task(void *arg) {
    printf("RX pairing task started\n");
    erc_paired_flag = false;
    while (!erc_paired_flag) {
        erc_rx_send_broadcast();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Paired, RX pairing task ended\n");
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
 * @brief manually configure peer mac address (no pairing)
 * 
 * @param mac mac address of the peer to be added
 * @return esp_err_t 
 */
esp_err_t erc_set_peer_mac(uint8_t *mac) {
    erc_paired_flag = true;
    memcpy(erc_peer_info.peer_addr, mac, sizeof(erc_peer_info.peer_addr));
    return ESP_OK;
}

/**
 * @brief function for sending data to paired device (both RX and TX can send data)
 * 
 * @param data pointer to erc_dataframe_t struct to be sent
 * @return esp_err_t 
 */
esp_err_t erc_send_data(erc_dataframe_t *data) {
    if (!erc_paired_flag) {
        return ESP_FAIL;
    }
    esp_err_t err = esp_now_send(erc_peer_info.peer_addr, (uint8_t *) data, sizeof(erc_dataframe_t));
    return err;
}

/**
 * @brief function to read receiver buffer to a struct provided in the argument
 * 
 * @param data pointer to erc_dataframe_t struct to store received data
 */
void erc_receive_data(erc_dataframe_t *data) {
    *data = incomingData;
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
    erc_configured = true;
    if (erc_config.mode == ERC_MODE_RX) {
        printf("RX mode\n");
    }
    if (erc_config.mode == ERC_MODE_TX) {
        printf("TX mode\n");
    }
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    esp_now_add_peer(&erc_peer_info);
}

