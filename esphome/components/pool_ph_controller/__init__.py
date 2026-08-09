"""ESPHome external component for pool pH control logic."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL
from esphome.components import sensor as sensor_, binary_sensor as binary_sensor_, switch as switch_
from esphome.components import sensor, binary_sensor

DEPENDENCIES = []

pool_controller_ns = cg.esphome_ns.namespace('pool_controller')
PoolPhController = pool_controller_ns.class_('PoolPhController', cg.Component)

CONF_PUMP_SWITCH = 'pump_switch'
CONF_CURRENT_PH_SENSOR = 'current_ph_sensor'
CONF_ACID_DOSING_BINARY = 'acid_dosing_binary_sensor'
CONF_PUMP_MANUAL_BINARY = 'pump_manual_disabled_binary_sensor'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PoolPhController),
    cv.Optional(CONF_UPDATE_INTERVAL, default=15000): cv.uint32_t,
    cv.Optional(CONF_PUMP_SWITCH): cv.use_id(switch_.Switch),
    cv.Optional(CONF_CURRENT_PH_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_ACID_DOSING_BINARY): binary_sensor_.binary_sensor_schema(),
    cv.Optional(CONF_PUMP_MANUAL_BINARY): binary_sensor_.binary_sensor_schema(),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_UPDATE_INTERVAL])
    await cg.register_component(var, config)

    # Attach optional pump switch
    if CONF_PUMP_SWITCH in config:
        cg.add(var.set_pump_switch(config[CONF_PUMP_SWITCH]))

    # Current pH sensor
    if CONF_CURRENT_PH_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_PH_SENSOR])
        cg.add(var.set_current_ph_sensor(sens))

    # Acid dosing enabled binary sensor
    if CONF_ACID_DOSING_BINARY in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_ACID_DOSING_BINARY])
        cg.add(var.set_acid_dosing_binary_sensor(bs))

    # Pump manual disabled binary sensor
    if CONF_PUMP_MANUAL_BINARY in config:
        bs2 = await binary_sensor.new_binary_sensor(config[CONF_PUMP_MANUAL_BINARY])
        cg.add(var.set_pump_manual_disabled_binary_sensor(bs2))
