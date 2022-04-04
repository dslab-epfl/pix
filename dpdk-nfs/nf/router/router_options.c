#include "router_options.h"

bool
handle_packet_timestamp(struct ipv4_hdr* header, uint32_t router_ip, uint32_t current_milliseconds_utc)
{
	uint8_t options_length = (header->version_ihl & 0xF) - 5;
	uint8_t* options_bytes = (uint8_t*) ((uint32_t*) header + 5);
	uint8_t options_bytes_count = options_length * 4;
	while (options_bytes_count > 0) {
		// Option type
		uint8_t opt_type = *options_bytes;
		options_bytes++;

		if (opt_type == 0) {
			// End of options
			break;
		}

		if (opt_type == 1) {
			// No-op
			continue;
		}

		// Option length
		uint8_t opt_length = *options_bytes;
		options_bytes++;

		// We're only interested in the timestamp
		if (opt_type != 68) {
			// -2 since we skipped type and length already
			options_bytes += opt_length - 2;
			continue;
		}

		// OK, we have a timestamp option; now, let's look at the pointer to see if we can do something
		// Remember pointer pointer (ha...) for later
		uint8_t* timestamp_pointer_ptr = options_bytes;
		uint8_t timestamp_pointer = *options_bytes;
		options_bytes++;
		if (timestamp_pointer > opt_length) {
			// Timestamp full!
			// Let's increment the overflow instead
			uint8_t timestamp_overflow = (*options_bytes & 0xF0) >> 4;
			if (timestamp_overflow == 0xF) {
				// Overflow overflow... discard packet, as per RFC 791
				return false;
			}

			// Increment overflow, continue
			uint8_t timestamp_flag = *options_bytes & 0xF;
			*options_bytes = ((timestamp_overflow + 1) << 4) + timestamp_flag;
			*options_bytes++;

			// Skip remaining bytes; -2 because we counted type and length, -2 because we counter pointer and overflow/flag
			*options_bytes += (opt_length - 2 - 2);
			continue;
		}


		// We have a timestamp option, and it's not full!
		// ...or is it? Still need to check whether we have space to write stuff

		// First get the flag, to know whether timestamps are prefixed with addresses
		uint8_t timestamp_flag = *options_bytes & 0xF;
		*options_bytes++;

		// timestamp pointer -1 cause it has a +1 according to the spec
		uint8_t remaining_length = opt_length - (timestamp_pointer - 1);
		uint8_t required_length = timestamp_flag == 0 ? 4 : 8;
		if (remaining_length < required_length) {
			// Not enough space, drop packet as per RFC 791
			return false;
		}

		// OK, so we have enough space; let's go write!
		// Remove 5 from the pointer since we have already skipped over type, length, pointer, overflow/flag, and pointer has a +1
		options_bytes += (timestamp_pointer - 5);

		// May be unaligned. Have fun dealing with _that_, contract writer! :D
		uint32_t* timestamp_bytes = (uint32_t*) options_bytes;

		// ... but first, if the tag is 3, addresses are pre-determined, so let's check whether we are supposed to write
		if (timestamp_flag == 3) {
			if (*timestamp_bytes != router_ip) {
				// Not for us
				continue;
			}

			timestamp_bytes++;
		}

		// If flag is 1, write our IP
		if (timestamp_flag == 1) {
			*timestamp_bytes = router_ip;
			timestamp_bytes++;
		}

		// Write the timestamp
		*timestamp_bytes = current_milliseconds_utc;

		// Increment the pointer
		*timestamp_pointer_ptr = *timestamp_pointer_ptr + required_length;

		// Then, skip over the remaining bytes (-2 because the timestamp pointer is the 3rd byte)
		options_bytes = timestamp_pointer_ptr + opt_length - 2;
	}

	// If we reached this, we're good!
	return true;
}
