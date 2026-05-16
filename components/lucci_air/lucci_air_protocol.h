#pragma once

#include "esphome/components/remote_base/remote_base.h"
#include <string>
#include <map>

namespace esphome {
namespace remote_base {

struct LucciAirData {
  std::string command;  // Command name
  uint64_t device_id;   // 50-bit device ID
  bool operator==(const LucciAirData &rhs) const {
    return command == rhs.command && device_id == rhs.device_id;
  }
};

class LucciAirProtocol : public RemoteProtocol<LucciAirData> {
 public:
  void encode(RemoteTransmitData *dst, const LucciAirData &data) override;
  optional<LucciAirData> decode(RemoteReceiveData src) override;
  void dump(const LucciAirData &data) override;

  static uint32_t get_command_start(const std::string &command);
  static uint32_t get_command_end(const std::string &command);
  static std::string get_command_name(uint32_t command_value);

 private:
  void encode_bit(RemoteTransmitData *dst, bool value, bool mark) const;
  void encode_signal_with_command(RemoteTransmitData *dst, uint32_t command, uint64_t device_id);
  optional<bool> decode_bit(RemoteReceiveData &src, bool is_mark, uint8_t bit_position) const;

  static const std::map<std::string, uint32_t> COMMANDS;
  static const uint32_t COMMAND_END_MASK = 0x00030003;
};

DECLARE_REMOTE_PROTOCOL(LucciAir)

template<typename... Ts> class LucciAirAction : public RemoteTransmitterActionBase<Ts...> {
 public:
  TEMPLATABLE_VALUE(std::string, command)
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
