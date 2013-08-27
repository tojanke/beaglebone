#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <time.h>
#include <iostream>
using namespace std;

#define BONE_CAPEMGR "/sys/devices/bone_capemgr.9/slots"
#define MAX_BUF 64
#define LOG_OUTPUT 1

enum PIN_DIRECTION{ INPUT=0, OUTPUT=1 };
enum PIN_VALUE{	LOW=0, HIGH=1, FAIL=13 };

#if LOG_OUTPUT
#define LOG_FILE "log.txt"
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
	#define SYSFS_PWM_DIR "/sys/class/pwm"
	
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
LOG::Write("FAIL", "PWM::InstOverlay()", "Unable to install FJ-GPIO!");
#endif
			return 1; }
			write(DTO, "FJ-GPIO", sizeof("FJ-GPIO"));
			close(DTO);
		}
		
		if (SlotExists("am33xx_pwm") == 0) {
			int DTP;
			DTP = open(BONE_CAPEMGR, O_WRONLY);
			if (DTP < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "PWM::InstOverlay()", "Unable to install am33xx_pwm!");
#endif
			return 1; }
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
LOG::Write("FAIL", "PWM::PIN()", "Unable to export Pin.");
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
LOG::Write("FAIL", "PWM::~PIN()", "Unable to unexport Pin.");
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
LOG::Write("FAIL", "PWM::PIN::GetPeriod()", "Unable to get PIN Period.");
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
LOG::Write("FAIL", "PWM::PIN::SetPeriod()", "Unable to set PIN Period.");
#endif	
			return fd; }
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
LOG::Write("FAIL", "PWM::PIN::GetDuty()", "Unable to get PIN Duty Cycle.");
#endif	
			return 1; }
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
LOG::Write("FAIL", "PWM::PIN::SetDuty()", "Unable to set PIN Duty Cycle.");
#endif	
			return fd; }
			snprintf(vbuf, sizeof(vbuf), "%d", duty_ns);
			write(fd, vbuf, sizeof(vbuf));
			cout << vbuf << endl;
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
LOG::Write("FAIL", "PWM::PIN::Enable()", "Unable to enable PIN.");
#endif	
			return fd; }
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
LOG::Write("FAIL", "PWM::PIN::Disable()", "Unable to disable PIN.");
#endif				
			return fd; }
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
	#define SYSFS_GPIO_DIR "/sys/class/gpio"

	class PIN {
	public:
		PIN( unsigned int gpio, PIN_DIRECTION flag ) {
			ID = gpio;
			int fd, len;
			char buf[MAX_BUF];
			fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
			if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "GPIO::PIN()", "Unable to export PIN.");
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
LOG::Write("FAIL", "GPIO::~PIN()", "Unable to unexport PIN.");
#endif
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
LOG::Write("FAIL", "GPIO::PIN::GetValue()", "Unable to get PIN value.");
#endif
				return FAIL;
			}
			read(fd, &ch, 1);
			if (ch != '0') { return HIGH;}
			else { return LOW; }
			close(fd);
		}
		int SetValue( PIN_VALUE value ) {
			int fd;
			char buf[MAX_BUF];
			snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", ID);
			fd = open(buf, O_WRONLY);
			if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "GPIO::PIN::SetValue()", "Unable to set PIN value.");
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
LOG::Write("FAIL", "GPIO::PIN::SetDalue()", "Unable to set PIN direction.");
#endif
			return fd; }
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
	char* LED_ADDR[] = {	"/sys/class/leds/beaglebone:green:usr0/brightness", 
				"/sys/class/leds/beaglebone:green:usr1/brightness", 
				"/sys/class/leds/beaglebone:green:usr2/brightness", 
				"/sys/class/leds/beaglebone:green:usr3/brightness"};
	

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
LOG::Write("FAIL", "USERLED::LED::on()", "Unable turn on LED.");
#endif			
			return fd; }
			write(fd, "1", sizeof("1"));			
			close(fd);
			return 0;
		}
		int on( int duration ) {
			int fd = open(Brightness, O_WRONLY);
			if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "USERLED::LED::on()", "Unable turn on LED.");
#endif
			return fd; }
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
LOG::Write("FAIL", "USERLED::LED::off()", "Unable turn off LED.");
#endif			
			return fd; }
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
LOG::Write("FAIL", "USERLED::ULedInit()", "Unable to initialize LED.");
#endif		
		return fd; }
		write(fd, "0", sizeof("0"));			
		close(fd);

		fd = open(LED_ADDR[1], O_WRONLY);
		if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "USERLED::ULedInit()", "Unable to initialize LED.");
#endif		
		return fd; }
		write(fd, "0", sizeof("0"));			
		close(fd);

		fd = open(LED_ADDR[2], O_WRONLY);
		if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "USERLED::ULedInit()", "Unable to initialize LED.");
#endif		
		return fd; }
		write(fd, "0", sizeof("0"));			
		close(fd);

		fd = open(LED_ADDR[3], O_WRONLY);
		if (fd < 0) { 
#if LOG_OUTPUT
LOG::Write("FAIL", "USERLED::ULedInit()", "Unable to initialize LED.");
#endif		
		return fd; }
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
