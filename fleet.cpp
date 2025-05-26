#include "fleetXpress.h"

Fleet::Fleet()
{
}

Fleet::Fleet(string fleetJsonFilename, string fleetDbFilename, string rentalDataJsonFilename)
{
    // Initialize SQLite database
    if (sqlite3_open(fleetDbFilename.c_str(), &dbase) != SQLITE_OK)
    {
        cerr << "Error opening database: " << sqlite3_errmsg(dbase) << endl;
        exit(EXIT_FAILURE);
    }

    // Create vehicles table
    string createVehiclesTable = "CREATE TABLE IF NOT EXISTS vehicles (" \
                                 "VID TEXT PRIMARY KEY, " \
                                 "VMfr TEXT, " \
                                 "VType TEXT, " \
                                 "VEngine TEXT, " \
                                 "VLastOdometerReading INTEGER, " \
                                 "VStatus TEXT);";
    if (sqlite3_exec(dbase, createVehiclesTable.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        cerr << "Error creating vehicles table: " << sqlite3_errmsg(dbase) << endl;
        exit(EXIT_FAILURE);
    }

    // Create allotments table
    string createAllotmentsTable = "CREATE TABLE IF NOT EXISTS allotments (" \
                                   "AID TEXT PRIMARY KEY, " \
                                   "ALicense TEXT, " \
                                   "AIssueDate TEXT, " \
                                   "AExpectedReturnDate TEXT, " \
                                   "AActualReturnDate TEXT, " \
                                   "AEstimatedCost INTEGER, " \
                                   "ASecurityDeposit INTEGER, " \
                                   "AFinalCost INTEGER, " \
                                   "AStartingOdometerReading INTEGER, " \
                                   "AEndingOdometerReading INTEGER);";
    if (sqlite3_exec(dbase, createAllotmentsTable.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
    {
        cerr << "Error creating allotments table: " << sqlite3_errmsg(dbase) << endl;
        exit(EXIT_FAILURE);
    }

    // Load fleet data from JSON
    ifstream fleetFile(fleetJsonFilename);
    Json::Value fleetData;
    fleetFile >> fleetData;

    for (const auto& vehicle : fleetData["vehicles"])
    {
        string insertVehicle = "INSERT OR IGNORE INTO vehicles (VID, VMfr, VType, VEngine, VLastOdometerReading, VStatus) VALUES (" \
                              "'" + vehicle["VID"].asString() + "', " \
                              "'" + vehicle["VMfr"].asString() + "', " \
                              "'" + vehicle["VType"].asString() + "', " \
                              "'" + vehicle["VEngine"].asString() + "', " \
                              + to_string(vehicle["VLastOdometerReading"].asInt()) + ", " \
                              "'Available');";
        if (sqlite3_exec(dbase, insertVehicle.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            cerr << "Error inserting vehicle: " << sqlite3_errmsg(dbase) << endl;
        }
    }

    // Load rental pricing data into memory
    ifstream rentalFile(rentalDataJsonFilename);
    rentalFile >> rentalData;
}

vector<Vehicle> Fleet::getAllFleetData()
{
}

Allotment Fleet::issueVehicle(string vehicleType, string licenseNumber, int numOfDays)
{
    Allotment allotment;

    // Query the database for the first available vehicle of the requested type
    sqlite3_stmt* stmt;
    string query = "SELECT VID FROM vehicles WHERE VType = ? AND VStatus = 'Available' LIMIT 1";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, vehicleType.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            string vehicleID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

            // Update vehicle status to 'Issued'
            string updateQuery = "UPDATE vehicles SET VStatus = 'Issued' WHERE VID = '" + vehicleID + "'";
            if (sqlite3_exec(dbase, updateQuery.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                cerr << "Error updating vehicle status: " << sqlite3_errmsg(dbase) << endl;
            }

            // Calculate estimated cost and security deposit
            int dailyRent = rentalData[vehicleType]["perDayRent"].asInt();
            int estimatedCost = dailyRent * numOfDays;
            int securityDeposit = 2 * dailyRent;

            // Populate allotment details
            allotment.AID = vehicleID;
            allotment.ALicense = licenseNumber;
            allotment.AIssueDate = getCurrentDate();
            allotment.AExpectedReturnDate = getFutureDate(numOfDays);
            allotment.AEstimatedCost = estimatedCost;
            allotment.ASecurityDeposit = securityDeposit;
            allotment.AStartingOdometerReading = 0; // Placeholder, should be fetched from the database

            // Insert allotment into the database
            string insertQuery = "INSERT INTO allotments (AID, ALicense, AIssueDate, AExpectedReturnDate, AEstimatedCost, ASecurityDeposit, AStartingOdometerReading) VALUES ('" +
                                 vehicleID + "', '" + licenseNumber + "', '" + allotment.AIssueDate + "', '" + allotment.AExpectedReturnDate + "', " +
                                 to_string(estimatedCost) + ", " + to_string(securityDeposit) + ", 0)";
            if (sqlite3_exec(dbase, insertQuery.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                cerr << "Error inserting allotment: " << sqlite3_errmsg(dbase) << endl;
            }
        }
        else
        {
            cerr << "No available vehicle of type " << vehicleType << endl;
        }
    }

    sqlite3_finalize(stmt);
    return allotment;
}

int Fleet::returnVehicle(string vehicleNumber, int odometerReading)
{
    int returnAmount = 0;

    // Query the database for the allotment details
    sqlite3_stmt* stmt;
    string query = "SELECT AEstimatedCost, ASecurityDeposit, AStartingOdometerReading, AExpectedReturnDate FROM allotments WHERE AID = ? AND AActualReturnDate IS NULL";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, vehicleNumber.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int estimatedCost = sqlite3_column_int(stmt, 0);
            int securityDeposit = sqlite3_column_int(stmt, 1);
            int startingOdometer = sqlite3_column_int(stmt, 2);
            string expectedReturnDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            // Validate odometer reading
            if (odometerReading < startingOdometer)
            {
                return INT_MAX; // Invalid odometer reading
            }

            // Calculate extra charges for late return or excess kilometers
            int extraDayCharges = 0;
            int extraKmCharges = 0;
            int allowedKm = rentalData["allowedKmPerDay"].asInt() * getDaysBetweenDates(expectedReturnDate, getCurrentDate());

            if (odometerReading - startingOdometer > allowedKm)
            {
                extraKmCharges = (odometerReading - startingOdometer - allowedKm) * rentalData["perKmRent"].asInt();
            }

            if (isDateAfter(getCurrentDate(), expectedReturnDate))
            {
                extraDayCharges = getDaysBetweenDates(expectedReturnDate, getCurrentDate()) * rentalData["perDayRent"].asInt();
            }

            int finalCost = estimatedCost + extraDayCharges + extraKmCharges;
            returnAmount = securityDeposit - finalCost;

            // Update allotment with return details
            string updateQuery = "UPDATE allotments SET AActualReturnDate = '" + getCurrentDate() + "', AFinalCost = " + to_string(finalCost) + ", AEndingOdometerReading = " + to_string(odometerReading) + " WHERE AID = '" + vehicleNumber + "'";
            if (sqlite3_exec(dbase, updateQuery.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                cerr << "Error updating allotment: " << sqlite3_errmsg(dbase) << endl;
            }

            // Update vehicle status to 'Available'
            string vehicleUpdateQuery = "UPDATE vehicles SET VStatus = 'Available' WHERE VID = '" + vehicleNumber + "'";
            if (sqlite3_exec(dbase, vehicleUpdateQuery.c_str(), nullptr, nullptr, nullptr) != SQLITE_OK)
            {
                cerr << "Error updating vehicle status: " << sqlite3_errmsg(dbase) << endl;
            }
        }
        else
        {
            return INT_MIN; // Vehicle not issued
        }
    }

    sqlite3_finalize(stmt);
    return returnAmount;
}

string Fleet::checkVehicleStatus(string vehicleNumber)
{
    string status;

    // Query the database for the status of the specified vehicle
    sqlite3_stmt* stmt;
    string query = "SELECT VStatus FROM vehicles WHERE VID = ?";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, vehicleNumber.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        else
        {
            status = "Vehicle not found";
        }
    }
    else
    {
        cerr << "Error querying vehicle status: " << sqlite3_errmsg(dbase) << endl;
    }

    sqlite3_finalize(stmt);
    return status;
}

vector<Vehicle> Fleet::allVehicleAvailability()
{
    vector<Vehicle> availableVehicles;

    // Query the database for all available vehicles
    sqlite3_stmt* stmt;
    string query = "SELECT VID, VMfr, VType, VEngine, VLastOdometerReading, VStatus FROM vehicles WHERE VStatus = 'Available'";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Vehicle vehicle;
            vehicle.VID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            vehicle.VMfr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            vehicle.VType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            vehicle.VEngine = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            vehicle.VLastOdometerReading = sqlite3_column_int(stmt, 4);
            vehicle.VStatus = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

            availableVehicles.push_back(vehicle);
        }
    }
    else
    {
        cerr << "Error querying available vehicles: " << sqlite3_errmsg(dbase) << endl;
    }

    sqlite3_finalize(stmt);
    return availableVehicles;
}

vector<Allotment> Fleet::getVehicleRentalHistory(string vehicleNumber, string strStartDate, string strEndDate)
{
    vector<Allotment> rentalHistory;

    // Query the database for rental history of the given vehicle within the date range
    // Ensure proper error handling and resource management
    sqlite3_stmt* stmt;
    string query = "SELECT * FROM RentalData WHERE VehicleID = ? AND IssueDate >= ? AND ReturnDate <= ?";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, vehicleNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, strStartDate.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, strEndDate.c_str(), -1, SQLITE_STATIC);

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            Allotment allotment;
            allotment.AID = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            allotment.ALicense = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            allotment.AIssueDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            allotment.AExpectedReturnDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            allotment.AActualReturnDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            allotment.AEstimatedCost = sqlite3_column_int(stmt, 5);
            allotment.ASecurityDeposit = sqlite3_column_int(stmt, 6);
            allotment.AFinalCost = sqlite3_column_int(stmt, 7);
            allotment.AStartingOdometerReading = sqlite3_column_int(stmt, 8);
            allotment.AEndingOdometerReading = sqlite3_column_int(stmt, 9);

            rentalHistory.push_back(allotment);
        }
    }

    sqlite3_finalize(stmt);
    return rentalHistory;
}

int Fleet::incomeOnVehicle(string vehicleNumber, string strStartDate, string strEndDate)
{
    int totalIncome = 0;

    // Query the database for income on the given vehicle within the date range
    sqlite3_stmt* stmt;
    string query = "SELECT SUM(FinalCost) FROM RentalData WHERE VehicleID = ? AND IssueDate >= ? AND ReturnDate <= ?";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, vehicleNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, strStartDate.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, strEndDate.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            totalIncome = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return totalIncome;
}

int Fleet::incomeOnFleet(string strStartDate, string strEndDate)
{
    int totalIncome = 0;

    // Query the database for total income on the fleet within the date range
    sqlite3_stmt* stmt;
    string query = "SELECT SUM(FinalCost) FROM RentalData WHERE IssueDate >= ? AND ReturnDate <= ?";

    if (sqlite3_prepare_v2(dbase, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, strStartDate.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, strEndDate.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            totalIncome = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return totalIncome;
}

Fleet::~Fleet()
{
}