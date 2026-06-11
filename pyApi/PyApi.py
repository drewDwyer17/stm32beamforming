from enum import IntEnum

import serial


DEFAULT_COM_PORT = "COM3"
DEFAULT_BAUD_RATE = 115200


class CommandID(IntEnum):
    PHASE_SHIFT = 0x01
    VGA = 0x02


# Mirrors SupportedAttenuationCommand_t in app/include/Vga.h.
SUPPORTED_ATTENUATION_VALUES = {
    0: 0b00000000,
    1: 0b00000100,
    2: 0b00001000,
    4: 0b00010000,
    8: 0b00100000,
    16: 0b01000000,
    22: 0b01011000,
    23: 0b01100000,
}


class Stm32BeamformerPyAPI:

    def __init__(self, com_port=DEFAULT_COM_PORT, baud_rate=DEFAULT_BAUD_RATE):
        self.default_com_port = com_port
        self.default_baud_rate = baud_rate
        self.serial_port = None
        self.command_vga = None
        self.command_phase_shift = None

    def connect_serial(self, com_port=None, baud_rate=None):
        self.com_port = com_port or self.default_com_port
        self.baud_rate = baud_rate or self.default_baud_rate

        if self.serial_port and self.serial_port.is_open:
            return

        self.serial_port = serial.Serial(self.com_port, self.baud_rate)

    def disconnect_serial(self):
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
            print("Serial port closed.")

    def send_uart(self, uart_frame):
        if not self.serial_port or not self.serial_port.is_open:
            raise RuntimeError("Serial port is not open. Call connect_serial() first.")

        self.serial_port.write(uart_frame)

    def frame_uart_command_fixedLength(self, command_id, payload):
        if command_id == VGA: 
            fixed_length = 1
        elif command_id == PHASE_SHIFT:
            fixed_length = 4 # edit the length as required but we need some way to make sure that we don't overflow the bfufer on the firmware side 
        # Frame format: [command_id][payload...].
        # The firmware checks the message identifier to choose the parser.
        return bytes([command_id]) + bytes(payload)

    def send_vga_uart(self, attenuation_db):
        if attenuation_db not in SUPPORTED_ATTENUATION_VALUES:
            supported = ", ".join(str(value) for value in SUPPORTED_ATTENUATION_VALUES)
            raise ValueError(f"Unsupported VGA attenuation. Supported dB values: {supported}")

        attenuation_command = SUPPORTED_ATTENUATION_VALUES[attenuation_db]
        self.command_vga = self.frame_uart_command(CommandID.VGA, [attenuation_command])
        self.send_uart(self.command_vga)
 
    def send_phase_shifter_uart(self, requested_shift_deg, opt_bit=False, unit_address_word=0):
        if not 0.0 <= requested_shift_deg < 360.0:
            wrapped_shift = requested_shift_deg % 360.0
            print(f"Warning: requested_shift_deg {requested_shift_deg} is out of range. Wrapped to {wrapped_shift}.")
            requested_shift_deg = wrapped_shift

        if not 0 <= unit_address_word <= 0x0F:
            raise ValueError("unit_address_word must fit in 4 bits.")

        phase_centidegrees = round(requested_shift_deg * 100)
        payload = [
            (phase_centidegrees >> 8) & 0xFF,
            phase_centidegrees & 0xFF,
            int(bool(opt_bit)),
            unit_address_word,
        ]

        # Firmware should parse this payload and call MakePSCommand().
        self.command_phase_shift = self.frame_uart_command(CommandID.PHASE_SHIFT, payload)
        self.send_uart(self.command_phase_shift)


if __name__ == "__main__":
    beamformer = Stm32BeamformerPyAPI()
    beamformer.connect_serial()

    try:
        beamformer.send_vga_uart(8)
        beamformer.send_phase_shifter_uart(205.3, opt_bit=False, unit_address_word=0b0011)
    finally:
        beamformer.disconnect_serial()
