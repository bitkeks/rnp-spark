# RNP 2017 Exercise 2 - Constructing an Ethernet frame

This repo contains my presented solution to the task "write a C program which constructs a raw Ethernet frame, fill it with a custom protocol and its header and send it over the socket".

Compile it with `make`. Run it with `sudo ./eth-frame <target MAC> <interface> "<message>"` (the " are important, your shell should translate it into one argument, if you put in multiple words).

## Dissector for Wireshark
To debug your frame builder, you can use the provided `spark_dissector.lua`. Run your Wireshark with `wireshark -X lua_script:spark_dissector.lua` and it will automatically load the script.

## License
Licensed under GPLv3. You are free to use, modify and distribute the code in this repository.
