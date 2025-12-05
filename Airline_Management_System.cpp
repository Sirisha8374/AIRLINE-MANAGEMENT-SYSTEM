// flight_booking.cpp
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <iomanip>
#include <ctime>
#include <fstream>
#include <sstream>
#include <memory>
#include <algorithm>
#include <cctype>

using namespace std;

// ----------------- Utilities -----------------
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pressEnterToContinue() {
    cout << "\nPress Enter to continue...";
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    cin.get();
}

string getCurrentDateTime() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

bool isValidEmail(const string& email) {
    auto at = email.find('@');
    auto dot = email.find('.', at == string::npos ? 0 : at);
    return (at != string::npos && dot != string::npos && at < dot);
}

bool isValidPhone(const string& phone) {
    int digits = 0;
    for (char c : phone) {
        if (isdigit(c)) digits++;
        else if (c == '+' || c == '-' || c == ' ') continue;
        else return false;
    }
    return digits >= 10;
}

int readIntInRange(int minv, int maxv) {
    while (true) {
        int x;
        if (!(cin >> x)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Enter a number: ";
            continue;
        }
        if (x < minv || x > maxv) {
            cout << "Enter a number between " << minv << " and " << maxv << ": ";
            continue;
        }
        return x;
    }
}

// ----------------- Credentials -----------------
const string ADMIN_USER = "admin";
const string ADMIN_PASS = "admin123";

// ----------------- Enums -----------------
enum MealPreference { VEGETARIAN = 0, NON_VEG = 1, VEGAN = 2, NO_MEAL = 3 };
enum PaymentMethod { CREDIT_CARD = 0, DEBIT_CARD = 1, UPI = 2, CASH = 3 };

// ----------------- Passenger -----------------
struct Passenger {
    string name;
    string phone;
    string email;
    string gender;
    MealPreference meal = NO_MEAL;
    bool wheelchair = false;
    int luggageKg = 0;

    Passenger() = default;
    Passenger(string n, string p, string e, string g, MealPreference m, bool w, int l)
        : name(move(n)), phone(move(p)), email(move(e)), gender(move(g)), meal(m), wheelchair(w), luggageKg(l) {}

    string mealString() const {
        switch (meal) {
            case VEGETARIAN: return "Vegetarian";
            case NON_VEG: return "Non-Veg";
            case VEGAN: return "Vegan";
            default: return "No Meal";
        }
    }

    void displayFull() const {
        cout << "Name: " << name << ", Phone: " << phone;
        if (!email.empty()) cout << ", Email: " << email;
        if (!gender.empty()) cout << ", Gender: " << gender;
        cout << "\nMeal: " << mealString();
        cout << ", Wheelchair: " << (wheelchair ? "YES" : "NO");
        cout << ", Luggage: " << luggageKg << "kg\n";
    }

    void displayLimited() const {
        cout << "Name: " << name << ", Phone: " << phone << "\n";
    }
};

// ----------------- Seats -----------------
class Seat {
public:
    string seatNo;
    double basePrice;
    bool booked = false;
    string preference; // Window, Aisle, Middle

    Seat(string s="", double p=0.0, string pref="Middle") : seatNo(move(s)), basePrice(p), preference(move(pref)) {}
    virtual ~Seat() = default;
    virtual double price() const = 0;
    virtual string type() const = 0;
    void display() const {
        cout << left << setw(6) << seatNo << setw(12) << type() << setw(10) << preference
             << "$" << fixed << setprecision(2) << setw(8) << price()
             << (booked ? "BOOKED" : "AVAILABLE") << "\n";
    }
};

class Economy : public Seat {
public:
    Economy(string s, double p, string pref="Middle") : Seat(move(s), p, move(pref)) {}
    double price() const override { return basePrice; }
    string type() const override { return "Economy"; }
};

class Business : public Seat {
public:
    Business(string s, double p, string pref="Middle") : Seat(move(s), p, move(pref)) {}
    double price() const override { return basePrice * 2.0; }
    string type() const override { return "Business"; }
};

class FirstClass : public Seat {
public:
    FirstClass(string s, double p, string pref="Middle") : Seat(move(s), p, move(pref)) {}
    double price() const override { return basePrice * 3.0; }
    string type() const override { return "First Class"; }
};

// ----------------- Payment -----------------
struct Payment {
    double amount = 0.0;
    PaymentMethod method = CASH;
    string txnId;
    string timestamp;

    Payment() = default;
    Payment(double a, PaymentMethod m) : amount(a), method(m) {
        timestamp = getCurrentDateTime();
        txnId = "TXN" + to_string(rand() % 900000 + 100000);
    }

    string methodStr() const {
        switch (method) {
            case CREDIT_CARD: return "Credit Card";
            case DEBIT_CARD: return "Debit Card";
            case UPI: return "UPI";
            case CASH: return "Cash";
            default: return "Unknown";
        }
    }

    void printReceipt() const {
        cout << "\n========== PAYMENT RECEIPT ==========\n";
        cout << "Transaction ID: " << txnId << "\n";
        cout << "Amount: $" << fixed << setprecision(2) << amount << "\n";
        cout << "Method: " << methodStr() << "\n";
        cout << "Time: " << timestamp << "\n";
        cout << "=====================================\n";
    }
};

// ----------------- Booking -----------------
struct Booking {
    int id;
    Passenger passenger;
    Seat* seat = nullptr; // pointer to flight seat
    unique_ptr<Payment> payment;
    string bookingTime;
    bool cancelled = false;

    Booking() = default;
    Booking(int i, Passenger p, Seat* s, unique_ptr<Payment> pay)
        : id(i), passenger(move(p)), seat(s), payment(move(pay)), cancelled(false) {
        bookingTime = getCurrentDateTime();
    }

    double totalAmount() const {
        if (!seat) return 0.0;
        double total = seat->price();
        if (passenger.luggageKg > 20) {
            total += (passenger.luggageKg - 20) * 10;
        }
        return total;
    }

    void displayLimited() const {
        cout << "\n--- Booking #" << id << " ---\n";
        passenger.displayLimited();
        if (seat) cout << "Seat: " << seat->seatNo << " (" << seat->type() << ")\n";
        cout << (cancelled ? "Status: CANCELLED\n" : "Status: CONFIRMED\n");
    }

    void displayFull() const {
        cout << "\n========== BOOKING DETAILS ==========\n";
        cout << "Booking ID: #" << id << "\n";
        cout << "Status: " << (cancelled ? "CANCELLED" : "CONFIRMED") << "\n";
        cout << "Booking Time: " << bookingTime << "\n\n";
        cout << "--- Passenger Info ---\n";
        passenger.displayFull();
        cout << "\n--- Seat Info ---\n";
        if (seat) seat->display();
        cout << "\n--- Payment Info ---\n";
        if (payment) {
            cout << "Base Fare: $" << fixed << setprecision(2) << seat->price() << "\n";
            if (passenger.luggageKg > 20) {
                cout << "Extra Luggage: $" << (passenger.luggageKg - 20) * 10 << "\n";
            }
            cout << "Total: $" << totalAmount() << "\n";
            cout << "Payment Method: " << payment->methodStr() << "\n";
            cout << "Transaction ID: " << payment->txnId << "\n";
        }
        cout << "=====================================\n";
    }

    void printBoardingPass() const {
        cout << "\n------------------------------------------\n";
        cout << "--         BOARDING PASS                  --\n";
        cout << "-----------------------------------------------\n";
        cout << " Booking ID: " << left << setw(27) << id << "\n";
        cout << " Passenger: " << left << setw(28) << passenger.name << "\n";
        cout << " Seat: " << left << setw(33) << (seat ? seat->seatNo : string("N/A")) << "\n";
        cout << " Class: " << left << setw(32) << (seat ? seat->type() : string("N/A")) << "\n";
        cout << " Meal: " << left << setw(33) << passenger.mealString() << "║\n";
        cout << "------------------------------------------------------\n";
    }
};

// ----------------- Flight -----------------
class Flight {
public:
    string flightNo;
    string src, dest;
    string depTime, arrTime;
    vector<unique_ptr<Seat>> seats;

    Flight() = default;
    Flight(string f, string s, string d, string dep, string arr)
        : flightNo(move(f)), src(move(s)), dest(move(d)), depTime(move(dep)), arrTime(move(arr)) {}

    void initDefaultSeats() {
        seats.clear();
        // Economy rows 1-5: A-D
        for (int r=1; r<=5; ++r) {
            seats.emplace_back(make_unique<Economy>(to_string(r)+"A", 100.0, "Window"));
            seats.emplace_back(make_unique<Economy>(to_string(r)+"B", 100.0, "Middle"));
            seats.emplace_back(make_unique<Economy>(to_string(r)+"C", 100.0, "Aisle"));
            seats.emplace_back(make_unique<Economy>(to_string(r)+"D", 100.0, "Window"));
        }
        // Business 6-8: A-C
        for (int r=6; r<=8; ++r) {
            seats.emplace_back(make_unique<Business>(to_string(r)+"A", 150.0, "Window"));
            seats.emplace_back(make_unique<Business>(to_string(r)+"B", 150.0, "Aisle"));
            seats.emplace_back(make_unique<Business>(to_string(r)+"C", 150.0, "Window"));
        }
        // First 9-10: A-B
        for (int r=9; r<=10; ++r) {
            seats.emplace_back(make_unique<FirstClass>(to_string(r)+"A", 200.0, "Window"));
            seats.emplace_back(make_unique<FirstClass>(to_string(r)+"B", 200.0, "Aisle"));
        }
    }

    Seat* findSeat(const string& seatNo) {
        for (auto& s : seats) {
            if (s->seatNo == seatNo) return s.get();
        }
        return nullptr;
    }

    void displayInfo() const {
        cout << flightNo << " : " << src << " -> " << dest
             << " | Dep: " << depTime << " Arr: " << arrTime << "\n";
    }

    void displaySeatMap() const {
        cout << "\n========== SEAT MAP ==========\n";
        // Group by row number - gather rows
        map<int, vector<const Seat*>> rows;
        for (const auto& s : seats) {
            string num = s->seatNo.substr(0, s->seatNo.size()-1);
            int r = stoi(num);
            rows[r].push_back(s.get());
        }
        cout << "   A    B    C    D\n";
        for (const auto& kv : rows) {
            cout << setw(2) << kv.first << " ";
            // ensure ordering A B C D by checking letters A-D
            vector<char> order = {'A','B','C','D'};
            for (char ch : order) {
                const Seat* match = nullptr;
                for (const Seat* s : kv.second) if (!s->seatNo.empty() && s->seatNo.back() == ch) match = s;
                if (match) {
                    cout << "[" << (match->booked ? 'X' : match->seatNo.back()) << "] ";
                } else {
                    cout << "     ";
                }
            }
            cout << "\n";
        }
        cout << "\n[X] = Booked, [Letter] = Available\n";
    }
};

// ----------------- Waitlist -----------------
class Waitlist {
    queue<pair<Passenger,string>> q; // passenger, desired class
public:
    void add(const Passenger& p, const string& classType) {
        q.push({p, classType});
        cout << "Added to waitlist. Position: " << q.size() << "\n";
    }
    bool empty() const { return q.empty(); }
    pair<Passenger,string> pop() {
        if (q.empty()) return {{}, ""};
        auto front = q.front(); q.pop(); return front;
    }
    void display() const {
        if (q.empty()) {
            cout << "Waitlist is empty\n"; return;
        }
        cout << "\n=== WAITLIST ===\n";
        queue<pair<Passenger,string>> copy = q;
        int pos=1;
        while (!copy.empty()) {
            auto &p = copy.front();
            cout << pos++ << ". " << p.first.name << " (" << p.second << ")\n";
            copy.pop();
        }
    }
};

// ----------------- Globals -----------------
Flight flight;
map<int, unique_ptr<Booking>> bookings; // active bookings
vector<unique_ptr<Booking>> cancelledBookings;
Waitlist waitlist;
int bookingCounter = 1;
bool adminLogged = false;

// ----------------- Forward decl -----------------
void saveBookingsToFile();
void loadBookingsFromFile();

// ----------------- Auth -----------------
bool adminLogin() {
    string u, p;
    cout << "\n=== ADMIN LOGIN ===\nUsername: ";
    cin >> u;
    cout << "Password: ";
    cin >> p;
    if (u == ADMIN_USER && p == ADMIN_PASS) {
        adminLogged = true;
        cout << "Login successful\n";
        return true;
    }
    cout << "Invalid credentials\n";
    return false;
}

// ----------------- UI Helpers -----------------
void showMenu() {
    cout << "\nFLIGHT BOOKING SYSTEM \n";
    flight.displayInfo();
    cout << "--------------------------------------------------\n";
    cout << "1. Book Ticket\n2. Cancel Ticket\n3. Modify Booking\n4. View All Bookings\n5. Search Bookings\n6. View Seat Map\n7. Generate Reports (Admin)\n8. View Waitlist\n9. View Cancelled Bookings (Admin)\n10. " << (adminLogged ? "Logout" : "Admin Login") << "\n11. Save & Exit\n";
    cout << "----------------------------------------------------\n";
    cout << "Choice: ";
}

// ----------------- Core Features -----------------
void showAvailableSeatsByClassAndPref(const string& classType, const string& pref="") {
    bool found=false;
    cout << left << setw(6) << "Seat" << setw(12) << "Class" << setw(10) << "Pref" << setw(10) << "Price" << "Status\n";
    cout << string(50, '-') << "\n";
    for (const auto& s : flight.seats) {
        if (s->type() == classType && !s->booked) {
            if (pref.empty() || s->preference == pref) {
                s->display();
                found = true;
            }
        }
    }
    if (!found) cout << "No available seats matching criteria\n";
}

void bookTicket() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    string name, phone, email, gender;
    int mealChoice, classChoice, prefChoice, luggageKg;
    char wheelChoice;
    cout << "\n=== PASSENGER DETAILS ===\n";
    while (true) {
        cout << "Name: "; getline(cin, name);
        if (!name.empty()) break;
        cout << "Name cannot be empty\n";
    }
    while (true) {
        cout << "Phone: "; getline(cin, phone);
        if (isValidPhone(phone)) break;
        cout << "Invalid phone. Provide at least 10 digits (symbols + - allowed)\n";
    }
    while (true) {
        cout << "Email: "; getline(cin, email);
        if (isValidEmail(email)) break;
        cout << "Invalid email, try again\n";
    }
    cout << "Gender (M/F/O): "; getline(cin, gender);

    cout << "\nMeal preference:\n1. Vegetarian\n2. Non-Veg\n3. Vegan\n4. No Meal\nChoice: ";
    mealChoice = readIntInRange(1,4);
    MealPreference meal = static_cast<MealPreference>(mealChoice-1);

    cout << "Wheelchair assistance? (y/n): ";
    cin >> wheelChoice;
    bool wheelchair = (wheelChoice == 'y' || wheelChoice == 'Y');

    cout << "Luggage weight (kg): ";
    while (!(cin >> luggageKg)) {
        cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Enter a number for luggage: ";
    }
    if (luggageKg > 20) cout << "Note: extra luggage charges apply ($10 per kg over 20kg)\n";

    cout << "\nSelect class:\n1. Economy\n2. Business\n3. First Class\nChoice: ";
    classChoice = readIntInRange(1,3);
    string classType = (classChoice==1 ? "Economy" : classChoice==2 ? "Business" : "First Class");

    cout << "\nSeat preference:\n1. Window\n2. Aisle\n3. Middle\n4. Any\nChoice: ";
    prefChoice = readIntInRange(1,4);
    string preference = "";
    if (prefChoice==1) preference="Window";
    else if (prefChoice==2) preference="Aisle";
    else if (prefChoice==3) preference="Middle";

    flight.displaySeatMap();
    showAvailableSeatsByClassAndPref(classType, preference);

    cout << "\nEnter seat number (e.g. 1A) or W for waitlist: ";
    string seatInput; cin >> seatInput;
    if (seatInput == "W" || seatInput == "w") {
        Passenger p{name, phone, email, gender, meal, wheelchair, luggageKg};
        waitlist.add(p, classType);
        pressEnterToContinue();
        return;
    }

    Seat* chosen = flight.findSeat(seatInput);
    if (!chosen || chosen->booked || chosen->type() != classType) {
        cout << "Seat not available or invalid. Would you like to join waitlist? (y/n): ";
        char c; cin >> c;
        if (c=='y' || c=='Y') {
            Passenger p{name, phone, email, gender, meal, wheelchair, luggageKg};
            waitlist.add(p, classType);
        }
        pressEnterToContinue();
        return;
    }

    double base = chosen->price();
    double luggageCharge = (luggageKg > 20) ? (luggageKg-20)*10 : 0;
    double total = base + luggageCharge;
    cout << fixed << setprecision(2);
    cout << "\nBase Fare: $" << base << "\n";
    if (luggageCharge > 0) cout << "Extra Luggage: $" << luggageCharge << "\n";
    cout << "Total: $" << total << "\n";

    cout << "\nPayment Method:\n1. Credit Card\n2. Debit Card\n3. UPI\n4. Cash\nChoice: ";
    int payChoice = readIntInRange(1,4);
    PaymentMethod pm = static_cast<PaymentMethod>(payChoice-1);
    auto pay = make_unique<Payment>(total, pm);

    // finalize booking
    chosen->booked = true;
    Passenger passenger{name, phone, email, gender, meal, wheelchair, luggageKg};
    auto booking = make_unique<Booking>(bookingCounter++, passenger, chosen, move(pay));
    booking->bookingTime = getCurrentDateTime();

    cout << "\nBooking successful!\n";
    booking->payment->printReceipt();
    booking->printBoardingPass();

    bookings[booking->id] = move(booking);

    pressEnterToContinue();
}

void cancelTicket() {
    cout << "\nEnter Booking ID to cancel: ";
    int id; if (!(cin >> id)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid ID\n"; pressEnterToContinue(); return; }

    auto it = bookings.find(id);
    if (it == bookings.end()) {
        cout << "Booking not found\n"; pressEnterToContinue(); return;
    }
    Booking* b = it->second.get();
    cout << "Are you sure you want to cancel booking #" << id << " ? (y/n): ";
    char c; cin >> c;
    if (!(c=='y' || c=='Y')) { cout << "Cancellation aborted\n"; pressEnterToContinue(); return; }

    // free seat
    if (b->seat) b->seat->booked = false;
    b->cancelled = true;

    double refund = b->totalAmount() * 0.9;
    cout << fixed << setprecision(2);
    cout << "Booking cancelled. Refund amount: $" << refund << " (90%)\n";

    // move to cancelled list
    cancelledBookings.push_back(move(bookings[id]));
    bookings.erase(id);

    // process waitlist: if someone waiting for the same class, notify (auto-assign not implemented)
    if (!waitlist.empty()) {
        auto next = waitlist.pop();
        // if next wants same class as freed seat, we just inform (in real app we'd email/message)
        if (b->seat && next.second == b->seat->type()) {
            cout << "Notified " << next.first.name << " from waitlist for class " << next.second << "\n";
        } else {
            // push back if not matched
            // For simplicity we won't push back: real app would reevaluate
            cout << "Processed one waitlist entry (not auto-assigned)\n";
        }
    }

    pressEnterToContinue();
}

void modifyBooking() {
    cout << "\nEnter Booking ID to modify: ";
    int id; if (!(cin >> id)) { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); cout << "Invalid ID\n"; pressEnterToContinue(); return; }
    auto it = bookings.find(id);
    if (it==bookings.end()) { cout << "Booking not found\n"; pressEnterToContinue(); return; }
    Booking* b = it->second.get();
    b->displayFull();
    cout << "\nModify options:\n1. Change Seat\n2. Update Meal Preference\n3. Cancel Booking\nChoice: ";
    int ch = readIntInRange(1,3);
    if (ch==1) {
        // show seats of same class
        flight.displaySeatMap();
        showAvailableSeatsByClassAndPref(b->seat->type());
        cout << "Enter new seat number: ";
        string s; cin >> s;
        Seat* newSeat = flight.findSeat(s);
        if (!newSeat || newSeat->booked || newSeat->type() != b->seat->type()) {
            cout << "Seat not available\n";
        } else {
            b->seat->booked = false;
            newSeat->booked = true;
            b->seat = newSeat;
            cout << "Seat changed successfully\n";
        }
    } else if (ch==2) {
        cout << "Select new meal:\n1. Vegetarian\n2. Non-Veg\n3. Vegan\n4. No Meal\nChoice: ";
        int mc = readIntInRange(1,4);
        b->passenger.meal = static_cast<MealPreference>(mc-1);
        cout << "Meal preference updated\n";
    } else {
        // cancel via modify
        cout << "Cancelling booking...\n";
        b->seat->booked = false;
        b->cancelled = true;
        cancelledBookings.push_back(move(it->second));
        bookings.erase(it);
        cout << "Booking cancelled\n";
    }
    pressEnterToContinue();
}

void searchBookings() {
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Search by name, phone, or booking ID: ";
    string q; getline(cin, q);
    bool found=false;
    for (auto& kv : bookings) {
        auto &b = kv.second;
        if (to_string(kv.first) == q ||
            b->passenger.name.find(q) != string::npos ||
            b->passenger.phone.find(q) != string::npos) {
            found = true;
            if (adminLogged) b->displayFull();
            else b->displayLimited();
        }
    }
    if (!found) cout << "No matching bookings found\n";
    pressEnterToContinue();
}

void viewBookings() {
    if (bookings.empty()) { cout << "No active bookings\n"; pressEnterToContinue(); return; }
    cout << "View options:\n1. Passenger View (Limited)\n2. Admin View (Full)\nChoice: ";
    int v = readIntInRange(1,2);
    if (v==2 && !adminLogged) {
        if (!adminLogin()) { cout << "Showing limited view\n"; v = 1; }
    }
    for (auto& kv : bookings) {
        if (v==2 && adminLogged) kv.second->displayFull();
        else kv.second->displayLimited();
    }
    pressEnterToContinue();
}

void generateReports() {
    if (!adminLogged) {
        if (!adminLogin()) { pressEnterToContinue(); return; }
    }
    double revEco=0, revBus=0, revFirst=0;
    int cntEco=0, cntBus=0, cntFirst=0;
    int veg=0, nonveg=0, vegan=0, nomeal=0;

    for (auto& kv : bookings) {
        auto& b = kv.second;
        string t = b->seat->type();
        double amt = b->totalAmount();
        if (t=="Economy") { revEco += amt; cntEco++; }
        else if (t=="Business") { revBus += amt; cntBus++; }
        else if (t=="First Class") { revFirst += amt; cntFirst++; }
        switch (b->passenger.meal) {
            case VEGETARIAN: veg++; break;
            case NON_VEG: nonveg++; break;
            case VEGAN: vegan++; break;
            case NO_MEAL: nomeal++; break;
        }
    }

    int totalSeats = (int)flight.seats.size();
    int bookedSeats = cntEco + cntBus + cntFirst;

    cout << "\n========== REPORTS ==========\n";
    cout << "-- Revenue by Class --\n";
    cout << "Economy: " << cntEco << " bookings, $" << revEco << "\n";
    cout << "Business: " << cntBus << " bookings, $" << revBus << "\n";
    cout << "First Class: " << cntFirst << " bookings, $" << revFirst << "\n";
    cout << "Total Revenue: $" << (revEco + revBus + revFirst) << "\n\n";
    cout << "-- Occupancy --\n";
    cout << "Booked: " << bookedSeats << "/" << totalSeats << " (" << fixed << setprecision(2) << (bookedSeats*100.0/totalSeats) << "%)\n\n";
    cout << "-- Meal Preferences --\n";
    cout << "Vegetarian: " << veg << ", Non-Veg: " << nonveg << ", Vegan: " << vegan << ", No Meal: " << nomeal << "\n\n";
    cout << "-- Cancelled Bookings --\n";
    cout << "Total Cancelled: " << cancelledBookings.size() << "\n";

    pressEnterToContinue();
}

void viewCancelledBookings() {
    if (!adminLogged) {
        if (!adminLogin()) { pressEnterToContinue(); return; }
    }
    if (cancelledBookings.empty()) { cout << "No cancelled bookings\n"; pressEnterToContinue(); return; }
    for (const auto& bptr : cancelledBookings) {
        bptr->displayFull();
    }
    pressEnterToContinue();
}

// ----------------- Save / Load -----------------
// Format per booking:
// id|name|phone|email|gender|meal|wheelchair|luggage|seatNo|bookingTime|paymentMethod|paymentAmount
void saveBookingsToFile() {
    ofstream f("bookings.txt");
    if (!f) {
        cout << "Error saving bookings to file\n";
        return;
    }
    f << bookings.size() << "\n";
    for (auto& kv : bookings) {
        Booking* b = kv.second.get();
        f << b->id << "|"
          << b->passenger.name << "|"
          << b->passenger.phone << "|"
          << b->passenger.email << "|"
          << b->passenger.gender << "|"
          << static_cast<int>(b->passenger.meal) << "|"
          << (b->passenger.wheelchair ? "1" : "0") << "|"
          << b->passenger.luggageKg << "|"
          << (b->seat ? b->seat->seatNo : "NONE") << "|"
          << b->bookingTime << "|"
          << (b->payment ? static_cast<int>(b->payment->method) : -1) << "|"
          << (b->payment ? b->payment->amount : 0.0)
          << "\n";
    }
    f.close();
    cout << "Bookings saved to bookings.txt\n";
}

void loadBookingsFromFile() {
    ifstream f("bookings.txt");
    if (!f) {
        // no file yet, not an error
        return;
    }
    int count = 0;
    if (!(f >> count)) return;
    string line; getline(f, line); // consume rest of first line
    for (int i=0;i<count;i++) {
        if (!getline(f, line)) break;
        if (line.empty()) continue;
        vector<string> parts;
        string token;
        stringstream ss(line);
        while (getline(ss, token, '|')) parts.push_back(token);
        if (parts.size() < 12) continue;
        int id = stoi(parts[0]);
        string name = parts[1];
        string phone = parts[2];
        string email = parts[3];
        string gender = parts[4];
        MealPreference meal = static_cast<MealPreference>(stoi(parts[5]));
        bool wheelchair = (parts[6] == "1");
        int luggage = stoi(parts[7]);
        string seatNo = parts[8];
        string bookingTime = parts[9];
        int payMethod = stoi(parts[10]);
        double payAmt = stod(parts[11]);

        Seat* seatPtr = flight.findSeat(seatNo);
        if (!seatPtr) {
            // seat not found - skip
            continue;
        }
        seatPtr->booked = true;
        Passenger p{name, phone, email, gender, meal, wheelchair, luggage};
        auto pay = make_unique<Payment>(payAmt, static_cast<PaymentMethod>(max(0, payMethod)));
        auto booking = make_unique<Booking>(id, p, seatPtr, move(pay));
        booking->bookingTime = bookingTime;
        bookings[id] = move(booking);
        if (id >= bookingCounter) bookingCounter = id + 1;
    }
    f.close();
    cout << "Previous bookings loaded (" << bookings.size() << ")\n";
}

// ----------------- Main -----------------
int main() {
    srand((unsigned)time(nullptr));
    // Initialize flight details & seats
    flight = Flight("AI101", "New York", "Los Angeles", "10:00 AM", "1:30 PM");
    flight.initDefaultSeats();

    // load previous bookings
    loadBookingsFromFile();

    while (true) {
        clearScreen();
        showMenu();
        int choice;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid choice\n";
            pressEnterToContinue();
            continue;
        }

        switch (choice) {
            case 1: bookTicket(); break;
            case 2: cancelTicket(); break;
            case 3: modifyBooking(); break;
            case 4: viewBookings(); break;
            case 5: searchBookings(); break;
            case 6: flight.displaySeatMap(); pressEnterToContinue(); break;
            case 7: generateReports(); break;
            case 8: waitlist.display(); pressEnterToContinue(); break;
            case 9: viewCancelledBookings(); break;
            case 10:
                if (adminLogged) { adminLogged = false; cout << "Logged out\n"; pressEnterToContinue(); }
                else { adminLogin(); pressEnterToContinue(); }
                break;
            case 11:
                saveBookingsToFile();
                cout << "Exiting... thank you\n";
                // cleanup handled by RAII
                return 0;
            default:
                cout << "Invalid choice\n"; pressEnterToContinue();
        }
    }

    return 0;
}