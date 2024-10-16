/*
 * ble_cus.c file for the custom ble service
 */
#include "ble_cus.h"

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_cus       Custom service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_cus_t *p_cus, ble_evt_t const *p_ble_evt) {
  ble_gatts_evt_write_t const *p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
  ble_cus_evt_t evt;

  // writing to the leds states characteristic
  if (p_evt_write->handle == p_cus->leds_states_char_handles.value_handle) {
    evt.params_command.command_data.p_data = p_evt_write->data;
    evt.params_command.command_data.length = p_evt_write->len;
    evt.evt_type = BLE_LEDS_STATES_CHAR_EVT_COMMAND_RX;

    p_cus->evt_handler(p_cus, &evt);
  }

  // writing to the buttons states characteristic (cccd) "client characteristic configuration descriptor"
  else if (p_evt_write->handle == p_cus->buttons_states_char_handles.cccd_handle) {
    if (ble_srv_is_notification_enabled(p_evt_write->data)) {
      evt.evt_type = BLE_BUTTONS_STATES_CHAR_NOTIFICATIONS_ENABLED;
    } else {
      evt.evt_type = BLE_BUTTONS_STATES_CHAR_NOTIFICATIONS_DISABLED;
    }

    p_cus->evt_handler(p_cus, &evt);
  }

  // writing to the potentiometer level characteristic (cccd)
  else if (p_evt_write->handle == p_cus->potentio_level_char_handles.cccd_handle) {
    if (ble_srv_is_notification_enabled(p_evt_write->data)) {
      evt.evt_type = BLE_POTENTIO_LEVEL_CHAR_NOTIFICATIONS_ENABLED;
    } else {
      evt.evt_type = BLE_POTENTIO_LEVEL_CHAR_NOTIFICATIONS_DISABLED;
    }

    p_cus->evt_handler(p_cus, &evt);
  }

  // writing to the gyro level characteristic (cccd)
  else if (p_evt_write->handle == p_cus->gyro_level_char_handles.cccd_handle) {
    if (ble_srv_is_notification_enabled(p_evt_write->data)) {
      evt.evt_type = BLE_GYRO_LEVEL_CHAR_NOTIFICATIONS_ENABLED;
    } else {
      evt.evt_type = BLE_GYRO_LEVEL_CHAR_NOTIFICATIONS_DISABLED;
    }

    p_cus->evt_handler(p_cus, &evt);
  }

  // writing to the accr level characteristic (cccd)
  else if (p_evt_write->handle == p_cus->accr_level_char_handles.cccd_handle) {
    if (ble_srv_is_notification_enabled(p_evt_write->data)) {
      evt.evt_type = BLE_ACCR_LEVEL_CHAR_NOTIFICATIONS_ENABLED;
    } else {
      evt.evt_type = BLE_ACCR_LEVEL_CHAR_NOTIFICATIONS_DISABLED;
    }

    p_cus->evt_handler(p_cus, &evt);
  }
}

/**@brief Function for handling the Custom servie ble events.
 *
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
void ble_cus_on_ble_evt(ble_evt_t const *p_ble_evt, void *p_context) {
  ble_cus_t *p_cus = (ble_cus_t *)p_context;

  switch (p_ble_evt->header.evt_id) {
  case BLE_GATTS_EVT_WRITE:
    on_write(p_cus, p_ble_evt);
    break;

  default:
    // No implementation needed.
    break;
  }
}

/**@brief Function for initializing the Custom ble service.
 *
 * @param[in]   p_cus       Custom service structure.
 * @param[in]   p_cus_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_init(ble_cus_t *p_cus, const ble_cus_init_t *p_cus_init) {

  uint32_t err_code;
  ble_uuid_t ble_uuid;
  ble_add_char_params_t add_char_params;

  /* Adding the service */

  // Initialize service structure.
  p_cus->evt_handler = p_cus_init->evt_handler;
  p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;

  // Add the Custom ble Service UUID
  ble_uuid128_t base_uuid = CUS_SERVICE_UUID_BASE;
  err_code = sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  ble_uuid.type = p_cus->uuid_type;
  ble_uuid.uuid = CUS_SERVICE_UUID;

  // Add the service to the database
  err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  /* Adding the service characteristics */
  // Add the gryo characteristic.

  uint8_t accr_char_init_value[6] ={0};

  memset(&add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = ACCR_STATES_CHAR_UUID;
  add_char_params.uuid_type = p_cus->uuid_type;

  add_char_params.init_len = 6; // (in bytes)
  add_char_params.max_len = 6;
  add_char_params.p_init_value = accr_char_init_value;

  add_char_params.char_props.read = 1;
  add_char_params.char_props.notify = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.cccd_write_access = SEC_OPEN;

  err_code = characteristic_add(p_cus->service_handle,
      &add_char_params,
      &p_cus->accr_level_char_handles);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  // Add the gryo characteristic.

  uint8_t gyro_char_init_value[6] ={0};

  memset(&add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = GYRO_STATES_CHAR_UUID;
  add_char_params.uuid_type = p_cus->uuid_type;

  add_char_params.init_len = 6; // (in bytes)
  add_char_params.max_len = 6;
  add_char_params.p_init_value = gyro_char_init_value;

  add_char_params.char_props.read = 1;
  add_char_params.char_props.notify = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.cccd_write_access = SEC_OPEN;

  err_code = characteristic_add(p_cus->service_handle,
      &add_char_params,
      &p_cus->gyro_level_char_handles);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }


  // Add the buttons characteristic.

  uint8_t buttons_char_init_value[4] = {0};

  memset(&add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = BUTTONS_STATES_CHAR_UUID;
  add_char_params.uuid_type = p_cus->uuid_type;

  add_char_params.init_len = 4;
  add_char_params.max_len = 4;
  add_char_params.p_init_value = buttons_char_init_value;

  add_char_params.char_props.read = 1;
  add_char_params.char_props.notify = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.cccd_write_access = SEC_OPEN;

  err_code = characteristic_add(p_cus->service_handle,
      &add_char_params,
      &p_cus->buttons_states_char_handles);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  // Add the potentio characteristic.

  uint8_t pot_char_init_value[4] = {0};

  memset(&add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = POTENTIO_LEVEL_CHAR_UUID;
  add_char_params.uuid_type = p_cus->uuid_type;

  add_char_params.init_len = 4; // (in bytes)
  add_char_params.max_len = 4;
  add_char_params.p_init_value = pot_char_init_value;

  add_char_params.char_props.read = 1;
  add_char_params.char_props.notify = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.cccd_write_access = SEC_OPEN;

  err_code = characteristic_add(p_cus->service_handle,
      &add_char_params,
      &p_cus->potentio_level_char_handles);
  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  // Add the leds characteristic.

  uint8_t leds_char_init_value[2] = {0};

  memset(&add_char_params, 0, sizeof(add_char_params));
  add_char_params.uuid = LEDS_STATES_CHAR_UUID;
  add_char_params.uuid_type = p_cus->uuid_type;

  add_char_params.init_len = 2; // (in bytes)
  add_char_params.max_len = 2;
  add_char_params.p_init_value = leds_char_init_value;

  add_char_params.char_props.read = 1;
  add_char_params.char_props.write = 1;

  add_char_params.read_access = SEC_OPEN;
  add_char_params.write_access = SEC_OPEN;

  err_code = characteristic_add(p_cus->service_handle,
      &add_char_params,
      &p_cus->leds_states_char_handles);

  if (err_code != NRF_SUCCESS) {
    return err_code;
  }

  return NRF_SUCCESS;
}

/**@brief Function for updating the potentio level value on the potentio ble characteristic.
 *
 * @param[in]   p_cus           Custom service structure.
 * @param[in]   p_potentio_level  Potentiometer level.
 * @param[in]   conn_handle     Connection handle.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_potentio_level_update(ble_cus_t *p_cus, uint8_t *p_potentio_level, uint16_t conn_handle) {
  ble_gatts_hvx_params_t params;
  uint16_t len = sizeof(p_potentio_level);
  
  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = p_cus->potentio_level_char_handles.value_handle;
  params.p_data = p_potentio_level;
  params.p_len = &len;

  return sd_ble_gatts_hvx(conn_handle, &params);
}

/**@brief Function for updating the buttons states on the buttons ble characteristic.
 *
 * @param[in]   p_cus             Custom service structure.
 * @param[in]   p_buttons_states  Buttons states.
 * @param[in]   conn_handle       Connection handle.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_buttons_states_update(ble_cus_t *p_cus, uint8_t *p_buttons_states, uint16_t conn_handle) {
  ble_gatts_hvx_params_t params;
  uint16_t len = sizeof(p_buttons_states);

  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = p_cus->buttons_states_char_handles.value_handle;
  params.p_data = p_buttons_states;
  params.p_len = &len;

  return sd_ble_gatts_hvx(conn_handle, &params);
}

/**@brief Function for updating the Gyro states on the Gyro ble characteristic.
 *
 * @param[in]   p_cus             Custom service structure.
 * @param[in]   gyroX             Gyro X state.
 * @param[in]   gyroY             Gyro Y state.
 * @param[in]   gyroZ             Gyro Z state.
 * @param[in]   conn_handle       Connection handle.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_gyro_states_update(ble_cus_t *p_cus, int16_t gyroX, int16_t gyroY, int16_t gyroZ, uint16_t conn_handle) {
  uint8_t gyro_level[6] = {0};

  gyro_level[0] = gyroZ & 0xFF;
  gyro_level[1] = gyroZ >> 8;
  gyro_level[2] = gyroY & 0xFF;
  gyro_level[3] = gyroY >> 8;
  gyro_level[4] = gyroX & 0xFF;
  gyro_level[5] = gyroX >> 8;

  ble_gatts_hvx_params_t params;
  uint16_t len = sizeof(gyro_level);
  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = p_cus->gyro_level_char_handles.value_handle;
  params.offset = 0;
  params.p_data = gyro_level;
  params.p_len = &len;

  return sd_ble_gatts_hvx(conn_handle, &params);
}


/**@brief Function for updating the Accr states on the Accr ble characteristic.
 *
 * @param[in]   p_cus             Custom service structure.
 * @param[in]   accrX             Accr X state.
 * @param[in]   accrY             Accr Y state.
 * @param[in]   accrZ             Accr Z state.
 * @param[in]   conn_handle       Connection handle.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

uint32_t ble_cus_accr_states_update(ble_cus_t *p_cus, int16_t accrX, int16_t accrY, int16_t accrZ, uint16_t conn_handle) {
  uint8_t accr_level[6] = {0};

  accr_level[0] = accrZ & 0xFF;
  accr_level[1] = accrZ >> 8;
  accr_level[2] = accrY & 0xFF;
  accr_level[3] = accrY >> 8;
  accr_level[4] = accrX & 0xFF;
  accr_level[5] = accrX >> 8;

  ble_gatts_hvx_params_t params;
  uint16_t len = sizeof(accr_level);
  memset(&params, 0, sizeof(params));
  params.type = BLE_GATT_HVX_NOTIFICATION;
  params.handle = p_cus->accr_level_char_handles.value_handle;
  params.offset = 0;
  params.p_data = accr_level;
  params.p_len = &len;

  return sd_ble_gatts_hvx(conn_handle, &params);
}
