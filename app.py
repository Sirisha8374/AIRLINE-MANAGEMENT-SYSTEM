import os
import datetime
import random
from flask import Flask, render_template, jsonify, request

app = Flask(__name__)

# Configuration matching C++ app
BOOKINGS_FILE = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'bookings.txt')
FLIGHT_INFO = {
    "flightNo": "AI101",
    "src": "New York",
    "dest": "Los Angeles",
    "depTime": "10:00 AM",
    "arrTime": "1:30 PM"
}

# Seat Configuration
SEATS = []
def init_seats():
    global SEATS
    SEATS = []
    # Economy 1-5: A-D
    for r in range(1, 6):
        SEATS.append({"id": f"{r}A", "class": "Economy", "price": 100.0, "pref": "Window"})
        SEATS.append({"id": f"{r}B", "class": "Economy", "price": 100.0, "pref": "Middle"})
        SEATS.append({"id": f"{r}C", "class": "Economy", "price": 100.0, "pref": "Aisle"})
        SEATS.append({"id": f"{r}D", "class": "Economy", "price": 100.0, "pref": "Window"})
    # Business 6-8: A-C
    for r in range(6, 9):
        SEATS.append({"id": f"{r}A", "class": "Business", "price": 300.0, "pref": "Window"}) # 150 * 2
        SEATS.append({"id": f"{r}B", "class": "Business", "price": 300.0, "pref": "Aisle"})
        SEATS.append({"id": f"{r}C", "class": "Business", "price": 300.0, "pref": "Window"})
    # First Class 9-10: A-B
    for r in range(9, 11):
        SEATS.append({"id": f"{r}A", "class": "First Class", "price": 600.0, "pref": "Window"}) # 200 * 3
        SEATS.append({"id": f"{r}B", "class": "First Class", "price": 600.0, "pref": "Aisle"})

init_seats()

def read_bookings():
    bookings = []
    if not os.path.exists(BOOKINGS_FILE):
        return bookings
    
    try:
        with open(BOOKINGS_FILE, 'r') as f:
            lines = f.readlines()
            if not lines: return bookings
            try:
                count = int(lines[0].strip())
            except ValueError:
                return bookings
                
            for line in lines[1:]:
                parts = line.strip().split('|')
                if len(parts) < 12: continue
                
                # Parse based on C++ format:
                # id|name|phone|email|gender|meal|wheelchair|luggage|seatNo|bookingTime|paymentMethod|paymentAmount
                booking = {
                    "id": int(parts[0]),
                    "name": parts[1],
                    "phone": parts[2],
                    "email": parts[3],
                    "gender": parts[4],
                    "meal": int(parts[5]),
                    "wheelchair": parts[6] == "1",
                    "luggage": int(parts[7]),
                    "seatNo": parts[8],
                    "bookingTime": parts[9],
                    "paymentMethod": int(parts[10]),
                    "amount": float(parts[11])
                }
                bookings.append(booking)
    except Exception as e:
        print(f"Error reading bookings: {e}")
        
    return bookings

def write_booking(booking):
    bookings = read_bookings()
    bookings.append(booking)
    
    with open(BOOKINGS_FILE, 'w') as f:
        f.write(f"{len(bookings)}\n")
        for b in bookings:
            line = f"{b['id']}|{b['name']}|{b['phone']}|{b['email']}|{b['gender']}|{b['meal']}|"
            line += f"{'1' if b['wheelchair'] else '0'}|{b['luggage']}|{b['seatNo']}|{b['bookingTime']}|"
            line += f"{b['paymentMethod']}|{b['amount']}\n"
            f.write(line)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/flight-info')
def get_flight_info():
    bookings = read_bookings()
    booked_seats = [b['seatNo'] for b in bookings if b['seatNo'] != "NONE"]
    
    # Calculate stats
    revenue = sum(b['amount'] for b in bookings)
    occupancy = len(booked_seats)
    
    seats_with_status = []
    for seat in SEATS:
        s = seat.copy()
        s['booked'] = s['id'] in booked_seats
        seats_with_status.append(s)
        
    return jsonify({
        "info": FLIGHT_INFO,
        "seats": seats_with_status,
        "stats": {
            "revenue": revenue,
            "occupancy": occupancy,
            "total_seats": len(SEATS)
        }
    })

@app.route('/api/bookings')
def get_bookings():
    return jsonify(read_bookings())

@app.route('/api/book', methods=['POST'])
def create_booking():
    data = request.json
    bookings = read_bookings()
    
    # Generate ID
    new_id = 1
    if bookings:
        new_id = max(b['id'] for b in bookings) + 1
        
    # Validate seat
    seat_id = data.get('seatNo')
    if any(b['seatNo'] == seat_id for b in bookings):
        return jsonify({"error": "Seat already booked"}), 400
        
    # Calculate price (backend validation)
    seat = next((s for s in SEATS if s['id'] == seat_id), None)
    if not seat:
        return jsonify({"error": "Invalid seat"}), 400
        
    luggage = int(data.get('luggage', 0))
    price = seat['price']
    if luggage > 20:
        price += (luggage - 20) * 10
        
    new_booking = {
        "id": new_id,
        "name": data['name'],
        "phone": data['phone'],
        "email": data['email'],
        "gender": data['gender'],
        "meal": int(data['meal']),
        "wheelchair": bool(data.get('wheelchair')),
        "luggage": luggage,
        "seatNo": seat_id,
        "bookingTime": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "paymentMethod": int(data['paymentMethod']),
        "amount": price
    }
    
    write_booking(new_booking)
    return jsonify({"success": True, "booking": new_booking})

if __name__ == '__main__':
    app.run(debug=True, port=5000)
