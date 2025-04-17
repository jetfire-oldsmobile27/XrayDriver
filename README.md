# Драйвер рентгеновской трубки

Управление рентгеновской трубкой через REST API с поддержкой COM-порта.

## 📦 Зависимости
- Conan 1.60+
- CMake 3.12+
- Компилятор с поддержкой C++17

## 🛠 Установка
1. Установите зависимости через Conan:
```bash
mkdir build && cd build
conan install .. --build=missing
```

2. Соберите проект:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## 🚀 Запуск
**Обычный режим:**
```bash
sudo ./build/xray_driver
```

**Режим демона:**
```bash
sudo ./build/xray_driver --daemon
```

## 🌐 REST API
Базовые эндпоинты:

### Установка напряжения
```bash
curl -X POST http://localhost:8080/api/voltage \
  -H "Content-Type: application/json" \
  -d '{"voltage": 150}'
```
- `voltage`: Значение в кВ (50-200)

### Старт экспозиции
```bash
curl -X POST http://localhost:8080/api/exposure \
  -H "Content-Type: application/json" \
  -d '{"duration_ms": 500}'
```
- `duration_ms`: Время в миллисекундах (100-5000)

### Проверка статуса
```bash
curl http://localhost:8080/api/status
```

## 🔧 Конфигурация
По умолчанию используется:
- Порт: `/dev/ttyXR0`
- Скорость: 38400 бод
- HTTP-порт: 8080

Для изменения параметров отредактируйте `config.json`.

## 🆘 Поддержка
При проблемах:
1. Проверьте права на COM-порт:
```bash
sudo chmod 666 /dev/ttyXR0
```

2. Убедитесь что порт свободен:
```bash
lsof /dev/ttyXR0
```

3. Логи находятся в `/var/log/xray_driver.log`