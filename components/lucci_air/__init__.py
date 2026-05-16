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

lucci_air_ns = cg.esphome_ns.namespace("remote_base")

(
    LucciAirData,
    LucciAirBinarySensor,
    LucciAirTrigger,
    LucciAirAction,
    LucciAirDumper,
) = declare_protocol("LucciAir")

LUCCI_AIR_COMMANDS = [
    "direction",
    "speed_1",
    "speed_2",
    "speed_3",
    "speed_4",
    "speed_5",
    "speed_6",
    "power",
    "timer_1h",
    "timer_4h",
    "timer_8h",
    "light",
    "speed_cycle",
    "away",
]

LUCCI_AIR_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_COMMAND): cv.one_of(*LUCCI_AIR_COMMANDS, lower=True),
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
    template_ = await cg.templatable(config[CONF_COMMAND], args, cg.std_string)
    cg.add(var.set_command(template_))
    template_ = await cg.templatable(config[CONF_DEVICE_ID], args, cg.uint64)
    cg.add(var.set_device_id(template_))
