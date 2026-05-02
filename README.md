# Wheeliebot

A small self-balancing, wheel-legged robot driven from your phone over WiFi. Two brushless gimbal motors run the wheels, two STS3032 serial-bus servos lift the legs, an MPU-6050 keeps it upright, and an ESP32 ties it all together. It self-balances within ~2 seconds of being lifted off the ground — and yes, it jumps.

Full build (BOM, prints, photos, assembly): **[Wheeliebot on Printables](https://www.printables.com/model/1475637)**

> ⚠️ **2026-05 — controller PCB v2 in progress.** I found design issues with the current controller PCB and I'm working on a revision. Firmware and assembly steps below are still accurate; **don't fab boards from this repo until v2 lands.**

This firmware is derived from the open-source **[Navbot-EN01](https://github.com/fuwei007)** project by **fuwei007**. The chassis, controller PCB, and documentation are my adaptation; the firmware and FD calibration tooling come from the upstream project.

## What's in this repo

```
wl_pro_robot/
  wl_pro_robot.ino       # main sketch (entry point)
  robot.{h,cpp}          # balance + drive control
  cpu0_task.{h,cpp}      # tasks pinned to ESP32 core 0
  wifi.{h,cpp}           # WiFi AP + control endpoint
  web_socket_client_util.*  # WebSocket transport for the phone UI
  basic_web.h            # the web UI served to your phone (HTML/JS blob)
  ble.{h,cpp}            # BLE control path
  Servo_STS3032.{h,cpp}  # STS3032 leg-servo driver (serial bus)
  ssd1362.{h,cpp}        # OLED status display
  oled_font.h            # font glyphs for the OLED
  feedback_util.*        # status / feedback helpers
  eeprom_util.*          # persisted calibration in EEPROM
  define.h
  libraries/             # bundled, version-pinned dependencies — install from here
    ArduinoJson/
    SCSservo/            # STS3032 / SCS / SMS bus protocol
    MPU6050_tockn/       # IMU
    Simple_FOC/          # field-oriented control for the gimbal motors
```

> Pin assignments live in `robot.h` and `Servo_STS3032.h`. They will be promoted into a table here once the v2 PCB is finalized — pinning them in two places before the board respin is asking for drift.

## Build environment

Pinned versions (newer ESP32 cores break the bundled servo lib — **do not use a newer board package**):

| Tool | Version |
|---|---|
| Arduino IDE | 2.3.4 |
| ESP32 board package (Espressif) | **2.0.3** (pin this) |
| Target board | ESP32 Dev Module |
| Partition scheme | **No OTA (Large APP)** — default partitioning is too small for this binary |

Libraries: install from this repo's `wl_pro_robot/libraries/` directory, **not** from the Arduino Library Manager. The bundled versions are pinned against API breakage in newer releases.

## Flashing the firmware

1. Clone this repo.
2. Open `wl_pro_robot/wl_pro_robot.ino` in Arduino IDE 2.3.4.
3. In **Tools → Board Manager**, install ESP32 by Espressif **v2.0.3**.
4. Copy each folder under `wl_pro_robot/libraries/` into your Arduino `libraries/` folder, *or* let the IDE pick them up from the sketch directory.
5. Set **Tools → Board** to *ESP32 Dev Module*.
6. Set **Tools → Partition Scheme** to *No OTA (Large APP)*.
7. Connect the ESP32, select the COM port, and **Upload**.

## Servo zero-position calibration

Before first run, both STS3032 leg servos need their zero positions set and unique bus IDs (left = 1, right = 2). The Printables build guide covers this end-to-end with photos — see the **Firmware & calibration** section there.

Short version: upload the `SteeringEngineDebug` sketch from the upstream Navbot-EN01 toolchain, run the FD calibration tool, calibrate one servo at a time with the other unplugged from the bus, and assign IDs. Reconnect both when done.

> If `Tools/SteeringEngineDebug` and `Tools/FD1984-240227/` aren't present in this repo, grab them from the [upstream Navbot-EN01 repo](https://github.com/fuwei007).

## First run

1. Plug a 7.4 V 1100–1500 mAh LiPo into the XT30.
2. Power switch on. Red LED + a few seconds of self-test.
3. Phone → join WiFi network `navbot_en01-XXXXXX` (password `12345678`).
4. Open `http://192.168.1.11` in your phone browser.
5. Tap **Robot Go**, lift the bot off the ground for ~2 seconds, set it down, drive.

## Troubleshooting

- **Sketch won't compile, errors about `SCServo` or `Simple_FOC`** — you're picking up a different version from `~/Arduino/libraries`. Either delete the duplicate or move the bundled `wl_pro_robot/libraries/<name>` to override it.
- **Sketch too large / partition errors** — partition scheme isn't set to *No OTA (Large APP)*.
- **Servos twitch or don't respond after upload** — IDs collide on the serial bus. Re-run servo zero-position calibration with one servo unplugged at a time and assign IDs 1 and 2.
- **WiFi `navbot_en01-XXXXXX` doesn't appear** — power-cycle. If it still doesn't show, USB into the ESP32 and check serial for boot messages.
- **`192.168.1.11` doesn't load** — your phone is on your home WiFi, not the bot's AP. Recheck the connected network.
- **Self-balance never settles** — check MPU-6050 wiring on I²C, confirm the bot was held still during boot self-test (the IMU calibrates against gravity at startup).

## License

Derived from [Navbot-EN01](https://github.com/fuwei007) by fuwei007. License inheritance pending — see [LICENSE](LICENSE) once added.

> TODO: Confirm the upstream license and add a matching LICENSE file before this repo is considered properly published.

## Contributing / feedback

Issues and PRs welcome. If you build one, post a make on the [Printables listing](https://www.printables.com/model/1475637) — I read every comment.