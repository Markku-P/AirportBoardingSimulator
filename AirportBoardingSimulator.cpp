/*
Airport Boarding Simulator
This was one home assignment of Concurrent Programming course

By Markku-P 2020

Assignment description
----------------------
4 people are at the airport. To prepare for departure, each of them has to first scan their boarding pass (which takes 1 min), and then to do the security check (which takes 10 minutes).

Assume that there is only one machine for scanning the boarding pass and only one security line. Explain why this pipeline is unbalanced. Compute its throughput.
Now assume that there are 2 security lines. Which is the new throughput?
If there were 4 security lines opened, would the pipeline be balanced?

Implement a test program to verify your findings.
Tip: You can simulate the throughput faster than 1/10 minutes. But after simulation, show the results based on original 1/10 minutes.
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;
using namespace std::chrono;

class People
{
public:
	int idNumber;
	bool boardingPassChecked = false;
	bool securityChecked = false;
	bool busy = false;

	People(int id)
	{
		idNumber = id;
	}
};

class BoardingPassScanner
{
public:

	BoardingPassScanner()
	{

	}

	void scanBoardingPass(People& people, atomic<int>& scannersInUse)
	{
		cout << "BOARDING PASS SCAN STARTED: people id " << people.idNumber << endl;

		// Delay 1min ( 1/10 = 6000ms )
		this_thread::sleep_for(chrono::milliseconds(6000));
		people.boardingPassChecked = true;
		people.busy = false;
		scannersInUse.fetch_sub(1);

		cout << "BOARDING PASS SCAN FINISHED: people id " << people.idNumber << endl;
	}
};

class SecurityLine
{
public:

	SecurityLine()
	{

	}

	void securityCheck(People& people, atomic<int>& securityLinesInUse)
	{
		cout << "SECURITY CHECK STARTED: people id " << people.idNumber << endl;

		// Delay 10min ( 1/10 = 1min )
		this_thread::sleep_for(chrono::minutes(1));
		people.securityChecked = true;
		people.busy = false;
		securityLinesInUse.fetch_sub(1);

		cout << "SECURITY CHECK FINISHED: people id " << people.idNumber << endl;
	}
};

int main()
{
	// Simulation start variables
	int boardingPassScannerCount = 1;
	int securityLineCount = 4;
	int peopleCount = 4;

	BoardingPassScanner boardingPassScanner;
	SecurityLine securityLine;
	vector<People> people;
	vector<thread> threads01;
	system_clock::time_point start = system_clock::now();

	// Create people
	for (int i = 0; i < peopleCount; i++)
	{
		people.push_back(People(i+1));
	}

	atomic<int> scannersInUse = 0;
	atomic<int> securityLinesInUse = 0;
	bool allChecked = false;

	while (!allChecked) {

		for (int i = 0; i < people.size(); i++)
		{
			// If boarding pass checking is not made
			if ((!people[i].boardingPassChecked) && (!people[i].busy))
			{
				if (scannersInUse.load() < boardingPassScannerCount)
				{
					people[i].busy = true;
					scannersInUse.fetch_add(1);
					threads01.push_back(thread(&BoardingPassScanner::scanBoardingPass, boardingPassScanner, ref(people[i]), ref(scannersInUse)));
				}
			}

			// If security check is not made
			if (people[i].boardingPassChecked && (!people[i].securityChecked) && (!people[i].busy))
			{
				if (securityLinesInUse.load() < securityLineCount)
				{
					people[i].busy = true;
					securityLinesInUse.fetch_add(1);
					threads01.push_back(thread(&SecurityLine::securityCheck, securityLine, ref(people[i]), ref(securityLinesInUse)));
				}
			}

			// Check if all the people boarding pass are checked and security checks are made
			allChecked = true;
			for (int j = 0; j < people.size(); j++)
			{
				if (!(people[j].boardingPassChecked && people[j].securityChecked))
				{
					allChecked = false;
				}
			}
		}
	}

	// Join all threads
	for (int i = 0; i < threads01.size(); i++)
	{
		if (threads01[i].joinable())
		{
			threads01[i].join();
		}
	}

	system_clock::time_point end = system_clock::now();

	// Calculate elapsed time and change time scale back to 10/1
	auto totalElapsedTime = duration_cast<seconds>(end - start) * 10;
	auto min = duration_cast<minutes>(totalElapsedTime);
	auto sec = duration_cast<seconds>((totalElapsedTime) - duration_cast<minutes>(totalElapsedTime));

	cout << "\nTotal time: " <<(min).count() << " Min " << (sec).count() << " Seconds" << endl;
	return 0;
}
