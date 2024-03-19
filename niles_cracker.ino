void setup() {
  // Initialize serial communication at 38400 baud:
  Serial.begin(38400);
}

void loop() {
  // Check if data is available to read
  if (Serial.available() > 0) {
    // Read the incoming string
    String incomingString = Serial.readString();

    // Trim any whitespace or newline characters
    incomingString.trim();

    // Check if the incoming string is what we're looking for
    if (incomingString == "\x1b") {
      Serial.println("...# Bam IR Capture Unit Ver 0.35      31Oct05..# Firmware Downloadable...");
    }
  }
}

