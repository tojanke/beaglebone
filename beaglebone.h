#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <time.h>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
using namespace std;

#define BONE_CAPEMGR "/sys/devices/bone_capemgr.8/slots"
#define MAX_BUF 64
#define I2C_BUF 0x80
#define LOG_OUTPUT 1
#define SYSFS_PWM_DIR "/sys/class/pwm"
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define LOG_FILE "log.txt"

#if LOG_OUTPUT
namespace LOG {
	int Write(char* Type, char* Parent, char* Event) {
		time_t tTime = time(0);
		struct tm* tmTime = localtime(&tTime);
		char D_D[MAX_BUF], D_M[MAX_BUF], D_Y[MAX_BUF], T_H[MAX_BUF], T_M[MAX_BUF], T_S[MAX_BUF];
		snprintf(D_D, sizeof(D_D), "%02d", tmTime->tm_mday);
		snprintf(D_M, sizeof(D_M), "%02d", tmTime->tm_mon);
		snprintf(D_Y, sizeof(D_Y), "%04d", tmTime->tm_year + 1900);
		snprintf(T_H, sizeof(T_H), "%02d", tmTime->tm_hour);
		snprintf(T_M, sizeof(T_M), "%02d", tmTime->tm_min);
		snprintf(T_S, sizeof(T_S), "%02d", tmTime->tm_sec);
		ofstream logfile;
		logfile.open (LOG_FILE, ios::out | ios::app);
		logfile << D_D
			<< "/"
			<< D_M
			<< "/"
			<< D_Y
			<< " "
			<< T_H
			<< ":"
			<< T_M
			<< "."
			<< T_S
			<< " ["
			<< Type
			<< "][In "
			<< Parent
			<< "] "
			<< Event
			<< endl;
  		logfile.close();
		return 0;
	}
}
#endif
namespace PWM {
	int SlotExists(const string & moduleName) {
		std::ifstream in(BONE_CAPEMGR);
		in.exceptions(ios::badbit);
		int slot = -1;
		while (in >> slot)
		{
			std::string restOfLine;
			std::getline(in, restOfLine);
			if (restOfLine.find(moduleName) != std::string::npos)
			{
				return 1;
			}
		}
		return 0;
	}

	int InstOverlay() {
		if (SlotExists("FJ-GPIO") == 0)  {
			int DTO;
			DTO = open(BONE_CAPEMGR, O_WRONLY);
			if (DTO < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::InstOverlay()", (char *)"Unable to install FJ-GPIO!");
				#endif
				return 1;
			}
			write(DTO, "FJ-GPIO", sizeof("FJ-GPIO"));
			close(DTO);
		}

		if (SlotExists("am33xx_pwm") == 0) {
			int DTP;
			DTP = open(BONE_CAPEMGR, O_WRONLY);
			if (DTP < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::InstOverlay()", (char *)"Unable to install am33xx_pwm!");
				#endif
				return 1;
			}
			write(DTP, "am33xx_pwm", sizeof("am33xx_pwm"));
			close(DTP);
		}

		return 0;
	}

	class PIN {
	public:
		PIN( unsigned int pwm, unsigned int period_hz ) {
			InstOverlay();
			ID = pwm;
			int fd, len;
			char buf[MAX_BUF];
			fd = open(SYSFS_PWM_DIR "/export", O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN()", (char *)"Unable to export Pin.");
				#endif
			}
			len = snprintf(buf, sizeof(buf), "%d", ID);
			write(fd, buf, len);
			close(fd);
			SetPeriod(period_hz);
			SetDuty(0);
		}
		~PIN() {
			int fd, len;
			char buf[MAX_BUF];
			fd = open(SYSFS_PWM_DIR "/unexport", O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::~PIN()", (char *)"Unable to unexport Pin.");
				#endif
			}
			len = snprintf(buf, sizeof(buf), "%d", ID);
			write(fd, buf, len);
			close(fd);
		}
		double GetPeriod() {
			int fd;
			char buf[MAX_BUF];
			char ch;
			snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/period_ns", ID);
			fd = open(buf, O_RDONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::GetPeriod()", (char *)"Unable to get PIN Period.");
				#endif
				return 1;
			}
			read(fd, &ch, 1);
			return ch;
			close(fd);
			return 0;
		}
		int SetPeriod( int period_hz ) {
			int period_ns = 1000000000 / period_hz;
			int fd, len;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/period_ns", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::SetPeriod()", (char *)"Unable to set PIN Period.");
				#endif
				return fd;
			}
			len = snprintf(buf, sizeof(buf), "%d", period_ns);
			write(fd, buf, len);
			close(fd);
			period_ref = period_ns;
			return 0;
		}
		double GetDuty() {
			int fd;
			char buf[MAX_BUF];
			char ch;
			snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/duty_ns", ID);
			fd = open(buf, O_RDONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::GetDuty()", (char *)"Unable to get PIN Duty Cycle.");
				#endif
				return 1;
			}
			read(fd, &ch, 1);
			return ch;
			close(fd);
			return 0;
		}
		int SetDuty( int duty_pp ) {
			int duty_ns = period_ref / 1000 * duty_pp;
			int fd;
			char dbuf[MAX_BUF], vbuf[MAX_BUF];
			snprintf(dbuf, sizeof(dbuf), SYSFS_PWM_DIR "/pwm%d/duty_ns", ID);
			fd = open(dbuf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::SetDuty()", (char *)"Unable to set PIN Duty Cycle.");
				#endif
				return fd;
			}
			snprintf(vbuf, sizeof(vbuf), "%d", duty_ns);
			write(fd, vbuf, sizeof(vbuf));
			close(fd);
			duty_ref = duty_ns;
			return 0;
		}
		int Enable() {
			int fd;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/run", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::Enable()", (char *)"Unable to enable PIN.");
				#endif
				return fd;
			}
			write(fd, "1", sizeof("1"));
			close(fd);
			return 0;
		}
		int Disable() {
			int fd;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_PWM_DIR "/pwm%d/run", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"PWM::PIN::Disable()", (char *)"Unable to disable PIN.");
				#endif
				return fd;
			}
			write(fd, "0", sizeof("0"));
			close(fd);
			return 0;
		}
	private:
		unsigned int ID;
		unsigned int period_ref;
		unsigned int duty_ref;
	};
}

namespace GPIO {
	enum PIN_DIRECTION{ INPUT=0, OUTPUT=1 };
	enum PIN_VALUE{	LOW=0, HIGH=1, FAIL=13 };
	class PIN {
	public:
		PIN( unsigned int gpio, PIN_DIRECTION flag ) {
			ID = gpio;
			int fd, len;
			char buf[MAX_BUF];
			fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"GPIO::PIN()", (char *)"Unable to export PIN.");
				#endif
			}
			len = snprintf(buf, sizeof(buf), "%d", ID);
			write(fd, buf, len);
			close(fd);
			SetDir(flag);
		}
		~PIN() {
			int fd, len;
			char buf[MAX_BUF];
			fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"GPIO::~PIN()", (char *)"Unable to unexport PIN.");
#				endif
			}
			len = snprintf(buf, sizeof(buf), "%d", ID);
			write(fd, buf, len);
			close(fd);
		}
		PIN_VALUE GetValue() {
			int fd;
			char buf[MAX_BUF];
			char ch;
			snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", ID);
			fd = open(buf, O_RDONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"GPIO::PIN::GetValue()", (char *)"Unable to get PIN value.");
				#endif
				return FAIL;
			}
			read(fd, &ch, 1);
			close(fd);
			if (ch != '0') { return HIGH;}
			return LOW;

		}
		int SetValue( PIN_VALUE value ) {
			int fd;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"GPIO::PIN::SetValue()", (char *)"Unable to set PIN value.");
				#endif
			return fd; }
			if (value==LOW)	{ write(fd, "0", sizeof("0")); }
			else { write(fd, "1", sizeof("1")); }
			close(fd);
			return 0;
		}
		int SetDir( PIN_DIRECTION flag ) {
			int fd;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR  "/gpio%d/direction", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"GPIO::PIN::SetDalue()", (char *)"Unable to set PIN direction.");
				#endif
				return fd;
			}
			if (flag == OUTPUT) { write(fd, "out", sizeof("out")); }
			else { write(fd, "in", sizeof("in")); }
			close(fd);
			return 0;
		}
	private:
		unsigned int ID;
	};
};

namespace USERLED {
	char* LED_ADDR[] = {	(char *)"/sys/class/leds/beaglebone:green:usr0/brightness",
			(char *)"/sys/class/leds/beaglebone:green:usr1/brightness",
			(char *)"/sys/class/leds/beaglebone:green:usr2/brightness",
			(char *)"/sys/class/leds/beaglebone:green:usr3/brightness"};


	class LED {
	public:
		LED() {
			index = 0;
			Brightness = LED_ADDR[index];
		}
		LED( int custom ) {
			index = custom;
			Brightness = LED_ADDR[index];
		}
		~LED() { }
		int on( ) {
			int fd = open(Brightness, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"USERLED::LED::on()", (char *)"Unable turn on LED.");
				#endif
				return fd;
			}
			write(fd, "1", sizeof("1"));
			close(fd);
			return 0;
		}
		int on( int duration ) {
			int fd = open(Brightness, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"USERLED::LED::on()", (char *)"Unable turn on LED.");
				#endif
				return fd;
			}
			write(fd, "1", 2);
			usleep(duration);
			write(fd, "0", sizeof("0"));
			close(fd);
			return 0;
		}
		int off() {
			int fd = open(Brightness, O_WRONLY);
			if (fd < 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"USERLED::LED::off()", (char *)"Unable turn off LED.");
				#endif
				return fd;
			}
			write(fd, "0", sizeof("0"));
			close(fd);
			return 0;
		}
	private:
		signed short int index;
		char* Brightness;
	};

	int ULedInit() {
		int fd;

		fd = open(LED_ADDR[0], O_WRONLY);
		if (fd < 0) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"USERLED::ULedInit()", (char *)"Unable to initialize LED.");
			#endif
			return fd;
		}
		write(fd, "0", sizeof("0"));
		close(fd);

		fd = open(LED_ADDR[1], O_WRONLY);
		if (fd < 0) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"USERLED::ULedInit()", (char *)"Unable to initialize LED.");
			#endif
			return fd;
		}
		write(fd, "0", sizeof("0"));
		close(fd);

		fd = open(LED_ADDR[2], O_WRONLY);
		if (fd < 0) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"USERLED::ULedInit()", (char *)"Unable to initialize LED.");
			#endif
			return fd;
		}
		write(fd, "0", sizeof("0"));
		close(fd);

		fd = open(LED_ADDR[3], O_WRONLY);
		if (fd < 0) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"USERLED::ULedInit()", (char *)"Unable to initialize LED.");
			#endif
			return fd;
		}
		write(fd, "0", sizeof("0"));
		close(fd);

		return 0;
	}

	void blink( int bps, int time ) {
		LED ULED[] = { 0, 1, 2, 3 };
		int intervall = 1000000 / bps;
		int times = (time * bps)/4;
		for (int t = 0; t < times; t++) {
			for (int L = 0; L < 4; L++) {
				ULED[L].on(intervall);
			}
		}
	}
}

namespace I2C {
#define I2C_BUS_1 "/dev/i2c-1"
#define I2C_BUS_2 "/dev/i2c-2"
#define I2C_BUS_3 "/dev/i2c-3"
	int WriteByte(char * i2cbus, char chip_address, char data_address, char value){
		int bus = open(i2cbus, O_RDWR);
		if ( bus == -1 ) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::WriteByte()", (char *)"Unable to open I2C Bus");
			#endif
			return(1);
		}
		int device = ioctl(bus, I2C_SLAVE,chip_address ) ;
		if (device==-1){
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::WriteByte()", (char *)"Unable to Register I2C Slave Address");
			#endif
			return(2);
		}
		char buffer[2];
			buffer[0] = data_address;
			buffer[1] = value;
		if ( write(bus, buffer, 2) != 2) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::WriteByte()", (char *)"Unable to Write I2C Value");
			#endif
			return(3);
		}
		close(bus);
		return 0;
	}
	char ReadByte(char * i2cbus, char chip_address, char data_address){
		int bus = open(i2cbus, O_RDWR);
		if ( bus == -1 ){
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::ReadByte()", (char *)"Unable to open I2C Bus");
			#endif
			return(1);
		}
		int device = ioctl(bus, I2C_SLAVE, chip_address) ;
		if (device==-1){
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::ReadByte()", (char *)"Unable to Register I2C Slave Address");
			#endif
			return(2);
		}
		char buffer[1];
		buffer[0] = data_address;
		if (write(bus, buffer, 1) != 1) {
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::ReadByte()", (char *)"Unable to prepare I2C Read");
			#endif
			return(3);}
		char value[1];
		if (read(bus, value, 1) != 1){
			#if LOG_OUTPUT
				LOG::Write((char *)"FAIL", (char *)"I2C::ReadByte()", (char *)"Unable to read I2C Address Value");
			#endif
			return(4);
		}
		close(bus);
		return value[0];
	}
}

namespace SENSORS {
#define MPU6050_ADDRESS_L 0x68
#define MPU6050_ADDRESS_H 0x69
#define MPU6050_PWR_MGMT_1 0x6B
#define MPU6050_RESET 0x80
#define MPU6050_GYROSCOPE_RANGE_SEL 0x1B
#define MPU6050_ACCELEROMETER_RANGE_SEL 0x1C
#define MPU6050_WHO_AM_I 0x75

	enum MPU6050_CLOCK_SOURCE {
		INTERNAL = 0,
		PLL_X_AXIS = 1,
		PLL_Y_AXIS = 2,
		PLL_Z_AXIS = 3,
		PLL_EXTERNAL_32_KHZ = 4,
		PLL_EXTERNAL_19_MHZ = 5
	};
	enum MPU6050_GYROSCOPE_RANGE {
		PLUSMINUS_250 = 0,
		PLUSMINUS_500 = 1,
		PLUSMINUS_1000 = 2,
		PLUSMINUS_2000 = 3
	};
	enum MPU6050_ACCELEROMETER_RANGE {
		PLUSMINUS_2_G = 0,
		PLUSMINUS_4_G = 1,
		PLUSMINUS_8_G = 2,
		PLUSMINUS_16_G = 3
	};

	class MPU6050 {
	public:
		MPU6050() {
			Address = MPU6050_ADDRESS_L;
			Bus = (char*)I2C_BUS_1 ;
		}
		MPU6050( char * i2cbus, char device_address) {
			Address = device_address;
			Bus = i2cbus;
		}
	private:
		char * Bus;
		char Address;
	public:
		int Reset () {
			int werr = I2C::WriteByte(Bus, Address, MPU6050_PWR_MGMT_1, MPU6050_RESET);
			if (werr != 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"SENSORS::MPU6050::SetClockSource()", (char *)"I2C Read Error");
				#endif
			}
			return 0;
		}
		int SetClockSource( MPU6050_CLOCK_SOURCE clksrc) {
			char g_reg = I2C::ReadByte(Bus, Address, MPU6050_PWR_MGMT_1);
			char value = clksrc;
			g_reg = g_reg & 0b11111000;
			value = g_reg | value;
			int werr = I2C::WriteByte(Bus, Address, MPU6050_PWR_MGMT_1, value);
			if (werr != 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"SENSORS::MPU6050::SetClockSource()", (char *)"I2C Read Error");
				#endif
			}
			return 0;
		}
		int SetAccelerometerRange( MPU6050_ACCELEROMETER_RANGE range ) {
			char a_reg = I2C::ReadByte(Bus, Address, MPU6050_ACCELEROMETER_RANGE_SEL);
			char value = range << 3;
			a_reg = a_reg & 0b11100111;
			value = a_reg | value;
			int werr = I2C::WriteByte(Bus, Address, MPU6050_ACCELEROMETER_RANGE_SEL, value);
			if (werr != 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"SENSORS::MPU6050::SetClockSource()", (char *)"I2C Read Error");
				#endif
			}
			return 0;
		}
		int SetGyroscopeRange( MPU6050_GYROSCOPE_RANGE range ) {
			char g_reg = I2C::ReadByte(Bus, Address, MPU6050_GYROSCOPE_RANGE_SEL);
			char value = range << 3;
			g_reg = g_reg & 0b11100111;
			value = g_reg | value;
			int werr = I2C::WriteByte(Bus, Address, MPU6050_GYROSCOPE_RANGE_SEL, value);
			if (werr != 0) {
				#if LOG_OUTPUT
					LOG::Write((char *)"FAIL", (char *)"SENSORS::MPU6050::SetClockSource()", (char *)"I2C Read Error");
				#endif
			}
			return 0;
		}
		int Calibrate( MPU6050_ACCELEROMETER_RANGE a_range, MPU6050_GYROSCOPE_RANGE g_range, MPU6050_CLOCK_SOURCE clksrc) {
			Reset();
			SetClockSource(clksrc);
			SetAccelerometerRange(a_range);
			SetGyroscopeRange(g_range);
			return 0;
		}
		char WhoAmI() {
			return I2C::ReadByte(Bus, Address, MPU6050_WHO_AM_I);
		}
	};
}
