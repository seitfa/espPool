"""ESPHome external component for Pool Config persistent settings."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

DEPENDENCIES = []

pool_config_ns = cg.esphome_ns.namespace('pool_controller')
PoolConfigComponent = pool_config_ns.class_('PoolConfigComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(PoolConfigComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
