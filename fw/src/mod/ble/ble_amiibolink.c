#include "ble_amiibolink.h"
#include "ble_main.h"
#include "df_buffer.h"
#include "nrf_log.h"
#include "ntag_def.h"
#include "ntag_emu.h"

static ntag_t m_ntag = {0};
uint32_t m_data_pos = 0;
ble_amiibolink_event_handler_t m_event_handler = {0};
void *m_event_ctx = {0};

void ble_amiibolink_set_event_handler(ble_amiibolink_event_handler_t handler, void *ctx) {
    m_event_handler = handler;
    m_event_ctx = ctx;
}

void ble_amiibolink_send_cmd(uint16_t cmd) { ble_nus_tx_data(&cmd, 2); }

void ble_amiibolink_write_ntag(buffer_t *buffer) {
    buff_get_u8(buffer); // 00
    uint8_t size = buff_get_u8(buffer);
    void *data_ptr = buff_get_data_ptr_pos(buffer);
    memcpy(m_ntag.data + m_data_pos, data_ptr, size);
    m_data_pos += size;
}

void ble_amiibolink_init(void) {}
void ble_amiibolink_received_data(const uint8_t *data, size_t length) {
    NRF_LOG_INFO("ble data received %d bytes", length);
    //NRF_LOG_HEXDUMP_INFO(data, length);

    ble_amiibolink_mode_t mode;

    NEW_BUFFER_READ(buffer, (void *)data, length);

    uint16_t cmd = buff_get_u16(&buffer);
    switch (cmd) {

    case 0xB1A1: // send model code ??
        // a1 b1 01
        // 01: 随机模式 02: 按序模式 03:读写模式
        mode = (ble_amiibolink_mode_t)buff_get_u8(&buffer);
        if (m_event_handler) {
            m_event_handler(m_event_ctx, BLE_AMIIBOLINK_EVENT_SET_MODE, &mode, sizeof(ble_amiibolink_mode_t));
        }
        ble_amiibolink_send_cmd(0xA1B1);
        break;

    case 0xB0A0: // seq 1
        ble_amiibolink_send_cmd(0xA0B0);
        break;

    case 0xACAC: // seq 2
        // ac ac 00 04 00 00 02 1c //540
        m_data_pos = 0;
        ble_amiibolink_send_cmd(0xCACA);
        break;

    case 0xABAB: // seq 3
        // ab ab 02 1c
        ble_amiibolink_send_cmd(0xBABA);
        break;

    case 0xAADD: // seq 4
        ble_amiibolink_write_ntag(&buffer);
        ble_amiibolink_send_cmd(0xDDAA);
        break;

    case 0xBCBC: // seq 5
        ble_amiibolink_send_cmd(0xCBCB);
        break;

    case 0xDDCC: // seq 6
        NRF_LOG_INFO("ntag_emu_set_tag");
        ntag_emu_set_tag(&m_ntag);
        if (m_event_handler) {
            m_event_handler(m_event_ctx, BLE_AMIIBOLINK_EVENT_TAG_UPDATED, &m_ntag, sizeof(m_ntag));
        }
        ble_amiibolink_send_cmd(0xCCDD);
        break;
    }
}