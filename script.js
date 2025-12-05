document.addEventListener('DOMContentLoaded', () => {
    fetchFlightInfo();
    fetchBookings();
});

let selectedSeat = null;
let flightData = null;

async function fetchFlightInfo() {
    const res = await fetch('/api/flight-info');
    const data = await res.json();
    flightData = data;
    
    renderFlightInfo(data.info);
    renderStats(data.stats);
    renderSeatMap(data.seats);
}

async function fetchBookings() {
    const res = await fetch('/api/bookings');
    const bookings = await res.json();
    renderBookings(bookings);
}

function renderFlightInfo(info) {
    document.getElementById('flight-route').innerHTML = `
        <span>${info.flightNo}</span>
        <span>•</span>
        <span>${info.src} ➝ ${info.dest}</span>
        <span>•</span>
        <span>${info.depTime}</span>
    `;
}

function renderStats(stats) {
    document.getElementById('stat-rev').textContent = `$${stats.revenue.toLocaleString()}`;
    document.getElementById('stat-occ').textContent = `${Math.round((stats.occupancy / stats.total_seats) * 100)}%`;
}

function renderSeatMap(seats) {
    const container = document.getElementById('seat-map');
    container.innerHTML = '';
    
    // Group by row
    const rows = {};
    seats.forEach(seat => {
        const rowNum = seat.id.slice(0, -1);
        if (!rows[rowNum]) rows[rowNum] = [];
        rows[rowNum].push(seat);
    });
    
    Object.keys(rows).sort((a,b) => parseInt(a)-parseInt(b)).forEach(rowNum => {
        const rowDiv = document.createElement('div');
        rowDiv.className = 'seat-row';
        
        const rowSeats = rows[rowNum];
        
        // Layout: A B | aisle | C D (for Eco) or A | B C (Bus) etc.
        // Simplified layout logic based on known structure
        
        rowSeats.forEach(seat => {
            const seatEl = document.createElement('div');
            seatEl.className = `seat ${seat.class.toLowerCase().replace(' ', '-')} ${seat.booked ? 'booked' : ''}`;
            seatEl.textContent = seat.id;
            
            if (!seat.booked) {
                seatEl.onclick = () => selectSeat(seat, seatEl);
            }
            
            rowDiv.appendChild(seatEl);
            
            // Add aisle spacer
            if (seat.id.endsWith('B') && rowSeats.length > 2) {
                const aisle = document.createElement('div');
                aisle.className = 'aisle';
                aisle.textContent = rowNum;
                rowDiv.appendChild(aisle);
            }
        });
        
        container.appendChild(rowDiv);
    });
}

function selectSeat(seat, el) {
    // Deselect previous
    document.querySelectorAll('.seat.selected').forEach(e => e.classList.remove('selected'));
    
    selectedSeat = seat;
    el.classList.add('selected');
    
    document.getElementById('selected-seat-display').textContent = `${seat.id} (${seat.class})`;
    document.getElementById('base-price').textContent = `$${seat.price}`;
    updateTotal();
}

function updateTotal() {
    if (!selectedSeat) return;
    let total = selectedSeat.price;
    const luggage = parseInt(document.getElementById('luggage').value) || 0;
    if (luggage > 20) {
        total += (luggage - 20) * 10;
    }
    document.getElementById('total-price').textContent = `$${total}`;
}

document.getElementById('luggage').addEventListener('input', updateTotal);

document.getElementById('booking-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    if (!selectedSeat) {
        alert('Please select a seat first');
        return;
    }
    
    const formData = {
        name: document.getElementById('name').value,
        phone: document.getElementById('phone').value,
        email: document.getElementById('email').value,
        gender: document.getElementById('gender').value,
        meal: document.getElementById('meal').value,
        wheelchair: document.getElementById('wheelchair').checked,
        luggage: document.getElementById('luggage').value,
        paymentMethod: document.getElementById('payment').value,
        seatNo: selectedSeat.id
    };
    
    try {
        const res = await fetch('/api/book', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(formData)
        });
        
        const result = await res.json();
        if (result.success) {
            showSuccess(result.booking);
            fetchFlightInfo(); // Refresh map
            fetchBookings(); // Refresh list
            document.getElementById('booking-form').reset();
            selectedSeat = null;
            document.getElementById('selected-seat-display').textContent = '-';
            document.getElementById('total-price').textContent = '$0';
        } else {
            alert(result.error);
        }
    } catch (err) {
        alert('Booking failed');
    }
});

function renderBookings(bookings) {
    const list = document.getElementById('booking-list');
    list.innerHTML = '';
    
    bookings.reverse().forEach(b => {
        const item = document.createElement('div');
        item.className = 'booking-item';
        item.innerHTML = `
            <div class="b-info">
                <h4>${b.name} <span style="font-weight:400; color:#94a3b8">(${b.seatNo})</span></h4>
                <div class="b-meta">#${b.id} • ${b.bookingTime}</div>
            </div>
            <div class="b-price">$${b.amount}</div>
        `;
        list.appendChild(item);
    });
}

function showSuccess(booking) {
    const modal = document.getElementById('success-modal');
    document.getElementById('success-msg').textContent = `Booking #${booking.id} Confirmed!`;
    modal.classList.add('active');
}

function closeModal() {
    document.getElementById('success-modal').classList.remove('active');
}
