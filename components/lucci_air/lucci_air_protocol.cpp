#include "lucci_air_protocol.h"
#include "esphome/core/log.h"

namespace esphome {
namespace remote_base {

static const char *const TAG = "remote.lucci_air";

/**
 * The Lucci Air ceiling fan remote sends out signals in pairs.
 * Each button press sends 5 repetitions of command start, then 5 repetitions of command end.
 * Each signal consists of 81 bits where:
 * - A narrow pulse (290us) represents a logical 0
 * - A wide pulse (875us) represents a logical 1
 * - The signal is given in reverse order, so the least significant bit is sent first
 * - Between each pulse is 5000us
 * - Between command pairs is 10000us
 *
 * Signal structure:
 * - First 50 bits: Device ID
 * - Remaining 31 bits: Command
 */

static const uint8_t SEQUENCE_LEN = 81;        // 50 device ID + 31 command bits
static const uint16_t BIT_ZERO_US = 290;       // A narrow pulse signaling logical 0
static const uint16_t BIT_ONE_US = 875;        // A wide pulse signaling logical 1
static const uint16_t GAP_US = 5000;           // Gap between signals
static const uint16_t COMMAND_GAP_US = 10000;  // Gap between command pairs

const std::map<std::string, uint32_t> LucciAirProtocol::COMMANDS = {
    {"direction", 0x1AAA9AAA}, {"speed_1", 0x16AA96AA},  {"speed_2", 0x29AAA9AA}, {"speed_3", 0x2A6AAA6A},
    {"speed_4", 0x15AA95AA},   {"speed_5", 0x25AAA5AA},  {"speed_6", 0x26AAA6AA}, {"power", 0x19AA99AA},
    {"timer_1h", 0x166A966A},  {"timer_4h", 0x266AA66A}, {"timer_8h", 0x1A6A9A6A}, {"light", 0x196A996A},
    {"speed_cycle", 0x1A9A9A9A}, {"away", 0x2A9AAA9A},
};

void LucciAirProtocol::encode_bit(RemoteTransmitData *dst, bool value, bool mark) const {
  if (value) {
    // Wide pulse = 1
    if (mark) {
      dst->mark(BIT_ONE_US);
    } else {
      dst->space(BIT_ONE_US);
    }
  } else {
    // Narrow pulse = 0
    if (mark) {
      dst->mark(BIT_ZERO_US);
    } else {
      dst->space(BIT_ZERO_US);
    }
  }
}

uint32_t LucciAirProtocol::get_command_start(const std::string &command) {
  auto it = COMMANDS.find(command);
  if (it != COMMANDS.end()) {
    return it->second;
  }
  return 0;  // Unknown command
}

uint32_t LucciAirProtocol::get_command_end(const std::string &command) {
  uint32_t start = get_command_start(command);
  return start ^ COMMAND_END_MASK;
}

std::string LucciAirProtocol::get_command_name(uint32_t command_value) {
  for (const auto &pair : COMMANDS) {
    if (pair.second == command_value || (pair.second ^ COMMAND_END_MASK) == command_value) {
      return pair.first;
    }
  }
  return "unknown";
}

void LucciAirProtocol::encode_signal_with_command(RemoteTransmitData *dst, uint32_t command, uint64_t device_id) {
  // Encode Device ID bits (0-49)
  for (uint8_t i = 0; i < 50; i++) {
    this->encode_bit(dst, device_id & (1ULL << i), i % 2 == 0);
  }

  // Encode Command bits (50-80)
  for (uint8_t i = 0; i < 31; i++) {
    this->encode_bit(dst, command & (1UL << i), (50 + i) % 2 == 0);
  }
}

void LucciAirProtocol::encode(RemoteTransmitData *dst, const LucciAirData &data) {
  ESP_LOGD(TAG, "Encoding Lucci Air signal for command: %s", data.command.c_str());

  uint32_t command_start = get_command_start(data.command);
  uint32_t command_end = get_command_end(data.command);

  if (command_start == 0) {
    ESP_LOGE(TAG, "Unknown command: %s", data.command.c_str());
    return;
  }

  // Send 5 repetitions of command start
  for (uint8_t rep = 0; rep < 5; rep++) {
    if (rep > 0) {
      dst->space(GAP_US);
    }
    this->encode_signal_with_command(dst, command_start, data.device_id);
  }

  // Gap between command pairs
  dst->space(COMMAND_GAP_US);

  // Send 5 repetitions of command end
  for (uint8_t rep = 0; rep < 5; rep++) {
    if (rep > 0) {
      dst->space(GAP_US);
    }
    this->encode_signal_with_command(dst, command_end, data.device_id);
  }
}

optional<bool> LucciAirProtocol::decode_bit(RemoteReceiveData &src, bool is_mark, uint8_t bit_position) const {
  if (is_mark) {
    if (src.peek_mark(BIT_ONE_US, bit_position)) {
      return true;
    }
    if (src.peek_mark(BIT_ZERO_US, bit_position)) {
      return false;
    }
    ESP_LOGV(TAG, "Failed to decode mark at bit %d", bit_position);
    return {};
  } else {
    if (src.peek_space(BIT_ONE_US, bit_position)) {
      return true;
    }
    if (src.peek_space(BIT_ZERO_US, bit_position)) {
      return false;
    }
    ESP_LOGV(TAG, "Failed to decode space at bit %d", bit_position);
    return {};
  }
}

optional<LucciAirData> LucciAirProtocol::decode(RemoteReceiveData src) {
  LucciAirData data{};

  // Need at least SEQUENCE_LEN
  if (src.size() < SEQUENCE_LEN) {
    return {};
  }

  uint32_t raw_command = 0;
  data.device_id = 0;

  // Decode all bits in the sequence
  for (uint8_t i = 0; i < SEQUENCE_LEN; i++) {
    bool is_mark = (i % 2 == 0);
    auto bit_result = decode_bit(src, is_mark, i);
    if (!bit_result.has_value()) {
      return {};
    }
    if (!*bit_result) {
      continue;
    }

    // Store bit in appropriate field based on position
    if (i < 50) {
      // Device ID bits (0-49)
      data.device_id |= (1ULL << i);
    } else if (i < SEQUENCE_LEN) {
      // Command bits (50-80)
      raw_command |= (1UL << (i - 50));
    }
  }

  // Convert raw command to command name
  data.command = get_command_name(raw_command);

  return data;
}

void LucciAirProtocol::dump(const LucciAirData &data) {
  ESP_LOGD(TAG, "Received Lucci Air: command=%s, device_id=0x%llX", data.command.c_str(), data.device_id);
}

}  // namespace remote_base
}  // namespace esphome
