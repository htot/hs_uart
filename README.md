# Edison high speed uart tester
This software uses the high speed uart of the Edison. It sends and receives data at 2Mb/sec. with parity even. It uses preambles and STX to indicate start of message and ETX to indicate the end of message. Binary data is encoded as text using base64 to allow detection of transmission errors. Memory mapped gpio pins 2 and 7 on the Edison arduino board are toggled when the uart is written and received message is decoded. 

The program will run when RX and TX (pin 0,1) are connected together. 2 Edisons can be connected together to send and receive each others data.


## Building

To compile just type:

```sh
make hs_serial
```
## License

See the LICENSE file.
