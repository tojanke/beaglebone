#include <iostream>
#include "multirotor.h"
#include <ymath.h>

using namespace std;

void main() {
	cout << "Orientation Test" << endl;
	cout << "Degrees for 150% Forward: " << ORIENTATION::GetRotX(150) << endl;
	cout << "Degrees for 150% Backward: " << ORIENTATION::GetRotX(-150) << endl;
	cout << "Degrees for 120% Right: " << ORIENTATION::GetRotY(120) << endl;
	cout << "Degrees for 120% Left: " << ORIENTATION::GetRotY(-120) << endl;
	cout << "Degrees for (1|2) to (4|8): " << ORIENTATION::GetRotZ(1, 2, 4, 8) << endl;
	cout << "Degrees for (4|1) to (3|5): " << ORIENTATION::GetRotZ(4, 1, 3, 5) << endl;
	cout << "Degrees for (1|4) to (5|3): " << ORIENTATION::GetRotZ(1, 4, 5, 3) << endl;
	cout << "Degrees for (8|8) to (2|2): " << ORIENTATION::GetRotZ(8, 8, 2, 2) << endl;
	POSITION::PosData* CP = &POSITION::Current;
	CP->SetRotX(27);
	CP->SetRotY(36);
	CP->SetkAlt(87);
	CP->SetkRot(-37);
	ENGINES::Frame* TC = &ENGINES::ThisCopter;
	TC->UpdateThrust();
	cout << "##########" << endl;
	cout << "Engine Readings for:" << endl << "Rotation X: " << CP->GetRotX() << endl << "Rotation Y: " << CP->GetRotY() << endl << "Altitude Constant: " << CP->GetkAlt() << "%" << endl
		<< "Rotation Constant: " << CP->GetkRot() << endl;
	cout << "##########" << endl;

	cout << "Engine 1 (Front Left): " << TC->Fr_Le->GetThrust() << endl;
	cout << "Engine 2 (Front Right): " << TC->Fr_Ri->GetThrust() << endl;
	cout << "Engine 3 (Back Left): " << TC->Ba_Le->GetThrust() << endl;
	cout << "Engine 4 (Back Right): " << TC->Ba_Ri->GetThrust() << endl;
	int asdf;
	cin >> asdf;
}
