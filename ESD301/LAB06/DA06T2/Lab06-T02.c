#define F_CPU 16000000UL

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define BIT_RATE_VAL 3
#define SLAVE_ADDRESS 0xD0
#define DIVIDER_ADDRESS 0x19
#define POWER_ADDRESS 0x6B
#define CONFIG_ADDRESS 0x1A
#define GYRO_CONFIG_ADDRESS 0x1B
#define INTERRUPT_ADDRESS 0x38
#define ACC_START_ADDRESS 0x3B
#define GYRO_START_ADDRESS 0x43
#define INTERRUPT_ADDRESS 0x38
#define ACC_SCALE 16384.0
#define GYRO_SCALE 16.4
#define UBBR_VALUE 103
#define TIMER_10MILLI 624
#define dt 0.01

volatile char myIntString[70];
volatile float accData[3], gyroData[3];
volatile float pitch = 0;
volatile float roll = 0;

// I2C functions.
void i2c_init() {
	// Initializes settings.
	TWBR0 = BIT_RATE_VAL;						// Sets bit rate.
	TWSR0 |= (0<<TWPS1)|(0<<TWPS0);				// Sets pre-scaler value to 1.
}

void i2c_start() {
	// Generates a start condition on SDA line.
	TWCR0 |= (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);	// Generates start condition.
	while ( !(TWCR0&(1<<TWINT)) );				// Waits for TWINT to go high.
}

void i2c_start_wait(uint8_t slaveAddress)
{
	uint8_t status = 0;
	// Repeatedly attempts to send start and address until an ACK is received.
	while (1) {
		TWCR0 = (1<<TWSTA)|(1<<TWEN)|(1<<TWINT);	// Generates start condition.
		while (!(TWCR0 & (1<<TWINT)));
		status = TWSR0 & 0xF8;						// Checks status for ACK.
		if (status != 0x08)
		continue;								// Sends slave address.
		TWDR0 = slaveAddress;
		TWCR0 = (1<<TWEN)|(1<<TWINT);
		while (!(TWCR0 & (1<<TWINT)));
		status = TWSR0 & 0xF8;						// Checks status for ACK.
		if (status != 0x18 )
		{
			i2c_stop();								// Stop if conditions aren't met.
			continue;
		}
		break;
	}
}

void i2c_stop() {
	// Generates a stop condition on the SDA line.
	TWCR0 |= (1<<TWSTO)|(1<<TWEN)|(1<<TWINT);	// Generates stop condition.
	while ( (TWCR0&(1<<TWSTO)) );				// Waits for TWINT to go high.
}

void i2c_send(uint8_t data) {
	// Sends data through the SDA line.
	TWDR0 = data;
	TWCR0 |= (1<<TWEN)|(1<<TWINT);
	while ( !(TWCR0&(1<<TWINT)) );				// Waits for TWINT to go high.
}

char i2c_read(uint8_t ack) {
	TWCR0 = (1<<TWEN)|(1<<TWINT)|(ack ? (1<<TWEA) : 0x00);
	while ( !(TWCR0&(1<<TWINT)) );				// Waits for TWINT to go high.
	return TWDR0;
}

// MP6050 functions
void initializeMP6050() {
	// Configures divider pre-scaler.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(DIVIDER_ADDRESS);		// Sends first register address.
	i2c_send(0x07);					// Sample rate register value as 7. (Sample at 1kHz)
	i2c_stop();						// Generates stop condition.
	
	// Configures power.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(POWER_ADDRESS);		// Sends register address.
	i2c_send(0x01);					// PLL with x-axis reference.
	i2c_stop();						// Generates stop condition.
	
	// Configures configuration register.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(CONFIG_ADDRESS);		// Sends register address.
	i2c_send(0x00);					// Sets Fs as 8kHz.
	i2c_stop();						// Generates stop condition.
	
	// Configures power.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(GYRO_CONFIG_ADDRESS);	// Sends register address.
	i2c_send(0x18);					// Sets full-scale range as +/- 2000 degrees.
	i2c_stop();						// Generates stop condition.
	
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(INTERRUPT_ADDRESS);	// Sends register address.
	i2c_send(0x01);					// Enables DATA_RDY_EN.
	i2c_stop();						// Generates stop condition.
}

void readRawData(float accData[], float gyroData[]) {
	// Sets pointer.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(ACC_START_ADDRESS);	// Sends XH address.
	
	// Starts reading in accelerometer data.
	i2c_start();					// Sends another start condition.
	i2c_send(SLAVE_ADDRESS|0x01);	// Sends slave address in read mode.
	accData[0] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(1) );
	accData[1] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(1) );
	accData[2] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(0) );
	i2c_stop();						// Generates stop condition.
	
	// Sets pointer.
	i2c_start_wait(SLAVE_ADDRESS);	// Generates start condition.
	i2c_send(GYRO_START_ADDRESS);	// Sends gyro XH address.
	
	// Starts reading in gyro data.
	i2c_start();					// Sends another start condition.
	i2c_send(SLAVE_ADDRESS|0x01);	// Sends slave address in read mode.
	gyroData[0] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(1) );
	gyroData[1] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(1) );
	gyroData[2] = ( ((int)i2c_read(1) << 8) + (int)i2c_read(0) );
	i2c_stop();						// Generates stop condition.
}

void scaleData(float accData[], float gyroData[]) {
	for (uint8_t i = 0; i < 3; i++) {
		accData[i] /= ACC_SCALE;
		gyroData[i] /= GYRO_SCALE;
	}
}

// USART functions.
// Enables the UART and it's transmitter.
void enableTransmit() {
	// Sets port.
	DDRD |= 0x02;
	
	// Sets baud rate.
	UBRR0 = UBBR_VALUE;
	
	// Sets UART settings.
	UCSR0C = 0b00000110;			// to async mode with no parity
	// 2 stop bits, and a frame of 8 bits.
	UCSR0B = 0b00001000;			// Enables transmit line.
}

// Disables the transmitter on UART.
void disableTransmit() {
	UCSR0B &= ~(1<<TXEN0);
}

// Sends one byte of data out the TX line.
void sendByte(uint8_t b) {
	while (!(UCSR0A & (1<<UDRE0)));		// Waits until the UDR0 register is ready.
	UDR0 = b;						// Sends the byte.
}

void sendString(char string[]) {
	int i = 0;
	while (string[i] != '\0')
	sendByte(string[i++]);
}

// Timer functions.
void initializeTimer() {
	// Sets up TOP.
	OCR1A = TIMER_10MILLI;
	
	// Sets timer.
	TCCR1A = 0x00;						// Sets timer to CTC mode with OCR1A TOP.
	TCCR1B = (1<<WGM12)|(1<<CS12);		// Sets pre-scaler to 256.
	
	// Sets up interrupt.
	TIMSK1 = (1<<OCIE1A);				// Enables OCR1A compare match interrupt.
	TIFR1 |= (1<<OCF1A);				// Clears interrupt flag.
	
	TCNT1 = 0;
}

int main() {
	// Initialize modules.
	i2c_init();
	enableTransmit();
	
	_delay_ms(150);				// Wait some time.
	
	initializeMP6050();
	initializeTimer();
	sei();
	
	while (1) {
		readRawData(accData, gyroData);						// Read data from MP6050.
		//scaleData(accData, gyroData);						// Applies scale value.
		snprintf(myIntString, 70, "Pitch: %f, Roll: %f\n", pitch, roll);
		sendString(myIntString);
		//snprintf(myIntString, 70, "%f, %f, %f, %f, %f, %f\n",
		//accData[0], accData[1], accData[2],
		//gyroData[0], gyroData[1], gyroData[2]);				// Converts to a string float.
		//sendString(myIntString);							// Sends data through USART.
	}
}

ISR(TIMER1_COMPA_vect) {
	float pitchAcc, rollAcc;
	// Integrate the gyroscope data -> int(angularSpeed) = angle
	pitch += ((float)gyroData[0] / GYRO_SCALE) * dt;
	// Angle around the X-axis
	roll -= ((float)gyroData[1] / GYRO_SCALE) * dt;
	// Angle around the Y-axis
	// Compensate for drift with accelerometer data if !bullshit
	// Sensitivity = -2 to 2 G at 16Bit -> 2G = 32768 && 0.5G = 8192
	int forceMagnitudeApprox = abs(accData[0]) + abs(accData[1]) + abs(accData[2]);
	if (forceMagnitudeApprox > 8192 && forceMagnitudeApprox < 32768)
	{
		// Turning around the X axis results in a vector on the Y-axis
		pitchAcc = atan2f((float)accData[1], (float)accData[2]) * 180 / M_PI;
		pitch = pitch * 0.98 + pitchAcc * 0.02;
		// Turning around the Y axis results in a vector on the X-axis
		rollAcc = atan2f((float)accData[0], (float)accData[2]) * 180 / M_PI;
		roll = roll * 0.98 + rollAcc * 0.02;
	}
	
	TIFR1 |= (1<<OCF1A);				// Clears interrupt flag.
}