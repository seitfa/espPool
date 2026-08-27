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
CONF_TARGET_PH_SENSOR = 'target_ph_sensor'
CONF_POOL_VOLUME_SENSOR = 'pool_volume_sensor'
CONF_TAC_SENSOR = 'tac_sensor'
CONF_ACID_STRENGTH_SENSOR = 'acid_strength_sensor'
CONF_DOSING_FLOWRATE_SENSOR = 'dosing_flowrate_sensor'
CONF_MAX_ACID_SENSOR = 'max_acid_sensor'
CONF_DAILY_ACID_USED_SENSOR = 'daily_acid_used_sensor'
CONF_ACID_ML_NEEDED_SENSOR = 'acid_ml_needed_sensor'
CONF_ACID_DOSING_BINARY = 'acid_dosing_binary_sensor'

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PoolPhController),
    cv.Optional(CONF_UPDATE_INTERVAL, default=15000): cv.uint32_t,
    cv.Optional(CONF_PUMP_SWITCH): cv.use_id(switch_.Switch),
    cv.Optional(CONF_CURRENT_PH_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_TARGET_PH_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_POOL_VOLUME_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_TAC_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_ACID_STRENGTH_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_DOSING_FLOWRATE_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_MAX_ACID_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_DAILY_ACID_USED_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_ACID_ML_NEEDED_SENSOR): sensor_.sensor_schema(),
    cv.Optional(CONF_ACID_DOSING_BINARY): binary_sensor_.binary_sensor_schema(),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_UPDATE_INTERVAL])
    await cg.register_component(var, config)

    if CONF_PUMP_SWITCH in config:
        pump = await cg.get_variable(config[CONF_PUMP_SWITCH])
        cg.add(var.set_pump_switch(pump))

    if CONF_CURRENT_PH_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_CURRENT_PH_SENSOR])
        cg.add(var.set_current_ph_sensor(sens))

    if CONF_TARGET_PH_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_TARGET_PH_SENSOR])
        cg.add(var.set_target_ph_sensor(sens))

    if CONF_POOL_VOLUME_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_POOL_VOLUME_SENSOR])
        cg.add(var.set_pool_volume_sensor(sens))

    if CONF_TAC_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_TAC_SENSOR])
        cg.add(var.set_tac_sensor(sens))

    if CONF_ACID_STRENGTH_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_ACID_STRENGTH_SENSOR])
        cg.add(var.set_acid_strength_sensor(sens))

    if CONF_DOSING_FLOWRATE_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_DOSING_FLOWRATE_SENSOR])
        cg.add(var.set_dosing_flowrate_sensor(sens))

    if CONF_MAX_ACID_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_MAX_ACID_SENSOR])
        cg.add(var.set_max_acid_sensor(sens))

    if CONF_DAILY_ACID_USED_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_DAILY_ACID_USED_SENSOR])
        cg.add(var.set_daily_acid_used_ml_sensor(sens))

    if CONF_ACID_ML_NEEDED_SENSOR in config:
        sens = await sensor.new_sensor(config[CONF_ACID_ML_NEEDED_SENSOR])
        cg.add(var.set_acid_ml_needed_sensor(sens))

    if CONF_ACID_DOSING_BINARY in config:
        bs = await binary_sensor.new_binary_sensor(config[CONF_ACID_DOSING_BINARY])
        cg.add(var.set_acid_dosing_binary_sensor(bs))

