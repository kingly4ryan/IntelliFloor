#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <bits/stdc++.h>

using namespace std;

// Structure to represent a floor plan
struct FloorPlan {
    string floorPlanId;
    string floorPlanData;
    time_t timestamp; // Timestamp of last update
    string username; // Username of the uploader
};

// Structure to represent an admin user
struct AdminUser {
    string username;
    string password;
    int priority; // Priority level of the admin user
};

// Structure to represent a meeting room
struct MeetingRoom {
    string roomId;
    int capacity;
    bool available;
};

// Global variables
map<string, FloorPlan> floorPlans;
vector<pair<string, string>> offlineChanges;
vector<AdminUser> adminUsers = {{"admin", "password", 1}, {"manager", "manage", 2}}; // Default admin user with priority 1
vector<MeetingRoom> meetingRooms = {{"1", 10, true}, {"2", 15, true}, {"3", 20, true}};
const string backupFileName = "backup_floor_plans.txt"; // Name of the backup file

// Function to authenticate admin users
bool authenticateAdmin(const string& username, const string& password) {
    auto it = find_if(adminUsers.begin(), adminUsers.end(), [&](const AdminUser& user) {
        return user.username == username && user.password == password;
    });

    return it != adminUsers.end();
}

// Function to get the priority of a user
int getUserPriority(const string& username) {
    auto it = find_if(adminUsers.begin(), adminUsers.end(), [&](const AdminUser& user) {
        return user.username == username;
    });
    if (it != adminUsers.end()) {
        return it->priority;
    }
    // Default priority if user not found
    return INT_MAX;
}

// Function to save floor plans to a file
void saveFloorPlansToFile(const string& filename) {
    ofstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Unable to open file for writing" << endl;
        return;
    }

    for (const auto& pair : floorPlans) {
        file << pair.second.floorPlanId << "," << pair.second.floorPlanData << "," << pair.second.timestamp << endl;
    }

    file.close();
    cout << "Floor plans saved to file successfully" << endl;
}

// Function to upload a floor plan with conflict resolution based on user priority
void uploadFloorPlan(const string& username, const string& password) {
    string floorPlanId, floorPlanData;

    cout << "Enter Floor Plan ID: ";
    cin >> floorPlanId;

    cout << "Enter Floor Plan Data: ";
    cin.ignore(); // Ignore newline character
    getline(cin, floorPlanData);

    if (!authenticateAdmin(username, password)) {
        cout << "Error: Authentication failed" << endl;
        return;
    }

    if (floorPlans.find(floorPlanId) != floorPlans.end()) {
        // Handle conflict resolution by comparing user priorities
        int currentUserPriority = getUserPriority(username);
        int existingUserPriority = getUserPriority(floorPlans[floorPlanId].username);
        if (currentUserPriority < existingUserPriority) {
            floorPlans[floorPlanId].floorPlanData = floorPlanData;
            floorPlans[floorPlanId].timestamp = time(nullptr);
            floorPlans[floorPlanId].username = username; // Update the username of the uploader
            cout << "Conflict resolved: Floor plan updated successfully" << endl;
        } else {
            cout << "You don't have sufficient privileges to update this floor plan" << endl;
        }
    } else {
        FloorPlan newFloorPlan;
        newFloorPlan.floorPlanId = floorPlanId;
        newFloorPlan.floorPlanData = floorPlanData;
        newFloorPlan.timestamp = time(nullptr);
        newFloorPlan.username = username; // Store the username of the uploader

        floorPlans[floorPlanId] = newFloorPlan;
        cout << "Floor plan uploaded successfully" << endl;

        // Trigger backup after uploading a new floor plan
        saveFloorPlansToFile(backupFileName);
    }
}

// Function to synchronize offline changes
void synchronizeOfflineChanges(const string& username, const string& password) {
    if (!authenticateAdmin(username, password)) {
        cout << "Error: Authentication failed" << endl;
        return;
    }

    for (const auto& change : offlineChanges) {
        uploadFloorPlan(username, password);
    }
    offlineChanges.clear();
    cout << "Offline changes synchronized successfully" << endl;
}

// Function to load floor plans from a file
void loadFloorPlansFromFile(const string& filename) {
    ifstream file(filename);

    if (!file.is_open()) {
        cout << "Error: Unable to open file for reading" << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        size_t pos1 = line.find(',');
        size_t pos2 = line.find(',', pos1 + 1);
        if (pos1 != string::npos && pos2 != string::npos) {
            string floorPlanId = line.substr(0, pos1);
            string floorPlanData = line.substr(pos1 + 1, pos2 - pos1 - 1);
            time_t timestamp = stoi(line.substr(pos2 + 1));
            floorPlans[floorPlanId].floorPlanId = floorPlanId;
            floorPlans[floorPlanId].floorPlanData = floorPlanData;
            floorPlans[floorPlanId].timestamp = timestamp;
        }
    }

    file.close();
    cout << "Floor plans loaded from file successfully" << endl;
}

// Function to suggest the best meeting room based on capacity and availability
string suggestMeetingRoom(int requiredCapacity) {
    int low = 0, high = meetingRooms.size() - 1, mid;
    string result = "";

    while (low <= high) {
        mid = low + (high - low) / 2;
        if (meetingRooms[mid].capacity >= requiredCapacity && meetingRooms[mid].available) {
            result = meetingRooms[mid].roomId;
            high = mid - 1; // Look for better options on the left side
        } else {
            low = mid + 1; // Look for options on the right side
        }
    }

    return result;
}

// Function to handle system failures and recover data integrity
void handleSystemFailure() {
    cout << "System failure detected. Recovering data integrity..." << endl;
    
    // Attempt to recover data from backup file
    loadFloorPlansFromFile(backupFileName);

    cout << "Data integrity recovered successfully" << endl;
}

// Function for error and exception handling
void handleError(const string& errorMessage) {
    cout << "Error: " << errorMessage << endl;
    // Implement error handling strategies here
    // For the sake of this example, we will simply log the error and continue execution
}

// Function to display the current floor plans
void displayFloorPlans() {
    cout << "Current Floor Plans:" << endl;
    for (const auto& pair : floorPlans) {
        cout << "Floor Plan ID: " << pair.second.floorPlanId << endl;
        cout << "Floor Plan Data: " << pair.second.floorPlanData << endl;
        cout << "Timestamp: " << ctime(&pair.second.timestamp); // Convert timestamp to string for readability
        cout << "Uploader: " << pair.second.username << endl; // Display the uploader username
        cout << endl;
    }
}

// Function to display the main menu
void displayMainMenu() {
    cout << "Main Menu" << endl;
    cout << "1. Upload Floor Plan" << endl;
    cout << "2. Synchronize Offline Changes" << endl;
    cout << "3. Suggest Meeting Room" << endl;
    cout << "4. Display Current Floor Plans" << endl;
    cout << "5. Exit" << endl;    
}



// Main function
int main() {



    // Load floor plans from file
    loadFloorPlansFromFile("floor_plans.txt");

    string username, password;
    bool authenticated = false;
    int attempts = 0;
    const int MAX_ATTEMPTS = 5;
    bool isAdmin = false;

    // User authentication loop with limit
    do {
        cout << "Enter username: ";
        cin >> username;
        cout << "Enter password: ";
        cin >> password;
        authenticated = authenticateAdmin(username, password);
        attempts++;
        if (!authenticated) {
            cout << "Authentication failed. Please try again." << endl;
            if (attempts >= MAX_ATTEMPTS) {
                cout << "You have exceeded the limit for authentication attempts. Exiting program." << endl;
                return 1; // Exit the program
            }
        } else {
            isAdmin = true;
        }
    } while (!authenticated && attempts < MAX_ATTEMPTS);

    int choice;

    if (isAdmin) {
        do {
            // Display main menu for admin mode
            displayMainMenu();
            // Prompt user for choice
            cout << "Enter your choice (1-5): ";
            cin >> choice;
            // Perform action based on user choice
            switch (choice) {
                case 1:
                    uploadFloorPlan(username, password);
                    break;
                case 2:
                    synchronizeOfflineChanges(username, password);
                    break;
                case 3:
                    {
                        int requiredCapacity;
                        cout << "Enter Required Capacity: ";
                        cin >> requiredCapacity;
                        string suggestedRoom = suggestMeetingRoom(requiredCapacity);
                        if (!suggestedRoom.empty()) {
                            cout << "Suggested meeting room: " << suggestedRoom << endl;
                        } else {
                            cout << "No available meeting room found" << endl;
                        }
                    }
                    break;
                case 4:
                    displayFloorPlans();
                    break;
                case 5:
                    cout << "Exiting program. Goodbye!" << endl;
                    return 0; // Exit the program
                default:
                    cout << "Invalid choice. Please enter a number between 1 and 5." << endl;
                    break;
            }
            // Clear screen for better readability
            cout << endl;
        } while (true);
    }

    return 0;
}
