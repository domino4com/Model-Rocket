# Model Rocket
Firmware for Model Rocket

## xChips in Use
| Name | Description | ChipSet | Notes |
| :--: | :-- | :-- | :-- |
| CWA | Standard Core | ESP32-WROOM-32 4Mb ||
| IIA | Accelerometer | KXTJ3-1057 ||
| IWB | Barometer | SPL06-001 ||
| PB02 | CoinCell Power ||Remember to get 2032 Coin Cell batteries |
| PPU | Power and Programmer || Only needed for programming the core |

## Harware Setup
1. Connect the CWA to the IIA, IWB, PB02 and PPU with the CWA at the top in a vertical column
1. Plug the kit into your computer using the PPU
1. Start a serial console/terminal on your computer (115200-N-8-1). This is not necessary, but helps with debugging.
1. Notice the LED colors when performing each step, then you can avoid having a computer connected for the real flight.
1. Go through each step below several times before the real flight takes place, and only then do the real flight, once you are fully familiar with the operation


## Step 1: Prepare
1. Click ⬅︎ and ➡ arrows on the core.
2. The core will format the internal memory. This will take 34-35 seconds.

## Step 2: Arm and Launch
1. Click on the 🤖 icon on the core. This will ready the rocket for launch. 
1. The core will sleep and wake up every 5 seconds, to check if the rocket is vertical. Keep the rocket horizontal until the launch pad and then only there make the rocket vertical.
1. Right now it starts recording immidiately, but in the future it will only start recording when the rocket is launched. And it will record until memory is full!

## Step 3: Download
1. Install a serial console/terminal that got Ymodem:
  MacOs: [serial](https://apps.apple.com/za/app/serial/id877615577?mt=12)
1. Connect to the the Serial port: `115200-N-8-1`
2. Click ⬆︎ and ⬇︎ arrown on the Core
3. When the the console says `Start the YMODEM download on your computer!`, start Ymodem.
4. Once downloaded, convert the `data.bin` file to `data.csv` using bin2csv.py Python program:
  `python bin2csv.py -i data.bin -o data.csv`

## Data in numbers
- Total formatted memory: 2581KB
- A flight recording will typical show:
  - Memory: 2468083 bytes used.
  - Time: 107047 ms ~ 107 seconds ~ 1 min, 47 sec
  - So ~1ms per record
- After download:
  - Onboard file binary file: 2.3MB
  - Converted CSV file (Altitude and Magnitude added): 14.3MB
  - **102 002** actual records
  - Recorded time: 104 684 ms = **1 min, 44 sec**, (Will be less since the final buffer flushing will fail)
  - Download time with Ymodem: **~7 Min**
  