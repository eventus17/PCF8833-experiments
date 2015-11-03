import serial

# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
	port='/dev/ttyUSB0',
	baudrate=9600,
)

ser.open()
ser.isOpen()

ser.write(chr(0x01)); #command
ser.write(chr(0x2B)); #PASET

ser.write(chr(0x02)); #data
ser.write(chr(0x20));

ser.write(chr(0x02));
ser.write(chr(0x20));

ser.write(chr(0x01)); #command
ser.write(chr(0x2A)); #CASET

ser.write(chr(0x02)); #data
ser.write(chr(0x3F));

ser.write(chr(0x02));
ser.write(chr(0x3F));

ser.write(chr(0x01)); #command
ser.write(chr(0x2A)); #CASET

ser.write(chr(0x02)); #data
ser.write(chr(0x3F));

ser.write(chr(0x02));
ser.write(chr(0x3F));

ser.write(chr(0x01)); #command
ser.write(chr(0x2C)); #RAMWR

ser.write(chr(0x02)); #data
ser.write(chr(0xE0)); #RED

ser.write(chr(0x00)); #DeselectLCD

ser.close()





#	GlcdWriteCmd(PASET);
#	GlcdWriteData(x);
#	GlcdWriteData(x);
#	
#	GlcdWriteCmd(CASET);
#	GlcdWriteData(y);
#	GlcdWriteData(y);
	
#	// set the color of the pixel
#	GlcdWriteCmd(RAMWR);
#	GlcdWriteData(ColorRGB8);
#	
#	DeselectLCD();