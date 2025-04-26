from bs4 import BeautifulSoup
import requests
import json
from dataclasses import dataclass
import gspread
from oauth2client.service_account import ServiceAccountCredentials
from pprint import pprint as pp


@dataclass
class Flight:
    id: str = None
    departure_city: str = None
    arrival_city: str = None
    # days: str = None
    days: list[int] = None
    departure_time: str = None
    arrival_time: str = None
    duration: str = None
    distance: int = None


@dataclass
class FlightJson:
    id: str = None
    departure_city: int = None
    arrival_city: int = None
    departure_time: int = None  # in minutes from start
    arrival_time: int = None
    duration: int = None
    distance: int = None


DAYS_TO_NUMBERS = {
    'пн': 0,
    'вт': 1,
    'ср': 2,
    'чт': 3,
    'пт': 4,
    'сб': 5,
    'вс': 6,
}


def parse_belavia() -> list[Flight]:
    url = 'https://aviatickets.by/airlines/belavia-belarusian-airlines-bru/?ysclid=m8byhd8tpz301359908'
    page = requests.get(url)
    soup = BeautifulSoup(page.text, "html.parser")

    flights_html = soup.findAll('tr')[1:]
    flights: list[Flight] = []
    for flight_html in flights_html:
        flight_data = flight_html.findAll('td')
        flights.append(Flight())
        flight = flights[-1]
        flight.id = flight_data[0].text.strip()
        from_to = flight_data[1].text.split(' — ')
        flight.departure_city = from_to[0].strip()
        flight.arrival_city = from_to[1].strip()
        flight.days = flight_data[2].text.strip()
        # days = flight_data[2].text.split(', ')
        # if days[0] == 'ежедневно':
        #     flight.days = [0, 1, 2, 3, 4, 5, 6]
        # else:
        #     flight.days = [DAYS_TO_NUMBERS[day] for day in days]
        flight.departure_time = flight_data[3].text.strip()
        flight.arrival_time = flight_data[4].text.strip()
        flight.duration = flight_data[5].text.strip()

    # for flight in flights:
    #     print(flight_to_array(flight))
    return flights


def parse_belavia_from_file(filename: str):
    flights: list[Flight] = []
    with open(filename, mode='r') as file:
        for line in file:
            flight_data = line.split('; ')
            flights.append(Flight())
            flight = flights[-1]
            flight.id = flight_data[0]
            from_to = flight_data[1].split(' - ')
            flight.departure_city = from_to[0]
            flight.arrival_city = from_to[1]
            flight.days = flight_data[2]
            # days = flight_data[2].text.split(', ')
            # if days[0] == 'ежедневно':
            #     flight.days = [0, 1, 2, 3, 4, 5, 6]
            # else:
            #     flight.days = [DAYS_TO_NUMBERS[day] for day in days]
            flight.departure_time = flight_data[3]
            flight.arrival_time = flight_data[4]
            flight.duration = flight_data[5]

        # for flight in flights:
        #     print(flight_to_array(flight))
        return flights


def flight_to_array(flight: Flight) -> list[str]:
    return [
        flight.id,
        flight.departure_city,
        flight.arrival_city,
        flight.departure_time,
        flight.arrival_time,
        flight.duration,
        flight.days,
    ]


def write_to_google_sheets(flights: list[Flight]):
    scope = ["https://spreadsheets.google.com/feeds", 'https://www.googleapis.com/auth/spreadsheets',
             "https://www.googleapis.com/auth/drive.file", "https://www.googleapis.com/auth/drive"]
    creds = ServiceAccountCredentials.from_json_keyfile_name("creds.json", scope)
    client = gspread.authorize(creds)
    sheet = client.open("Расписание белавиа").sheet1
    for index in range(len(flights)):
        sheet.insert_row(flight_to_array(flights[index]), index + 104)


def write_cities(cities: list[str]):
    scope = ["https://spreadsheets.google.com/feeds", 'https://www.googleapis.com/auth/spreadsheets',
             "https://www.googleapis.com/auth/drive.file", "https://www.googleapis.com/auth/drive"]
    creds = ServiceAccountCredentials.from_json_keyfile_name("creds.json", scope)
    client = gspread.authorize(creds)
    sheet = client.open("Расписание белавиа").sheet1
    cities.sort()
    cities.append(cities[0])
    cities[0] = 'Город'
    print(cities)
    sheet.insert_row(cities, 141)
    # column = 12
    # for index in range(len(cities)):
    #     sheet.update_cell(index + 1, column, cities[index])


def get_cities_ids(cities: dict[str, float]) -> dict[str, int]:
    result = {}
    id = 0
    for city in cities.keys():
        result[city] = id
        id += 1
    return result


def get_duration(time: str) -> int:
    hours = int(time[:time.find('ч')]) if time.find('ч') != -1 else 0
    minutes_start = 0 if hours == 0 else time.find('ч') + 2
    minutes = int(time[minutes_start : time.find('м')])
    return hours * 60 + minutes


def get_flights_json(flights: list[Flight], cities: dict[str, float], cities_ids, main_time_zone = 8):
    result = []
    for flight in flights:
        departure_time_zone = cities[flight.departure_city]
        diff_dep = main_time_zone - departure_time_zone
        time_dep = flight.departure_time.split(':')
        hours_dep = int(time_dep[0])
        minutes_dep = int(time_dep[1])

        if float(int(diff_dep)) != diff_dep:  # Deli
            minutes_dep += 30
            diff_dep -= 0.5
        hours_dep += int(diff_dep)

        arrival_time_zone = cities[flight.arrival_city]
        diff_arr = main_time_zone - arrival_time_zone
        time_arr = flight.arrival_time.split(':')
        hours_arr = int(time_arr[0])
        minutes_arr = int(time_arr[1])

        if float(int(diff_arr)) != diff_arr:  # Deli
            minutes_arr += 30
            diff_arr -= 0.5
        hours_arr += int(diff_arr)

        for day_dep in flight.days:
            day_arr = day_dep if hours_dep <= hours_arr else day_dep + 1
            result.append(FlightJson())
            flight_json = result[-1]
            flight_json.id = flight.id
            flight_json.departure_city = cities_ids[flight.departure_city]
            flight_json.arrival_city = cities_ids[flight.arrival_city]
            flight_json.distance = flight.distance
            flight_json.departure_time = day_dep * 24 * 60 + hours_dep * 60 + minutes_dep
            flight_json.arrival_time = day_arr * 24 * 60 + hours_arr * 60 + minutes_arr
            flight_json.duration = flight_json.arrival_time - flight_json.departure_time
            assert flight_json.duration == get_duration(flight.duration), f'{flight.id}: {flight_json.duration} == {get_duration(flight.duration)}'

    return result


def read_flights_from_table() -> list[Flight]:
    scope = ["https://spreadsheets.google.com/feeds", 'https://www.googleapis.com/auth/spreadsheets',
             "https://www.googleapis.com/auth/drive.file", "https://www.googleapis.com/auth/drive"]
    creds = ServiceAccountCredentials.from_json_keyfile_name("creds.json", scope)
    client = gspread.authorize(creds)
    sheet = client.open("Расписание белавиа").sheet1

    flights = []
    data = sheet.get_all_records()
    for flight_dict in data:
        flights.append(Flight())
        flight = flights[-1]
        flight.id = flight_dict['id']
        flight.departure_city = flight_dict['Departure city']
        flight.arrival_city = flight_dict['Arrival city']
        flight.distance = flight_dict['Distance (km)']
        flight.departure_time = flight_dict['Departure time']
        flight.arrival_time = flight_dict['Arrival time']
        flight.duration = flight_dict['Flight duration']
        days = flight_dict['Days of flight'].split(', ')
        if days[0] == 'ежедневно':
            flight.days = [0, 1, 2, 3, 4, 5, 6]
        else:
            flight.days = [DAYS_TO_NUMBERS[day] for day in days]
    return flights


def get_cities() -> dict[str, float]:
    scope = ["https://spreadsheets.google.com/feeds", 'https://www.googleapis.com/auth/spreadsheets',
             "https://www.googleapis.com/auth/drive.file", "https://www.googleapis.com/auth/drive"]
    creds = ServiceAccountCredentials.from_json_keyfile_name("creds.json", scope)
    client = gspread.authorize(creds)
    sheet = client.open("Расписание белавиа").get_worksheet(1)

    cities_names = sheet.col_values(1)
    time_zones = sheet.col_values(2)
    cities = {}
    for row in range(1, len(cities_names)):
        time_zone = float(time_zones[row]) if time_zones[row] != '5,5' else 5.5
        cities[cities_names[row]] = time_zone
    return cities


def get_aircrafts() -> list[dict]:
    id = 0
    aircrafts = []
    for i in range(5):  # Boeing 737-800
        aircrafts.append({
            'id': id,
            'seats': 158,
            'flight_cost': 15,
        })
        id += 1
    for i in range(6):  # Boeing 737-500
        aircrafts.append({
            'id': id,
            'seats': 132,
            'flight_cost': 15,
        })
        id += 1
    for i in range(7):  # Boeing 737-300
        aircrafts.append({
            'id': id,
            'seats': 149,
            'flight_cost': 15,
        })
        id += 1
    for i in range(4):  # Embraer 195
        aircrafts.append({
            'id': id,
            'seats': 116,
            'flight_cost': 12,
        })
        id += 1
    for i in range(3):  # Embraer 175
        aircrafts.append({
            'id': id,
            'seats': 88,
            'flight_cost': 9,
        })
        id += 1
    for i in range(4):  # CRJ-100/200 LR
        aircrafts.append({
            'id': id,
            'seats': 50,
            'flight_cost': 5,
        })
        id += 1
    return aircrafts


def write_flights_json(flights: list[FlightJson], airports_number):
    with open('timetable.json', 'w') as file:
        flights_list = []
        id = 0
        for flight in flights:
            flights_list.append({
                "id": id,
                "departure airport": flight.departure_city,
                "arrival airport": flight.arrival_city,
                "departure time": flight.departure_time - 60,
                "arrival time": flight.arrival_time + 60,
                "distance": flight.distance,
                "min passengers": 1,
            })
            id += 1
        airports = [{'id': airport_id, 'stay_cost': 1} for airport_id in range(airports_number)]
        json.dump({'flights': flights_list, 'aircrafts': get_aircrafts(), 'airports': airports}, file, indent=4)


def write_input_file(flights: list[FlightJson],
                     aircrafts: list[dict],
                     airports: dict):
    max_time = 1000000000
    for flight in flights:
        max_time = max(max_time, flight.arrival_time)

    result = {
        'flights number': len(flights),
        'aircrafts number': len(aircrafts),
        'airports number': len(airports),
        'hours in cycle': max_time + 60,
        'hour size': 60,
    }
    with open('input.json', 'w') as file:
        json.dump(result, file, indent=4)


if __name__ == '__main__':
    # flights = parse_belavia_from_file('file.txt')

    # print(len(flights))
    # write_to_google_sheets(flights[102:])

    cities = get_cities()
    # print(cities)
    cities_ids = get_cities_ids(cities)
    # print(cities_ids)
    flights = read_flights_from_table()
    flights_json = get_flights_json(flights, cities, cities_ids)
    write_flights_json(flights_json, len(cities))
    write_input_file(flights_json, get_aircrafts(), cities)
