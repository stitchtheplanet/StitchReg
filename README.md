# Stitch Regulator for Juki TL Machines

This is a stitch regulator that can be used with Juki TL machines. Commercial versions of these are very expensive ($700 - $1,000 USD), but the way they work is fairly simple. You can build a DIY version using an Arduino microcontroller, an old PS/2 optical mouse, and a handful of inexpensive electronic components.

## Questions

### Q: What is a stitch regulator?
A: A stitch regulator senses movement of the machine or fabric and adjusts the speed of the needle to maintain a consistent stitch length.

### Q: What machines does this work on?
A: Any Juki TL machine should work. Specifically, any machine that uses the Juki JC-001 pedal.

### Q: Can it work with other machines?
A: Maybe? If the foot pedal acts as a variable resistor this could potentially be adapted. You might have to change the R2 and R3 resistors and possibly use a different version of the 4131 chip.

### Q: Does this work with or without a quilting frame?
A: It can work with both, depending how you mount the sensor!

### Q: Do I need to know electronics or coding to use this?
A: It would help, but no. At least I hope not, if I write the instructions well enough!

### Q: Will you build and sell me one of these?
A: Not at this point, it's still under development and subject to change a lot.


## How It Works

StitchReg works by using the optical sensor from a mouse to calculate the speed of the machine/fabric and control the speed of the machine by acting as the foot pedal with a digital potentiometer.

The Juki foot pedal is basically a resistor and there are 3 states we need to care about:

- **idle** - when the foot pedal is not depressed forward or backward it sits around 140k ohms
- **thread cutter** - to trigger the thread cutter the pedal drops to around 44k ohms
- **sewing** - sewing happens, slow to fast, from around 20k ohms to around 500 ohms

## Building One For Yourself

### What You Need

Currently this is just breadboarded with an Arduino Uno Rev 3. Once the design is final I'll create a PCB and enclosure.

- [PS/2 Optical Mouse](https://www.amazon.com/dp/B0DH58555P)
- [Arduino kit](https://www.amazon.com/dp/B01D8KOZF4)
- [MCP4131-104E/P](https://www.mouser.com/ProductDetail/Microchip-Technology/MCP4131-103E-P)
- [Momentary switch](https://www.amazon.com/dp/B0BR41KCDP)

The parts below are in the linked arduino kit, but I'll find better options.

- Relay (in the above arduino kit)
- 10k ohm resistor (2) (in the above arduino kit)
- 100k ohm resistor (in the above arduino kit)
- Buzzer (in the above arduino kit)
- Barrel Jack (cut the one off the battery connector in the arduino kit)

### Schematic

![Schematic](https://github.com/user-attachments/assets/296f13f5-c883-4133-b986-caafcdf1954d)
