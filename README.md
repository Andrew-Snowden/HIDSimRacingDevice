# HIDSimRacingDevice
ATMega32u4 programmed to work as a sequential shifter, clutch, brake, accelerator, and handbrake over USB HID

ATMega32u4 is programmed to be recognized as a HID device with 4 axes and 6 buttons. All programming was done baremetal, direct-register access. No libraries were used.
Another project will be done to make a FFB steering wheel

There exists code that reads position from a rotary encoder but currently that isn't transmitted over HID. That code will be used with the FFB wheel.
