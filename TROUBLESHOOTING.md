# Troubleshooting

## MacOS: on some devices, the button does nothing at all

MacOS only allows control mute/unmute on devices where it is implemented in hardware; these can not be directly supported.

One workaround is to use [Loopback] to create a virtual device that
forwards to your real device, and use that device instead for everything.

## MacOS: on some devices, muting for input also mutes output, or the other way around

The behavior here can only be directly fixed by Apple or the device manufacturer.

One workaround is to use [Loopback] to create a virtual device that
forwards to your real device, and use that device instead for everything.

## No property page, or does not work

1. 32-bit Windows is not supported.
2. If you can see the property page, open the device list; if your device is listed multiple times, you usually want the first one, but try every entry.
3. Try fully quitting the StreamDeck software and re-opening it. To fully quit, right click on the system tray icon, and select "Quit Stream Deck".
4. Check your anti-virus or other anti-malware history to make sure it has not manipulated it.
5. Try removing the action/icon and re-adding it.
6. Try uninstalling and re-installing the plugin. If this fixes the issue, check your anti-virus/anti-malware software again.

[Loopback]: https://rogueamoeba.com/loopback/
