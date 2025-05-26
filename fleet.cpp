#include "fleetXpress.h"

Fleet::Fleet()
{
}

Fleet::Fleet(string fleetJsonFilename, string fleetDbFilename, string rentalDataJsonFilename)
{	
}

vector<Vehicle> Fleet::getAllFleetData()
{
}

Allotment Fleet::issueVehicle(string vehicleType, string licenseNumber, int numOfDays)
{
}

int Fleet::returnVehicle(string vehicleNumber, int odometerReading)
{
}

string Fleet::checkVehicleStatus(string vehicleNumber)
{
}

vector<Vehicle> Fleet::allVehicleAvailability()
{
}

vector<Allotment> Fleet::getVehicleRentalHistory(string vehicleNumber, string strStartDate, string strEndDate)
{
}

int Fleet::incomeOnVehicle(string vehicleNumber, string strStartDate, string strEndDate)
{
}

int Fleet::incomeOnFleet(string strStartDate, string strEndDate)
{
}

Fleet::~Fleet()
{
}