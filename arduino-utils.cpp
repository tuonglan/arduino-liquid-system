#include <Arduino.h>
#include "arduino-utils.hh"

int read_from_console(char * const str, uint8_t size) {
  if (!Serial.available())
    return 0;

  delay(64);  // Wait for all characters to arrive
  memset(str, 0, size);      // Clear the str

  uint8_t count=0;
  while (Serial.available()) {
    char c = Serial.read();
    if (c >= 32 && count < size) {
      str[count] = c;
      count++;
    }
  }
  str[count] = '\0';

  return count;
}

int string_to_numbers(char const * const str, uint8_t str_max_size, long * const numbers, uint8_t num_size) {
  String new_str(str);
  int str_length = new_str.length();
  int found_size = 0;

  uint8_t start_idx = 0;
  uint8_t end_idx = 0;
  for (int i=0;i<num_size;++i) {
    start_idx = end_idx;
    end_idx = new_str.indexOf(' ', start_idx+1);

    String sub_str = new_str.substring(start_idx, end_idx);
    numbers[i] = sub_str.toInt();
    found_size++;
  }

  return found_size;
}
