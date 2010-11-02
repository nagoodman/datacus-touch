Touch2Tuio

A small program to translate and forward native Windows 7 touch messages to TUIO clients.

Requirements:

    * Multi-touch hardware supporting the Windows 7 API.
    * A TUIO-aware client application listening on the default port (3333)

Disclaimer:

This is very much work in progress and probably should not be used for production environments. We have tested it with our hardware and client applications and it worked fine for us but we do not guarantee that is runs with all hardware/software or runs stable in all situations.

Manual:

The current interface is command line only and there are not many options available as of yet. We plan to expose further options, e.g., the TUIO port in the future. If you need more options right now, feel free to add them yourself (that's the beauty of the open source model) and let us know about it!

1.) Start your TUIO-aware client application and write down the main window's name (for full screen applications you might have to check in the task manager).

2.) Start Touch2Tuio and provide the full name (capitalization doesn't matter) of the client window as an argument, e.g., "touch2tuio.exe mytuioclientapp".

Touch2Tuio uses Windows Hooks to grab the touch messages from the client application and to disable the default touch mouse emulation.

Note:

You need the 32-bit Touch2Tuio version for 32-bit TUIO clients (even on 64-bit systems) and 64-bit Touch2Tuio version for 64-bit TUIO clients.

Contact:

Marc or Benjamin
http://dm.tzi.de/research/hci/touch2tuio/