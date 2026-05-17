"""Lucci Air ceiling fan 433 MHz protocol for ESPHome.

Registers a `lucci_air` protocol with esphome's built-in `remote_base`,
making it available to `remote_transmitter` actions and `remote_receiver`
binary sensors / triggers / dumpers.
"""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.remote_base import (
    declare_protocol,
    register_action,
    register_binary_sensor,
    register_dumper,
    register_trigger,
)
from esphome.const import CONF_COMMAND, CONF_DEVICE_ID

CODEOWNERS = ["@jeremy8883"]
DEPENDENCIES = ["remote_base"]

# Empty schema -- presence of `lucci_air:` in the user's YAML is what triggers
# this module's import (and the @register_* decorators below) so that
# `remote_transmitter.transmit_lucci_air` becomes a known action.
CONFIG_SCHEMA = cv.Schema({})


async def to_code(config):
    pass


# C++ types live in the existing `esphome::remote_base` namespace.
lucci_air_ns = cg.esphome_ns.namespace("remote_base")

LucciAirCommand = lucci_air_ns.enum("LucciAirCommand", is_class=True)

LUCCI_AIR_COMMANDS = {
    "direction": LucciAirCommand.DIRECTION,
    "speed_1": LucciAirCommand.SPEED_1,
    "speed_2": LucciAirCommand.SPEED_2,
    "speed_3": LucciAirCommand.SPEED_3,
    "speed_4": LucciAirCommand.SPEED_4,
    "speed_5": LucciAirCommand.SPEED_5,
    "speed_6": LucciAirCommand.SPEED_6,
    "power_off": LucciAirCommand.POWER_OFF,
    "timer_1h": LucciAirCommand.TIMER_1H,
    "timer_4h": LucciAirCommand.TIMER_4H,
    "timer_8h": LucciAirCommand.TIMER_8H,
    "light_toggle": LucciAirCommand.LIGHT_TOGGLE,
    "speed_cycle": LucciAirCommand.SPEED_CYCLE,
    "away": LucciAirCommand.AWAY,
}

(
    LucciAirData,
    LucciAirBinarySensor,
    LucciAirTrigger,
    LucciAirAction,
    LucciAirDumper,
) = declare_protocol("LucciAir")

LUCCI_AIR_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_COMMAND): cv.enum(LUCCI_AIR_COMMANDS, lower=True),
        cv.Required(CONF_DEVICE_ID): cv.hex_uint64_t,
    }
)


@register_binary_sensor("lucci_air", LucciAirBinarySensor, LUCCI_AIR_SCHEMA)
def lucci_air_binary_sensor(var, config):
    cg.add(
        var.set_data(
            cg.StructInitializer(
                LucciAirData,
                ("command", config[CONF_COMMAND]),
                ("device_id", config[CONF_DEVICE_ID]),
            )
        )
    )


@register_trigger("lucci_air", LucciAirTrigger, LucciAirData)
def lucci_air_trigger(var, config):
    pass


@register_dumper("lucci_air", LucciAirDumper)
def lucci_air_dumper(var, config):
    pass


@register_action("lucci_air", LucciAirAction, LUCCI_AIR_SCHEMA)
async def lucci_air_action(var, config, args):
    template_ = await cg.templatable(config[CONF_COMMAND], args, LucciAirCommand)
    cg.add(var.set_command(template_))
    template_ = await cg.templatable(config[CONF_DEVICE_ID], args, cg.uint64)
    cg.add(var.set_device_id(template_))
