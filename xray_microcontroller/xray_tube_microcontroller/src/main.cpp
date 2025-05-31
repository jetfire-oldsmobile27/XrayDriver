#include <Arduino.h>

// Определение пинов
const uint8_t HV_RELAY_PIN = PB14;    // Реле высокого напряжения
const uint8_t FILAMENT_RELAY_PIN = PB12;  // Реле накала
const uint8_t VOLTMETER_PWM_PIN = PA0;   // ШИМ для вольтметра
const uint8_t BACKLIGHT_PIN = PA7;       // Управление подсветкой
const uint8_t FAULT_LED_PIN = PC13;      // Индикатор ошибки

// Параметры системы
uint16_t target_voltage_kV = 0;      // Целевое напряжение (кВ)
uint16_t current_voltage_kV = 0;     // Текущее напряжение (кВ)
float target_current_mA = 0.0f;      // Целевой ток (мА)
float measured_current_mA = 0.0f;    // Измеренный ток (мА)
bool exposure_active = false;         // Флаг активности экспозиции
uint32_t exposure_end_time = 0;       // Время окончания экспозиции
bool filament_on = false;             // Состояние накала
bool fault_condition = false;         // Флаг аварийного состояния

void set_voltage(uint16_t kV);
void set_current(float mA);
void start_exposure(uint32_t duration_ms);
void stop_exposure();
void emergency_stop();
void update_voltmeter();
void send_status();
void check_current();

void setup() {
  Serial2.begin(38400); 
  
  // Настройка пинов
  pinMode(HV_RELAY_PIN, OUTPUT);
  pinMode(FILAMENT_RELAY_PIN, OUTPUT);
  pinMode(VOLTMETER_PWM_PIN, OUTPUT);
  pinMode(BACKLIGHT_PIN, OUTPUT);
  pinMode(FAULT_LED_PIN, OUTPUT);
  
  // Инициализация ШИМ для вольтметра
  analogWriteResolution(12);  // 12-битное разрешение
  analogWriteFrequency(1000); // Частота 1 кГц
  
  // Начальное состояние
  digitalWrite(BACKLIGHT_PIN, HIGH);  // Включить подсветку
  digitalWrite(FAULT_LED_PIN, LOW);   // Выключить индикатор ошибки
  set_voltage(0);
  set_current(0.0f);
  
  Serial2.println("SYSTEM_READY");
}

void loop() {
  // Обработка входящих команд
  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    command.trim();
    
    if (command.startsWith("SET_VOLTAGE")) {
      uint16_t kV = command.substring(11).toInt();
      set_voltage(kV);
    }
    else if (command.startsWith("SET_CURRENT")) {
      float mA = command.substring(11).toFloat();
      set_current(mA);
    }
    else if (command.startsWith("START_EXPOSURE")) {
      uint32_t duration = command.substring(14).toInt();
      start_exposure(duration);
    }
    else if (command.equals("STOP_EXPOSURE")) {
      stop_exposure();
    }
    else if (command.equals("EMERGENCY_STOP")) {
      emergency_stop();
    }
    else if (command.equals("GET_STATUS")) {
      send_status();
    }
    else {
      Serial2.println("ERROR: Unknown command");
    }
  }
  
  // Проверка окончания экспозиции
  if (exposure_active && millis() >= exposure_end_time) {
    stop_exposure();
  }
  
  // Плавное нарастание напряжения
  if (current_voltage_kV < target_voltage_kV) {
    current_voltage_kV++;
    update_voltmeter();
    delay(10);  // Задержка для плавности
  }
  
  // Плавное снижение напряжения
  if (current_voltage_kV > target_voltage_kV) {
    current_voltage_kV--;
    update_voltmeter();
    delay(10);  // Задержка для плавности
  }
  
  // Регулярная проверка тока
  static uint32_t last_current_check = 0;
  if (millis() - last_current_check > 100) {
    check_current();
    last_current_check = millis();
  }
  
  // Мигание индикатором при аварии
  if (fault_condition) {
    digitalWrite(FAULT_LED_PIN, millis() % 500 < 250);
  }
}

void set_voltage(uint16_t kV) {
  if (kV < 10 || kV > 150) {
    Serial2.println("ERROR: Voltage out of range (10-150 kV)");
    return;
  }
  
  target_voltage_kV = kV;
  Serial2.print("VOLTAGE_SET:");
  Serial2.println(target_voltage_kV);
}

void set_current(float mA) {
  if (mA < 0.01f || mA > 0.4f) {
    Serial2.println("ERROR: Current out of range (0.01-0.4 mA)");
    return;
  }
  
  target_current_mA = mA;
  filament_on = (mA > 0.01f);
  digitalWrite(FILAMENT_RELAY_PIN, filament_on ? HIGH : LOW);
  
  Serial2.print("CURRENT_SET:");
  Serial2.println(target_current_mA, 2);
}

void start_exposure(uint32_t duration_ms) {
  if (fault_condition) {
    Serial2.println("ERROR: System in fault condition");
    return;
  }
  
  if (current_voltage_kV < 10) {
    Serial2.println("ERROR: Voltage too low for exposure");
    return;
  }
  
  exposure_active = true;
  exposure_end_time = millis() + duration_ms;
  digitalWrite(HV_RELAY_PIN, HIGH);
  
  Serial2.print("EXPOSURE_STARTED:");
  Serial2.println(duration_ms);
}

void stop_exposure() {
  exposure_active = false;
  digitalWrite(HV_RELAY_PIN, LOW);
  Serial2.println("EXPOSURE_STOPPED");
}

void emergency_stop() {
  exposure_active = false;
  target_voltage_kV = 0;
  target_current_mA = 0.0f;
  digitalWrite(HV_RELAY_PIN, LOW);
  digitalWrite(FILAMENT_RELAY_PIN, LOW);
  fault_condition = true;
  
  Serial2.println("EMERGENCY_STOP");
}

void update_voltmeter() {
  // Преобразуем кВ в значение ШИМ (0-4095)
  uint32_t pwm_value = map(current_voltage_kV, 0, 150, 0, 4095);
  analogWrite(VOLTMETER_PWM_PIN, pwm_value);
}

void send_status() {
  Serial2.print("STATUS:");
  Serial2.print("V="); Serial2.print(current_voltage_kV);
  Serial2.print(",T="); Serial2.print(target_voltage_kV);
  Serial2.print(",I="); Serial2.print(measured_current_mA, 2);
  Serial2.print(",E="); Serial2.print(exposure_active ? "1" : "0");
  Serial2.print(",F="); Serial2.print(filament_on ? "1" : "0");
  Serial2.print(",A="); Serial2.print(fault_condition ? "1" : "0");
  Serial2.println();
}

void check_current() {
  // Здесь должна быть реальная логика измерения тока
  // Для теста эмулируем измерение
  
  if (!filament_on) {
    measured_current_mA = 0.0f;
    return;
  }
  
  // Простая модель: измеренный ток = целевой ток ± 5%
  float variation = (random(-50, 50) / 1000.0f);
  measured_current_mA = target_current_mA + variation;
  
  // Проверка на перегрузку
  if (measured_current_mA > target_current_mA * 1.15f) {
    Serial2.print("WARNING: Current overload ");
    Serial2.println(measured_current_mA, 2);
    
    if (measured_current_mA > target_current_mA * 1.25f) {
      emergency_stop();
      Serial2.println("FAULT: Current limit exceeded");
    }
  }
}