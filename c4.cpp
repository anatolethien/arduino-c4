#include <Keypad.h>

const int BUZZER = 13;

const int DATA = 10;
const int CLOCK = 9;
const int LATCH = 8;
int dec_digits[10] {1,79,18,6,76,36,32,15,0,4};

class Buzzer {
  int melody[8] = {262, 196, 196, 220, 196, 0, 247, 262};
  int noteDurations[8] = {4, 8, 8, 4, 4, 4, 4, 4};
public:
  void playMelodyWarning() {
    for (int thisNote = 0; thisNote < 5; thisNote++) {
      int noteDuration = 500 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote] + 200, noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER);
    }
  }
  void playMelodyError() {
    for (int thisNote = 1; thisNote < 3; thisNote++) {
      int noteDuration = 500 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote] - 100, noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER);
    }
  }
  void playMelodyGameOver() {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
      int noteDuration = 1000 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote], noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER);
    }
  }
  void playMelodyVictory() {
    for (int thisNote = 2; thisNote < 6; thisNote++) {
      int noteDuration = 500 / noteDurations[thisNote];
      tone(BUZZER, melody[thisNote] + 400, noteDuration);
      int pauseBetweenNotes = noteDuration * 1.30;
      delay(pauseBetweenNotes);
      noTone(BUZZER);
    }
  }
};

Buzzer buzzer;

class Code {
  int difficulty = 2; //change difficulty
  String code;
  String guess;
public:
  void setCode(String set) { //cheat
    code = set;
  }
  void generate() { //generate a four digit pin code
    randomSeed(analogRead(0));
    char buffer[10];
    for (int i = 0; i < difficulty; i++) {
      code.concat(ltoa(random(1, 9), buffer, 10));
    }
  }
  void resetGuess() {
    guess = "";
  }
  bool isComplete() {
    if (guess.length() >= difficulty) {
      return true;
    } else {
      return false;
    }
  }
  bool doesMatch() {
    if (code == guess) {
      return true;
    } else {
      return false;
    }
  }
  void appendGuess(char key) {
    if (isComplete()) {
      resetGuess();
    }
    Serial.print(key);
    guess.concat(key);
    if (isComplete()) {
      Serial.print("\n");
      if (!doesMatch()) {
        Serial.println("Wrong code! Try again!");
        buzzer.playMelodyError();
      }
    }
  }
};

Code code;

class Timer {
  // unsigned long timer = 15000; //15 seconds
  unsigned long timer = 40000; //40 seconds
  unsigned long startTime;
public:
  void start() {
    startTime = millis();
  }
  unsigned long elapsed() {
    return millis() - startTime;
  }
  int remaining() {
    return int((timer - elapsed()) / 1000);
  }
  bool isOver() {
    if (remaining() <= 0) {
      return true;
    } else {
      return false;
    }
  }
};

Timer timer;

const byte ROWS = 3; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};
byte rowPins[ROWS] = {7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 3, 2}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600);
  pinMode(DATA, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  code.generate();
  //code.setCode("69");
}

void loop() {
  timer.start();
  while (isRunning()) {
    delay(100); //tinkercad.com lag
    char key = keypad.getKey();
    if (key != NO_KEY) {
      code.appendGuess(key);
    }
    if (timer.remaining() == 10) {
      buzzer.playMelodyWarning();
    }
    displayNumber(timer.remaining());
  }
  displayNumber(0);
  if (code.doesMatch() && !timer.isOver()) {
    Serial.println("\nCongratulations! You win!");
    buzzer.playMelodyVictory();
    code.generate();
  } else {
    Serial.println("\nTime's up! You lose!");
    buzzer.playMelodyGameOver();
  }
}

int firstDigit(int n) {
  while (n >= 10) {
    n /= 10;
  }
  return n;
}

int lastDigit(int n) {
  return n % 10;
}

void displayNumber(int n) {
  int first = firstDigit(n);
  int last = lastDigit(n);
  digitalWrite(CLOCK, LOW);
  if (n < 10) {
    shiftOut(DATA, LATCH, LSBFIRST, dec_digits[n]);
    shiftOut(DATA, LATCH, LSBFIRST, dec_digits[0]);
  } else {
    shiftOut(DATA, LATCH, LSBFIRST, dec_digits[last]);
    shiftOut(DATA, LATCH, LSBFIRST, dec_digits[first]);
  }
  digitalWrite(CLOCK, HIGH);
}

bool isRunning() {
  if (code.isComplete() && code.doesMatch()) {
    return false;
  } else if (timer.isOver()) {
    return false;
  } else {
    return true;
  }
}
