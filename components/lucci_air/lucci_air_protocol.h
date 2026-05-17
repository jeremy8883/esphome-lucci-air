#pragma once

#include "esphome/components/remote_base/remote_base.h"

namespace esphome {
namespace remote_base {

enum class LucciAirCommand : uint8_t {
  UNKNOWN = 0,
  DIRECTION,
  SPEED_1,
  SPEED_2,
  SPEED_3,
  SPEED_4,
  SPEED_5,
  SPEED_6,
  POWER_OFF,
  TIMER_1H,
  TIMER_4H,
  TIMER_8H,
  LIGHT_TOGGLE,
  SPEED_CYCLE,
  AWAY,
};

struct LucciAirData {
  LucciAirCommand command;
  uint64_t device_id;  // 50-bit device ID
  bool operator==(const LucciAirData &rhs) const {
    return command == rhs.command && device_id == rhs.device_id;
  }
};

class LucciAirProtocol : public RemoteProtocol<LucciAirData> {
 public:
  void encode(RemoteTransmitData *dst, const LucciAirData &data) override;
  optional<LucciAirData> decode(RemoteReceiveData src) override;
  void dump(const LucciAirData &data) override;

  static const char *command_to_string(LucciAirCommand command);

 private:
  static uint32_t command_to_start_value(LucciAirCommand command);
  static LucciAirCommand value_to_command(uint32_t command_value);

  void encode_bit(RemoteTransmitData *dst, bool value, bool mark) const;
  void encode_signal_with_command(RemoteTransmitData *dst, uint32_t command, uint64_t device_id);
  optional<bool> decode_bit(RemoteReceiveData &src, bool is_mark, uint8_t bit_position) const;

  static const uint32_t COMMAND_END_MASK = 0x00030003;
};

DECLARE_REMOTE_PROTOCOL(LucciAir)

template<typename... Ts> class LucciAirAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(LucciAirCommand, command)
  TEMPLATABLE_VALUE(uint64_t, device_id)

  void encode(RemoteTransmitData *dst, Ts... x) override {
    LucciAirData data{};
    data.command = this->command_.value(x...);
    data.device_id = this->device_id_.value(x...);
    LucciAirProtocol().encode(dst, data);
  }
};

}  // namespace remote_base
}  // namespace esphome
