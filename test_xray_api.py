import requests
import time
import json

# Конфигурация
BASE_URL = "http://localhost:8080/api"
TEST_PORT = "COM3"  # Порт микроконтроллера для прямого тестирования

def test_rest_api():
    """Тестирование REST API драйвера"""
    print("=== Testing X-Ray Driver REST API ===")
    
    # 1. Установка параметров
    print("\n1. Setting tube parameters:")
    response = requests.post(
        f"{BASE_URL}/configure",
        json={"voltage": 120, "current": 0.25}
    )
    print_response(response)
    
    # 2. Запуск экспозиции
    print("\n2. Starting exposure:")
    response = requests.post(
        f"{BASE_URL}/exposure/now",
        json={"duration": 2000, "mode": "test"}
    )
    print_response(response)
    
    # 3. Мониторинг статуса
    print("\n3. Monitoring status:")
    for _ in range(5):
        response = requests.get(f"{BASE_URL}/status")
        status = response.json()
        print(f"Voltage: {status['voltage_kv']}kV, Current: {status['current_ma']}mA")
        print(f"Exposure: {'Active' if status['exposure_active'] else 'Inactive'}")
        time.sleep(0.5)
    
    # 4. Аварийная остановка
    print("\n4. Emergency stop:")
    response = requests.post(f"{BASE_URL}/emergency_stop")
    print_response(response)
    
    # 5. Получение логов
    print("\n5. Getting logs:")
    response = requests.get(f"{BASE_URL}/logs/system")
    print(json.dumps(response.json(), indent=2))
    
    print("\n=== REST API Test Completed ===")

def test_direct_comms(port=TEST_PORT):
    """Прямое тестирование связи с микроконтроллером"""
    import serial
    print(f"\n=== Testing Direct Communication ({port}) ===")
    
    try:
        ser = serial.Serial(port, 38400, timeout=1)
        print("Port opened successfully")
        
        # Тестовая последовательность команд
        commands = [
            "GET_STATUS",
            "SET_VOLTAGE 80",
            "SET_CURRENT 0.15",
            "GET_STATUS",
            "START_EXPOSURE 1000",
            "GET_STATUS",
            "STOP_EXPOSURE",
            "EMERGENCY_STOP"
        ]
        
        for cmd in commands:
            print(f"\n>>> {cmd}")
            ser.write(f"{cmd}\n".encode())
            time.sleep(0.5)
            
            # Чтение всех доступных ответов
            while ser.in_waiting:
                response = ser.readline().decode().strip()
                print(f"<<< {response}")
    
    except Exception as e:
        print(f"Error: {str(e)}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
        print("=== Direct Communication Test Completed ===")

def print_response(response):
    """Печать ответа сервера"""
    print(f"Status: {response.status_code}")
    try:
        print(json.dumps(response.json(), indent=2))
    except:
        print(response.text)

if __name__ == "__main__":
    # Тестирование REST API драйвера
    test_rest_api()
    
    # Прямое тестирование микроконтроллера (требует подключения)
    # test_direct_comms()