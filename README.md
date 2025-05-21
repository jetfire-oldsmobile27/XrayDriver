# Драйвер рентгеновской трубки (обновленная документация)

Управление рентгеновской трубкой через REST API с поддержкой COM-порта и расширенными функциями

## 📦 Зависимости
- Conan 1.60+
- CMake 3.12+
- Компилятор с поддержкой C++17
- OpenCV 4.5+
- Boost 1.75+
- SQLite3
- spdlog

## 🛠 Установка
1. Установите зависимости через Conan:
```bash
mkdir build && cd build
conan install .. --build=missing -s compiler.cppstd=17
```

2. Соберите проект:
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DOPENCV_DIR=/path/to/opencv
cmake --build . --config Release
```

## 🚀 Запуск
**Основные режимы:**
```bash
sudo ./build/xray_driver [--daemon] [--config /path/to/config.json]
```

**Параметры:**
- `--daemon` - запуск в фоновом режиме
- `--config` - путь к конфигурационному файлу (по умолчанию: ./config.json)

## 🌐 REST API v2

### Управление оборудованием
```bash
# Установка напряжения
curl -X POST http://localhost:8080/api/voltage \
  -H "Content-Type: application/json" \
  -d '{"voltage": 150}'

# Регулировка мощности
curl -X POST http://localhost:8080/api/power \
  -H "Content-Type: application/json" \
  -d '{"power": 2.5}'

# Запуск экспозиции
curl -X POST http://localhost:8080/api/exposure \
  -H "Content-Type: application/json" \
  -d '{"duration_ms": 500, "auto_power": true}'
```

### Работа с настройками
```bash
# Получение конфигурации
curl http://localhost:8080/api/config/voltage_limit

# Обновление параметров
curl -X POST http://localhost:8080/api/config \
  -H "Content-Type: application/json" \
  -d '{"key": "exposure_mode", "value": "pulsed"}'
```

### Мониторинг системы
```bash
# Полный статус системы
curl http://localhost:8080/api/status

# История экспозиций
curl http://localhost:8080/api/history?limit=10
```

## 🛠 Конфигурация
Пример `config.json`:
```json
{
  "com_port": "/dev/ttyXR0",
  "baud_rate": 38400,
  "http_port": 8080,
  "max_voltage": 200,
  "model_path": "./models/thickness_model.pb",
  "db_path": "/var/db/xray_system.db"
}
```

## 🖥 Архитектура системы
Основные компоненты:
- **XRayTubeController** - единая точка управления оборудованием (Singleton)
- **Protocol Strategies** - реализация различных протоколов связи
- **Power Regulator** - адаптивная регулировка мощности на основе анализа изображений
- **Event Notifier** - система событийной модели
- **SQLite DB** - хранение истории операций и настроек

## 🔌 Поддержка оборудования
Интеграция с системами:
- Рентгеновские трубки серии XT-2000/XTF-3000
- Цифровые детекторы DX-Series
- Системы безопасности RADGuard Pro

## 📊 Анализ изображений
Система автоматически регулирует мощность на основе оценки толщины объекта:
```c++
PowerRegulator regulator(model);
float new_power = regulator.adjust_power(image, current_power);
```

Поддерживаемые форматы:
- DICOM
- RAW-16
- PNG с метаданными

## 🛠 Техническая поддержка
**Важные команды:**
```bash
# Проверка блокировок порта
sudo fuser /dev/ttyXR0

# Экспорт логов
journalctl -u xray_driver -n 100

# Экстренная остановка
sudo ./xray_driver --safe-stop
```

**Логирование:**
- Основные логи: `/var/log/jetservice.log`
- Аварийные дампы: `/var/crash/xray_driver/`
- Аудит операций: `/var/db/xray_system.db`

## 📚 Дополнительные материалы
- [Интеграция с DICOM-системами](docs/DICOM_INTEGRATION.md)
- [Протокол обмена данными](docs/PROTOCOL_SPEC.md)
- [Тестирование безопасности](docs/SECURITY_CHECKS.md)

## ⚠️ Безопасность
Все соединения защищены механизмом двойной аутентификации. Для доступа к API требуется токен:
```bash
curl -H "Authorization: Bearer YOUR_TOKEN" http://localhost:8080/api/status
```