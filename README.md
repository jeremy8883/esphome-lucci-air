# esphome-lucci-air

An [ESPHome external component](https://esphome.io/components/external_components.html) that adds a `lucci_air` 433 MHz RF protocol to ESPHome's `remote_base`, letting you control Lucci Air ceiling fans from any ESP board with a 433 MHz transmitter.

This is not my code. The protocol was reverse-engineered by [Swiftwork](https://github.com/Swiftwork/esphome-external-components). This repo packages it as a standalone external component so you don't need to fork or rebuild ESPHome.

## Install

Add the component to your ESPHome YAML:

```yaml
external_components:
  - source: github://jeremy8883/esphome-lucci-air
    components: [lucci_air]

# This empty block must be present -- it tells ESPHome to actually load the
# external component and register the `lucci_air` protocol with remote_base.
lucci_air:
```

You'll need a 433 MHz transmitter (and, for the first-time setup, a receiver) wired to GPIO pins.

## Step 1 - find your fan's device ID

Each Lucci remote is paired to a 50-bit `device_id`. To capture yours, temporarily wire a 433 MHz receiver to your ESP and flash a minimal config:

```yaml
external_components:
  - source: github://jeremy8883/esphome-lucci-air
    components: [lucci_air]

lucci_air:

remote_receiver:
  pin: GPIO1  # whichever pin you wired the receiver's DATA to
  dump: lucci_air
  tolerance: 35%
  idle: 25ms
```

Open the ESPHome logs, then press a button on the physical Lucci remote near the receiver. You should see a line like:

```
[D][remote.lucci_air]: Received Lucci Air: command=light_toggle, device_id=0x2566A9A99A5A6
```

Copy the `device_id` value for each remote you want to control.

## Step 2 - transmit commands

Wire a 433 MHz transmitter to a GPIO and add a `remote_transmitter`:

```yaml
remote_transmitter:
  pin: GPIO1
  carrier_duty_percent: 100%   # 433 MHz module, not IR, so keep at 100%
```

Call `remote_transmitter.transmit_lucci_air` from any automation. eg.

```yaml
fan:
  - platform: template
    name: "Bedroom fan"
    speed_count: 6
    on_turn_off:
      - remote_transmitter.transmit_lucci_air:
          command: power_off
          device_id: 0x2566A9A99A5A6
    on_speed_set:
      - remote_transmitter.transmit_lucci_air:
          command: !lambda |-
            using esphome::remote_base::LucciAirCommand;
            switch ((int) x) {
              case 1: return LucciAirCommand::SPEED_1;
              case 2: return LucciAirCommand::SPEED_2;
              case 3: return LucciAirCommand::SPEED_3;
              case 4: return LucciAirCommand::SPEED_4;
              case 5: return LucciAirCommand::SPEED_5;
              case 6: return LucciAirCommand::SPEED_6;
              default: return LucciAirCommand::UNKNOWN;
            }
          device_id: 0x2566A9A99A5A6

button:
  - platform: template
    name: "Reverse fan direction"
    on_press:
      - remote_transmitter.transmit_lucci_air:
          command: direction
          device_id: 0x2566A9A99A5A6
```

## Supported commands

`power_off`, `light_toggle`, `direction`, `speed_cycle`, `away`, `speed_1` … `speed_6`, `timer_1h`, `timer_4h`, `timer_8h`.

Notes:
- `power_off` only powers the fan off, `speed_1`...`speed_6` turns it on.
- `light_toggle` toggles the light; the protocol has no "on" or "off" form, so track local state if you want a non-toggle switch in Home Assistant.
