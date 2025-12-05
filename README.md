# Airline Management System

A comprehensive Airline Management System featuring a robust C++ console application and a modern Web User Interface.

## 🌟 Features

### Core System (C++)
- **Booking Management**: Book, modify, and cancel tickets.
- **Seat Management**: View seat maps for Economy, Business, and First Class.
- **Passenger Details**: Manage passenger info, meal preferences, and luggage.
- **Admin Panel**: Generate reports, view all bookings, and manage the system.
- **Data Persistence**: All data is saved to `bookings.txt`.

### Web Interface (Python/Flask)
- **Visual Seat Map**: Interactive seat selection with real-time availability.
- **Modern Dashboard**: Track revenue, occupancy, and recent bookings.
- **Responsive Design**: Beautiful glassmorphism UI that works on all devices.
- **Real-time Sync**: Seamlessly integrates with the C++ backend data.

## 🛠️ Tech Stack
- **Backend**: C++ (Core Logic), Python Flask (Web Server)
- **Frontend**: HTML5, CSS3, JavaScript
- **Data Storage**: Text-based file system (`bookings.txt`)

## 🚀 Getting Started

### Prerequisites
- C++ Compiler (GCC/MinGW)
- Python 3.x
- Flask (`pip install flask`)

### Running the C++ Application
1. Compile the code:
   ```bash
   g++ AMS4.cpp -o AMS
   ```
2. Run the executable:
   ```bash
   ./AMS
   ```

### Running the Web UI
1. Install dependencies:
   ```bash
   pip install flask
   ```
2. Start the server:
   ```bash
   python web_ui/app.py
   ```
3. Open your browser and visit: `http://127.0.0.1:5000`

## 📂 Project Structure
```
├── AMS4.cpp            # Main C++ Source Code
├── bookings.txt        # Data Storage
├── web_ui/
│   ├── app.py          # Flask Backend
│   ├── static/         # CSS & JS
│   └── templates/      # HTML Templates
└── README.md           # Documentation
```
