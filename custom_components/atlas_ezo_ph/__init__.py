"""ESPHome external component for Atlas Scientific EZO pH sensors."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL
from esphome.components import i2c

DEPENDENCIES = ['i2c']

pool_controller_ns = cg.esphome_ns.namespace('pool_controller')
AtlasEzoPhSensor = pool_controller_ns.class_('AtlasEzoPhSensor', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AtlasEzoPhSensor),
    cv.Optional('address', default=0x63): cv.uint8_t,
    cv.Optional(CONF_UPDATE_INTERVAL, default=15000): cv.uint32_t,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config['address'], config[CONF_UPDATE_INTERVAL])
    await cg.register_component(var, config)
