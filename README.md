# Ethernet Protocol and ARQ Implementation

This repository contains the implementation of an Ethernet-like protocol and a Stop-and-Wait Automatic Repeat Request (ARQ) system using Arduino. The project focuses on data link layer communication, error detection, and handling acknowledgments for reliable data transfer.

## Files in the Repository

- **`RX_LAB4.ino`**: Arduino implementation of the receiver logic for the Ethernet protocol and Stop-and-Wait ARQ.
- **`TX_LAB4.ino`**: Arduino implementation of the transmitter logic for the Ethernet protocol and Stop-and-Wait ARQ.
- **`EthernetLab.h`**: Header file providing CRC32 calculation and mode management functions.
- **`Lab file.pdf`**: Detailed instructions for implementing Ethernet-like frames and the Stop-and-Wait ARQ protocol.
- **`design.pdf`**: Flowcharts and design documentation for the Ethernet and ARQ protocols.

## Project Overview

### Ethernet-Like Protocol
This implementation mimics Ethernet frame structures but is simplified for Arduino's capabilities. Frames include fields for addressing, type, length, payload, and error detection.

#### Frame Structure
- **Destination Address**: 1 byte.
- **Source Address**: 1 byte.
- **Type**: 1 byte (reserved for VLAN tags, set to 0).
- **Length**: 1 byte (payload size in bytes).
- **Payload**: Variable-length data.
- **Frame Check Sequence (FCS)**: 4 bytes of CRC32 for error detection.

#### CRC32
The `EthernetLab.h` file provides a `calculateCRC` function to compute CRC32 for error detection. The checksum is appended to frames before transmission.

### Stop-and-Wait ARQ Protocol
The ARQ protocol ensures reliable data transfer by implementing acknowledgment and retransmission mechanisms. It operates as follows:
1. **Sender**:
   - Sends a frame and waits for an acknowledgment (ACK).
   - Retransmits the frame if no ACK is received within a timeout period.
2. **Receiver**:
   - Validates the frame using CRC32 and checks the sequence number.
   - Sends an ACK for valid frames and ignores duplicates.

#### RTT and Error Probability
- **RTT Calculation**: Round-trip time is measured from frame transmission to ACK reception. It is averaged for dynamic timeout adjustment.
- **Error Probability**: Measured as the ratio of bad frames to total frames received. The receiver maintains counters for error statistics.

## Implementation Details

### Functions in `EthernetLab.h`
- **`calculateCRC(char* payload, int payload_size)`**: Computes the CRC32 of the payload.
- **`setMode(int mod)`**: Sets the mode (transmitter or receiver) for CRC calculation:
  - `0`: Transmitter mode.
  - `1`: Receiver mode.

### Key Features
- **Ethernet Frames**: Implements variable-length frames with CRC-based error detection.
- **Stop-and-Wait ARQ**: Ensures reliable data transfer through retransmission and acknowledgment.
- **Inter-Frame Gap (IFG)**: Enforces a delay between frame transmissions.

## Getting Started

1. **Hardware Setup**: Connect two Arduino boards via UART or a similar serial communication channel.
2. **Upload Code**: Use `TX_LAB4.ino` on the sender Arduino and `RX_LAB4.ino` on the receiver Arduino.
3. **Test Transmission**: Verify frame structure, error detection, and acknowledgment handling.
4. **Measure Performance**:
   - Compute average RTT for dynamic timeout adjustment.
   - Measure error probability using counters for bad and total frames.

## Design Documentation

### Transmitter States
1. **Create Package**: Initializes frame with destination, source, payload, and CRC.
2. **Send Frame**: Transmits the frame and starts the timeout timer.
3. **Wait for ACK**: Awaits acknowledgment and retransmits if the timer expires.

### Receiver States
1. **Receive Frame**: Reads and parses the frame.
2. **Validate Frame**: Checks CRC and sequence number.
3. **Send ACK**: Sends an acknowledgment for valid frames.

### Flowcharts
Refer to `design.pdf` for detailed state diagrams and data flow.

## Tools and References
- **CRC Basics**: [Cyclic Redundancy Check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
- **Arduino Reference**: [Arduino Documentation](https://www.arduino.cc/reference/en/)
- **CRC Calculator**: [Online CRC Calculator](https://asecuritysite.com/comms/crc_div)

## Contribution

Contributions are welcome! Please document your changes thoroughly and ensure compatibility with the existing implementation.
