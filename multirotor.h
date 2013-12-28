#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <float.h>

namespace SENSORS {
	class Accelerometer {
	public:
		Accelerometer(int PIN) {
			rPIN = PIN;
		}
		int GetAcceleration(float &AccelerationX, float &AccelerationY, float &AccelerationZ){
			AccelerationX = 0;
			AccelerationY = 0;
			AccelerationZ = 0;
			return 0;
		}
	private:
		int rPIN;
	};

	class Gyroscope{
	public:
		Gyroscope(int PIN) {
			rPIN = PIN;
		}
		int GetRotation(float &RotationX, float &RotationY, float &RotationZ){
			RotationX = 0;
			RotationY = 0;
			RotationZ = 0;
			return 0;
		}
	private:
		int rPIN;
	};

	class Barometer{
	public:
		Barometer(int PIN) {
			rPIN = PIN;
		}
		float GetAltitude(){
			return 1 * (0);
		}
	private:
		int rPIN;
	};

	class Magnetometer{
	public:
		Magnetometer(int PIN) {
			rPIN = PIN;
		}
		float GetOrientation(){
			return 0;
		}
	private:
		int rPIN;
	};

	Accelerometer MP80(13);
	Gyroscope CR13(115);
	Barometer ALT14(27);
	Magnetometer NOR546(65);
}

namespace POSITION {
	class PosData {
	public:
		PosData(){
			RotationX = 0;
			RotationY = 0;
			RotationZ = 0;
			kRotation = 0;
			PositionX = 0;
			PositionY = 0;
			AltitudeZ = 0;
			kAltitude = 0;
		}

		float GetRotX() { return RotationX; }
		float GetRotY() { return RotationY; }
		float GetRotZ()	{ return RotationZ; }
		int GetkRot() { return kRotation; }
		float GetPosX() { return PositionX; }
		float GetPosY() { return PositionY; }
		float GetAltZ() { return AltitudeZ; }
		int GetkAlt() { return kAltitude; }
		void SetRotX(float RotX)	{ RotationX = RotX; }
		void SetRotY(float RotY) 	{ RotationY = RotY; }
		void SetRotZ(float RotZ) 	{ RotationZ = RotZ; }
		void SetkRot(int kRot) 	{ kRotation = kRot; }
		void SetPosX(float PosX) 	{ PositionX = PosX; }
		void SetPosY(float PosY) 	{ PositionY = PosY; }
		void SetAltZ(float AltZ) 	{ AltitudeZ = AltZ; }
		void SetkAlt(int kAlt) 	{ kAltitude = kAlt; }

	private:
		float RotationX;
		float RotationY;
		float RotationZ;
		int kRotation;
		float PositionX;
		float PositionY;
		float AltitudeZ;
		int kAltitude;
	};//PosData

	class Waypoint {
	public:
		Waypoint(float PosX, float PosY, float AltZ) {
			PositionX = PosX;
			PositionY = PosY;
			AltitudeZ = AltZ;
			Orientation = 0;
		}
		Waypoint(float PosX, float PosY, float AltZ, float OrtZ) {
			PositionX = PosX;
			PositionY = PosY;
			AltitudeZ = AltZ;
			Orientation = OrtZ;
		}
		float GetPosX() 			{ return PositionX; }
		float GetPosY() 			{ return PositionY; }
		float GetAltZ() 			{ return AltitudeZ; }
		float GetOrtZ() 			{ return Orientation; }
		void SetPosX(float PosX) 	{ PositionX = PosX; }
		void SetPosY(float PosY) 	{ PositionY = PosY; }
		void SetAltZ(float AltZ) 	{ AltitudeZ = AltZ; }
		void SetOrtZ(float OrtZ) 	{ Orientation = OrtZ; }

	private:
		float PositionX;
		float PositionY;
		float AltitudeZ;
		float Orientation;
	};//WaypointData

	PosData Current;
}//POSITION

namespace ORIENTATION {
	double GetRotX(int kFor) {
		double Fk = 1 / (1 + abs(kFor) * 0.001);
		if (kFor < 0) 	{ return 360 - (acos(Fk)* (180 / M_PI)); }
		else { return 	(acos(Fk)* (180 / M_PI)); }
	}
	double GetRotY(int kSid) {
		double Fk = 1 / (1 + (abs(kSid) * 0.001));
		if (kSid < 0) 	{ return 360 - (acos(Fk)* (180 / M_PI)); }
		else 			{ return (acos(Fk)* (180 / M_PI)); }
	}
	double GetRotZ(float CurrentX, float CurrentY, float DestX, float DestY) {
		if (CurrentY < DestY){
			if (CurrentX < DestX) {
				float IntvX = DestX - CurrentX;
				float IntvY = DestY - CurrentY;
				return (atan(IntvX / IntvY)*(180 / M_PI));
			}
			else if (CurrentX > DestX) {
				float IntvX = CurrentX - DestX;
				float IntvY = DestY - CurrentY;
				return 360 - (atan(IntvX / IntvY)*(180 / M_PI));
			}
		}
		else if (CurrentY > DestY) {
			if (CurrentX < DestX) {
				float IntvX = DestX - CurrentX;
				float IntvY = CurrentY - DestY;
				return 180 - (atan(IntvX / IntvY)*(180 / M_PI));
			}
			else if (CurrentX > DestX) {
				float IntvX = CurrentX - DestX;
				float IntvY = CurrentY - DestY;
				return 180 + (atan(IntvX / IntvY)*(180 / M_PI));
			}
		}
		return 0;
	}
}//ORIENTATION

namespace ENGINES {
	enum ConfPIN {
		FRONT_LEFT = 1,
		FRONT_RIGHT = 2,
		BACK_LEFT = 3,
		BACK_RIGHT = 4
	};

	class Rotor {
	public:
		Rotor(ConfPIN CP) {
			Thrust = 0;
			Pos = CP;
		}
		int SetThrust(int Thr) {
			return 0;
		};
		int GetThrust() { return Thrust; }
	private:
		int Thrust;
		ConfPIN Pos;
	};

	class Frame {
	public:
		Frame() {
			Fr_Le = new Rotor(FRONT_LEFT);
			Fr_Ri = new Rotor(FRONT_RIGHT);
			Ba_Le = new Rotor(BACK_LEFT);
			Ba_Ri = new Rotor(BACK_RIGHT);
		}
		int UpdateThrust() {
			float RadRotX = POSITION::Current.GetRotX() / 180 * M_PI;
			float RadRotY = POSITION::Current.GetRotY() / 180 * M_PI;
			float tGeneral = (Fhover
				* (1 + POSITION::Current.GetkAlt() / 1000)
				/ cos(RadRotX))
				/ cos(RadRotY);
			float tClockwise = (tGeneral / 4)
				* (1 - POSITION::Current.GetkRot() / 1000);
			float tCounterClockwise = (tGeneral / 4)
				* (1 + POSITION::Current.GetkRot() / 1000);
			Fr_Le->SetThrust(ceilf(tClockwise));
			Fr_Ri->SetThrust(ceilf(tCounterClockwise));
			Ba_Le->SetThrust(ceilf(tCounterClockwise));
			Ba_Ri->SetThrust(ceilf(tClockwise));
			return 0;
		}
		int MeasureHover() {
			float StartAlt = SENSORS::ALT14.GetAltitude();
			float Alt1;
			float Alt2;
			int Hover = 1;
			while (Hover == 1){
				Alt1 = SENSORS::ALT14.GetAltitude();
				Alt2 = SENSORS::ALT14.GetAltitude();
				if ((Alt1 + Alt2) / 2 > StartAlt) {
					Fhover--;
					UpdateThrust();
				}
				else if ((Alt1 + Alt2) / 2 < StartAlt) {
					Fhover++;
					UpdateThrust();
				}
				else {
					Hover = 0;
				}
			}
			return 0;
		}

		Rotor * Fr_Le;
		Rotor * Fr_Ri;
		Rotor * Ba_Le;
		Rotor * Ba_Ri;
	private:
		int Fhover = 500;

	};

	Frame ThisCopter;
}//ENGINES


