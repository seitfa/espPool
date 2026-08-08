"""ESPHome external component for pool pH control logic."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

DEPENDENCIES = []

pool_controller_ns = cg.esphome_ns.namespace('pool_controller')
PoolPhController = pool_controller_ns.class_('PoolPhController', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PoolPhController),
    cv.Optional(CONF_UPDATE_INTERVAL, default=15000): cv.uint32_t,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_UPDATE_INTERVAL])
    await cg.register_component(var, config)
