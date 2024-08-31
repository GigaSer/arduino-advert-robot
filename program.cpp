// Константы для направления
constexpr uint8_t DIRECTION_LEFT_PIN = 12;
constexpr uint8_t DIRECTION_RIGHT_PIN = 13;

// Сенсоров.
constexpr uint8_t SENSOR_LEFT_PIN = 6;
constexpr uint8_t SENSOR_RIGHT_PIN = 5;

// Не используется, пин тормоза. Показал свою неэффективность в ходе разработки.
constexpr uint8_t BRAKE_LEFT_PIN = 8;
constexpr uint8_t BRAKE_RIGHT_PIN = 9;

// Пин скорости
constexpr uint8_t SPEED_RIGHT_PIN = 3;
constexpr uint8_t SPEED_LEFT_PIN = 11;

// Для упрощения чтения программы, направление.
constexpr uint8_t FORWARD = HIGH;
constexpr uint8_t BACKWARD = LOW;

// ВКЛ | ВЫКЛ
constexpr uint8_t ON = HIGH;
constexpr uint8_t OFF = LOW;

// Количество миллисекунд на единицу поворота (в радианах).
constexpr float MS_PER_ANGLE = 150;

// Время начала таймера.
unsigned long lastTimeStamp = 0;

// Обнуление таймера.
inline void resetTimer() {
  lastTimeStamp = millis();
}

// Получение отрезка времени между стартом таймера и моментом вызова функции.
inline unsigned long getTimerDeltaTime() {
  return millis() - lastTimeStamp;
}

// Установка направления для колеса.
inline void setLeftWheelDirection(uint8_t dir) {
  digitalWrite(DIRECTION_LEFT_PIN, dir);
}

inline void setRightWheelDirection(uint8_t dir) {
  digitalWrite(DIRECTION_RIGHT_PIN, !dir);
}

// Проверка на наличие пропасти у сенсора.
inline bool isRightSensorActivated() {
  return !digitalRead(SENSOR_RIGHT_PIN);
}

inline bool isLeftSensorActivated() {
  return !digitalRead(SENSOR_LEFT_PIN);
}

// Включение мотора.
inline void setLeftEngineStatus(uint8_t status) {
  digitalWrite(SPEED_LEFT_PIN, status); // Вариант с мощностью: analogWrite(SPEED_LEFT_PIN, status ? 75 : 0);
}

inline void setRightEngineStatus(uint8_t status) {
  digitalWrite(SPEED_RIGHT_PIN, status); // Вариант с мощностью: analogWrite(SPEED_RIGHT_PIN, status ? 75 : 0);
}

// Не используется, установка тормоза.
inline void setRightEngineBrake(uint8_t status) {
  digitalWrite(BRAKE_RIGHT_PIN, status);
}

inline void setLeftEngineBrake(uint8_t status) {
  digitalWrite(BRAKE_LEFT_PIN, status);
}

// Состояния робота.
enum RobotStatus {
  // Запуск
  Start,
  // Движение
  Moving,
  // Смена угла.
  ChangeAngle,
  // Торможение.
  Braking,
  // Задний ход, откат.
  Backstep
};

// Состояние робота.
RobotStatus activeStatus = Start;

// Случайное время поворота и направление.
int needWait = 0;
uint8_t engineSelect = 0;

void setup() {
  pinMode(DIRECTION_LEFT_PIN, OUTPUT);
  pinMode(DIRECTION_RIGHT_PIN, OUTPUT);
  pinMode(SENSOR_LEFT_PIN, INPUT_PULLUP);
  pinMode(SENSOR_RIGHT_PIN, INPUT_PULLUP);
  pinMode(SPEED_LEFT_PIN, OUTPUT);
  pinMode(SPEED_RIGHT_PIN, OUTPUT);
  pinMode(BRAKE_LEFT_PIN, OUTPUT);
  pinMode(BRAKE_RIGHT_PIN, OUTPUT);

  setRightWheelDirection(FORWARD);
  setLeftWheelDirection(FORWARD);

  randomSeed(analogRead(0));
}

void loop() {
  if (activeStatus == Start) {
    if (!isLeftSensorActivated() && !isRightSensorActivated()) {
      if (getTimerDeltaTime() > 500) {
        activeStatus = Moving;
        resetTimer();
      }
    } else {
      resetTimer();
    }
    setLeftEngineStatus(OFF);
    setRightEngineStatus(OFF);
  }
  else if (activeStatus == Moving) {
    if (isLeftSensorActivated() || isRightSensorActivated()) {
      if (getTimerDeltaTime() > 10) {
        resetTimer();
        activeStatus = Backstep;
        return;
      }
    } else {
      resetTimer();
    }

    setRightWheelDirection(FORWARD);
    setLeftWheelDirection(FORWARD);
    setLeftEngineStatus(ON);
    setRightEngineStatus(ON);
  } else if (activeStatus == Backstep) {
    if (getTimerDeltaTime() > 100 && !isLeftSensorActivated() && !isRightSensorActivated()) {
        activeStatus = ChangeAngle;
        resetTimer();
        return;
    }
    setLeftWheelDirection(BACKWARD);
    setRightWheelDirection(BACKWARD);
    setLeftEngineStatus(ON);
    setRightEngineStatus(ON);
  } else if (activeStatus == ChangeAngle) {
    if (needWait == 0) {
      constexpr uint16_t half_pi_power = 15707;
      constexpr uint16_t three_pi_half_power = 47123;

      float randomAngle = random(half_pi_power, three_pi_half_power) / 10000.0;
      needWait = randomAngle * MS_PER_ANGLE;
      engineSelect = random(0, 2);
    }

    if (getTimerDeltaTime() > needWait) {
      needWait = 0;
      resetTimer();
      activeStatus = Moving;
      return;
    } else {
      if (engineSelect == 0) {
        setLeftWheelDirection(BACKWARD);
        setRightWheelDirection(FORWARD);
      } else {
        setLeftWheelDirection(FORWARD);
        setRightWheelDirection(BACKWARD);
      }
      setLeftEngineStatus(ON);
      setRightEngineStatus(ON);

    }
  }
}
