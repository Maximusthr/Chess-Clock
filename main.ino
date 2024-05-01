#include <Wire.h>               // Inclui a biblioteca Wire para comunicação I2CAD
#include <LiquidCrystal_I2C.h>  // Inclui a biblioteca para LCD I2C
#include <TM1637Display.h>      // Inclui a biblioteca para p display de 7 segmentos TM1637

#define DIO_PIN_1 9
#define CLK_PIN_1 8
#define DIO_PIN_2 11
#define CLK_PIN_2 10
#define PAUSE_BUTTON_PIN 2
#define INCREMENT_BUTTON_1_PIN 3
#define INCREMENT_BUTTON_2_PIN 1
#define BUTTON_PLUS_PIN 7
#define BUTTON_MINUS_PIN 6
#define BUTTON_PLUS_INC 5
#define BUTTON_MINUS_INC 4
#define BUTTON_START 13
#define BUZZER 12

double number1 = 0;
double number2 = 0;

int time1;
int time2;

int incremento = 0;

int player = 1;
int moves = 0;

bool start = false;
bool buzzerPlay = false;

volatile bool paused = false;
volatile bool lastPauseButtonState = HIGH;
volatile bool lastIncrementButton1State = HIGH;
volatile bool lastIncrementButton2State = HIGH;
volatile bool lastButtonPlus = HIGH;
volatile bool lastButtonMinus = HIGH;
volatile bool lastButtonPlusSec = HIGH;
volatile bool lastButtonMinusSec = HIGH;
volatile bool lastButtonStart = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Define o endereço I2C do LCD (0x27) e o tamanho (16x2)

// Cria instâncias dos displays T1637
TM1637Display display1(CLK_PIN_1, DIO_PIN_1);
TM1637Display display2(CLK_PIN_2, DIO_PIN_2);

void setup() {
  pinMode(PAUSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INCREMENT_BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(INCREMENT_BUTTON_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PLUS_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MINUS_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PLUS_INC, INPUT_PULLUP);
  pinMode(BUTTON_MINUS_INC, INPUT_PULLUP);
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display1.setBrightness(7);  // Define o brilho do display 7 segmentos
  display2.setBrightness(7);
  display1.showNumberDec(number1, false);
  display2.showNumberDec(number2, false);

  lcd.init();       // Inicializa o LCD
  lcd.backlight();  // Liga o backlight (iluminação) do LCD

  lcd.setCursor(0, 0);
  lcd.print("RELOGIO XADREZ");
  lcd.setCursor(0, 1);
  lcd.print("ENGENHARIA");
  delay(1000);
  lcd.setCursor(0, 1);
  lcd.print("COMPUTACAO");
  delay(1000);
  for (int i = 0; i < 16; i++) {
    lcd.scrollDisplayLeft();
    delay(100);
  }
  lcd.clear();
}

void loop() {
  debounceButtons();

  if (!start) {
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("SET TIME");
  } else {
    lcd.clear();
    if (!paused) {
      if (player == 1 && number1 > 0 && number2 > 0) {
        lcd.setCursor(4, 0);
        lcd.print("BRANCAS");
        number1 -= 0.1;
        timeFormat();
        display1.showNumberDec(time1, false);
      } else if (number2 > 0 && number1 > 0) {
        lcd.setCursor(4, 0);
        lcd.print(" PRETAS");
        number2 -= 0.1;
        timeFormat();
        display2.showNumberDec(time2, false);
      } else {
        if (number1 < number2) {
          endByTime("PRETAS", moves);
        } else {
          endByTime("BRANCAS", moves);
        }
        if (!buzzerPlay) {
          playBuzzer();
        }
      }
      lcd.setCursor(14, 1);
      lcd.print(moves);

    } else {
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("PAUSA");
    }
  }

  delay(60);
}

void debounceButtons() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastDebounceTime > debounceDelay) {
    bool pauseButtonState = digitalRead(PAUSE_BUTTON_PIN);
    bool incrementButton1State = digitalRead(INCREMENT_BUTTON_1_PIN);
    bool incrementButton2State = digitalRead(INCREMENT_BUTTON_2_PIN);
    bool buttonPlus = digitalRead(BUTTON_PLUS_PIN);
    bool buttonMinus = digitalRead(BUTTON_MINUS_PIN);
    bool buttonPlusSec = digitalRead(BUTTON_PLUS_INC);
    bool buttonMinusSec = digitalRead(BUTTON_MINUS_INC);
    bool buttonStart = digitalRead(BUTTON_START);

    if (pauseButtonState != lastPauseButtonState) {
      lastPauseButtonState = pauseButtonState;
      if (pauseButtonState == LOW) {
        paused = !paused;
      }
    }

    if (incrementButton1State != lastIncrementButton1State) {
      lastIncrementButton1State = incrementButton1State;
      if (incrementButton1State == LOW && player == 1) {
        incrementDisplay(1);
      }
    }

    if (incrementButton2State != lastIncrementButton2State) {
      lastIncrementButton2State = incrementButton2State;
      if (incrementButton2State == LOW && player == 2) {
        moves += 1;
        incrementDisplay(2);
      }
    }

    if (buttonPlus != lastButtonPlus) {
      lastButtonPlus = buttonPlus;
      if (buttonPlus == LOW) {
        number1 += 60;
        number2 += 60;
      }
    }

    if (buttonMinus != lastButtonMinus) {
      lastButtonMinus = buttonMinus;
      if (buttonMinus == LOW && number1 > 0) {
        number1 -= 60;
        number2 -= 60;
      }
    }

    if (buttonPlusSec != lastButtonPlusSec) {
      lastButtonPlusSec = buttonPlusSec;
      if (buttonPlusSec == LOW) {
        incremento += 1;
      }
    }

    if (buttonMinusSec != lastButtonMinusSec) {
      lastButtonMinusSec = buttonMinusSec;
      if (buttonMinusSec == LOW && incremento > 0) {
        incremento -= 1;
      }
    }

    if (buttonStart != lastButtonStart) {
      lastButtonStart = buttonStart;
      if (buttonStart == LOW) {
        if (start) {
          number1 = 0;
          number2 = 0;
          incremento = 0;
          player = 1;
          moves = 0;
          buzzerPlay = false;
          start = false;
        } else if (number1 > 0 || number2 > 0) {
          start = true;
        }
      }
    }

    lastDebounceTime = currentMillis;
  }
}

void incrementDisplay(int displayNumber) {
  if (displayNumber == 1) {
    player = 2;
    number1 += incremento;
    timeFormat();
    display1.showNumberDec(time1, false);
  } else if (displayNumber == 2) {
    player = 1;
    number2 += incremento;
    timeFormat();
    display2.showNumberDec(time2, false);
  }
}

void timeFormat() {
  int minRest1 = (int)number1 / 60;
  int secRest1 = (int)number1 % 60;
  time1 = minRest1 * 100 + secRest1;

  int minRest2 = (int)number2 / 60;
  int secRest2 = (int)number2 % 60;
  time2 = minRest2 * 100 + secRest2;
}

void endByTime(String vencedor, int lances) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(vencedor + " VENCEM!");
  lcd.setCursor(3, 1);
  lcd.print("Lances: " + String(lances));
  delay(1000);
}

void playBuzzer() {
  digitalWrite(BUZZER, HIGH);
  tone(BUZZER, 1000, 2500);
  buzzerPlay = true;
}