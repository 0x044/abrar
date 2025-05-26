#include "../fleetXpress.h"
#include <gtest/gtest.h>

vector<Vehicle> populateExpectedVehicles()
{
    vector<Vehicle> vec;
    ifstream ff("giventests/vehicles.txt");
    Vehicle v;
    
    while (ff >> v.VID >> v.VMfr >> v.VType >> v.VEngine >> v.VLastOdometerReading >> v.VStatus)
        vec.push_back(v);
    
    ff.close();
    return vec;
}

TEST(TestFleetOperations, GivenFleet)
{
    Fleet f("fleetData.json", "giventests/fleet.db", "rentalData.json");

    vector<Vehicle> exp_v = populateExpectedVehicles();
    vector<Vehicle> v = f.getAllFleetData();
    
    ASSERT_EQ(16, v.size());
    for (int i = 0; i < exp_v.size(); ++i)
    {
        ASSERT_EQ(exp_v[i].VID, v[i].VID);
        ASSERT_EQ(exp_v[i].VMfr, v[i].VMfr);
        ASSERT_EQ(exp_v[i].VType, v[i].VType);
        ASSERT_EQ(exp_v[i].VEngine, v[i].VEngine);
        ASSERT_EQ(exp_v[i].VLastOdometerReading, v[i].VLastOdometerReading);
        ASSERT_EQ(exp_v[i].VStatus, v[i].VStatus);
    }

    Allotment a = f.issueVehicle("Sedan", "TX123", 4);
    Allotment exp_a = Allotment{};
    string t;
    time_t curr_time = time(nullptr);

    exp_a.AID = "954e0084";
    exp_a.ALicense = "TX123";
    converttmDateTimeTostrDatestrTime(curr_time, exp_a.AIssueDate, t);
    converttmDateTimeTostrDatestrTime(curr_time + 4 * 24 * 60 * 60, exp_a.AExpectedReturnDate, t);
    exp_a.AEstimatedCost = 24000;
    exp_a.ASecurityDeposit = 12000;
    exp_a.AStartingOdometerReading = 30;
    
    ASSERT_EQ(exp_a.AID, a.AID);
    ASSERT_EQ(exp_a.ALicense, a.ALicense);
    ASSERT_EQ(exp_a.AIssueDate, a.AIssueDate);
    ASSERT_EQ(exp_a.AExpectedReturnDate, a.AExpectedReturnDate);
    ASSERT_EQ(exp_a.AActualReturnDate, a.AActualReturnDate);
    ASSERT_EQ(exp_a.AEstimatedCost, a.AEstimatedCost);
    ASSERT_EQ(exp_a.ASecurityDeposit, a.ASecurityDeposit);
    ASSERT_EQ(exp_a.AFinalCost, a.AFinalCost);
    ASSERT_EQ(exp_a.AStartingOdometerReading, a.AStartingOdometerReading);
    ASSERT_EQ(exp_a.AEndingOdometerReading, a.AEndingOdometerReading);

    int amount = f.returnVehicle("954e0084", 50);
    ASSERT_EQ(-12000, amount);

    string vehicleStatus = f.checkVehicleStatus("5c500b88");
    ASSERT_EQ("Available", vehicleStatus);

    exp_v[0].VLastOdometerReading = 50;

    v = f.allVehicleAvailability();
    ASSERT_EQ(16, v.size());
    for (int i = 0; i < exp_v.size(); ++i)
    {
        ASSERT_EQ(exp_v[i].VID, v[i].VID);
        ASSERT_EQ(exp_v[i].VMfr, v[i].VMfr);
        ASSERT_EQ(exp_v[i].VType, v[i].VType);
        ASSERT_EQ(exp_v[i].VEngine, v[i].VEngine);
        ASSERT_EQ(exp_v[i].VLastOdometerReading, v[i].VLastOdometerReading);
        ASSERT_EQ(exp_v[i].VStatus, v[i].VStatus);
    }

    curr_time = time(nullptr);
    string startDate, endDate;
    converttmDateTimeTostrDatestrTime(curr_time - 24 * 60 * 60, startDate, t);
    converttmDateTimeTostrDatestrTime(curr_time + 24 * 60 * 60, endDate, t);
    vector<Allotment> vec_allot = f.getVehicleRentalHistory("954e0084", startDate, endDate);
    
    exp_a.AID = "954e0084";
    exp_a.ALicense = "TX123";
    converttmDateTimeTostrDatestrTime(curr_time, exp_a.AIssueDate, t);
    converttmDateTimeTostrDatestrTime(curr_time + 4 * 24 * 60 * 60, exp_a.AExpectedReturnDate, t);
    converttmDateTimeTostrDatestrTime(curr_time, exp_a.AActualReturnDate, t);
    exp_a.AEstimatedCost = 24000;
    exp_a.ASecurityDeposit = 12000;
    exp_a.AFinalCost = 24000;
    exp_a.AStartingOdometerReading = 30;
    exp_a.AEndingOdometerReading = 50;
    
    ASSERT_EQ(1, vec_allot.size());
    ASSERT_EQ(exp_a.AID, vec_allot[0].AID);
    ASSERT_EQ(exp_a.ALicense, vec_allot[0].ALicense);
    ASSERT_EQ(exp_a.AIssueDate, vec_allot[0].AIssueDate);
    ASSERT_EQ(exp_a.AExpectedReturnDate, vec_allot[0].AExpectedReturnDate);
    ASSERT_EQ(exp_a.AActualReturnDate, vec_allot[0].AActualReturnDate);
    ASSERT_EQ(exp_a.AEstimatedCost, vec_allot[0].AEstimatedCost);
    ASSERT_EQ(exp_a.ASecurityDeposit, vec_allot[0].ASecurityDeposit);
    ASSERT_EQ(exp_a.AFinalCost, vec_allot[0].AFinalCost);
    ASSERT_EQ(exp_a.AStartingOdometerReading, vec_allot[0].AStartingOdometerReading);
    ASSERT_EQ(exp_a.AEndingOdometerReading, vec_allot[0].AEndingOdometerReading);
    
    int income = f.incomeOnVehicle("954e0084", startDate, endDate);
    ASSERT_EQ(24000, income);

    int totalIncome = f.incomeOnFleet(startDate, endDate);
    ASSERT_EQ(24000, income);
}