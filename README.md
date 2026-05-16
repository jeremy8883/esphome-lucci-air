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

Open the ESPHome logs, then press a button (e.g. power) on the physical Lucci remote near the receiver. You should see a line like:

```
[D][remote.lucci_air]: Received Lucci Air: command=power, device_id=0x2566A9A99A5A6
```

Copy the `device_id` value for each remote you want to control.

## Step 2 - transmit commands

Wire a 433 MHz transmitter to a GPIO and add a `remote_transmitter`:

```yaml
remote_transmitter:
  pin: GPIO1
  carrier_duty_percent: 100%   # 433 MHz module, not IR, so keep at 100%
```

Then call `remote_transmitter.transmit_lucci_air` from any automation:

```yaml
button:
  - platform: template
    name: "Fan power"
    on_press:
      - remote_transmitter.transmit_lucci_air:
          command: power
          device_id: 0x2566A9A99A5A6
```

## Supported commands

`power`, `light`, `direction`, `speed_cycle`, `away`, `speed_1` … `speed_6`, `timer_1h`, `timer_4h`, `timer_8h`.

Note: `light` is a toggle, so you'll need to track your own optimistic state if you want to know whether it's currently on.
