#include <iostream>
#include <fstream>
#include <json/json.h>
#include "utilityFunctions.h"
#include "fleet.h"

using namespace std;

void displayUserMenu(Fleet& fleet)
{
    int choice;
    do
    {
        cout << "\nUser Menu:\n";
        cout << "1. Get All Fleet Data\n";
        cout << "2. Issue Vehicle\n";
        cout << "3. Return Vehicle\n";
        cout << "4. Check Vehicle Status\n";
        cout << "5. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            fleet.getAllFleetData();
            break;
        case 2:
            // Call issueVehicle
            break;
        case 3:
            // Call returnVehicle
            break;
        case 4:
            // Call checkVehicleStatus
            break;
        case 5:
            cout << "Exiting User Menu...\n";
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 5);
}

void displayAdminMenu(Fleet& fleet)
{
    int choice;
    do
    {
        cout << "\nAdmin Menu:\n";
        cout << "1. Get All Fleet Data\n";
        cout << "2. Issue Vehicle\n";
        cout << "3. Return Vehicle\n";
        cout << "4. Check Vehicle Status\n";
        cout << "5. Get All Vehicle Availability\n";
        cout << "6. Get Vehicle Rental History\n";
        cout << "7. Income On Vehicle\n";
        cout << "8. Income On Fleet\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            fleet.getAllFleetData();
            break;
        case 2:
            // Call issueVehicle
            break;
        case 3:
            // Call returnVehicle
            break;
        case 4:
            // Call checkVehicleStatus
            break;
        case 5:
            fleet.allVehicleAvailability();
            break;
        case 6:
            // Call getVehicleRentalHistory
            break;
        case 7:
            // Call incomeOnVehicle
            break;
        case 8:
            // Call incomeOnFleet
            break;
        case 9:
            cout << "Exiting Admin Menu...\n";
            break;
        default:
            cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 9);
}

int main()
{
    Fleet fleet("fleetData.json", "fleet.db", "rentalData.json");

    string username, password;
    cout << "Enter username: ";
    cin >> username;
    cout << "Enter password: ";
    cin >> password;

    string role = authenticateUser(username, password);

    if (role == "user")
    {
        displayUserMenu(fleet);
    }
    else if (role == "admin")
    {
        displayAdminMenu(fleet);
    }
    else
    {
        cout << "Authentication failed. Exiting...\n";
    }

    return 0;
}