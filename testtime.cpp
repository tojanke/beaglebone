#include <iostream>
using namespace std;
#include <time.h>

int main()
{
   time_t theTime = time(0);
	struct tm *aTime = localtime(&theTime);
	int day = aTime->tm_mday;
	int month = aTime->tm_mon;
	int year = aTime->tm_year;
	int hour = aTime->tm_hour;
	int minute = aTime->tm_min;
	int second = aTime->tm_sec;
	cout << day << " "<< month << " " << year << " " << hour << " " << minute << " " << second << endl;
}