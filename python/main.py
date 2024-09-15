from time import sleep
from si4735.si4735 import SI4735

def main():
    radio = SI4735()
    if q := radio.get_device_address():
        print(f"Device found at address {hex(q)}")
    else:
        print("Device not found")
        exit()

    radio.reset()

    radio.power_up('FM')
    radio.get_rev()
    radio.tune_fm(10110)  # Tune to 101.1 MHz
    radio.set_volume(30)
    status = radio.get_status()

    print(f"Status: {status}")
    tune_status = radio.get_tune_status()
    print(f"Tune Status: {tune_status}")
    while True:
        sleep(1)
        tune_status = radio.get_tune_status()
        print(f"Tune Status: {tune_status}")

if __name__ == "__main__":
    main()
