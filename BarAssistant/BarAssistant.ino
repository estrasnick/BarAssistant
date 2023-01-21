// Compile for Arduino Nano 33 IoT
// Select "ArduinoOTA" as programmer and use "Upload using programmer"

#include <RGBmatrixPanel.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoOTA.h>
#include <Adafruit_SleepyDog.h>
#include "arduino_secrets.h" 

#define NITEMS(x) sizeof(x) / sizeof(x[0])
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 32
#define CHAR_WIDTH 5
#define CHAR_HEIGHT 8
#define DEFAULT_FOUR_ROW false
#define SCROLL_LOOP_WHITESPACE 4 // Amount of whitespace to introduce when looping scrolled text
#define SCROLL_COUNTER_MAX 1 // Higher numbers yield a slower scroll speed
#define DISPLAY_DURATION_MS 120000 // How long to display a result
#define IDLE_FPS 18         // Maximum frames-per-second for the idle animation
#define IDLE_SATURATION 255 // Saturation of idle animation (0-255)
#define IDLE_VALUE 140 // Value (in HSV) of idle animation (0-255, but seems to not display at low values)
#define DEFAULT_IDLE_ON true // If true, the board will boot with idle animation set to play
#define HUE_INCREMENT 0 // Amount by which to cycle through hue range for each font color (0 will disable cycling)
#define TEXT_SATURATION 255 // Saturation of font (0-255)
#define TEXT_VALUE 150 // Value (in HSV) of font (0-255, but seems to not display at low values)
#define CONNECTION_CHECK_INTERVAL_MS 3000 // Period to check wifi connection
#define PING_CHECK_INTERVAL_MS 110000 // Period to check connection by pinging a host
#define PING_HOSTNAME "192.168.1.1" // Set to the address of any device on your local network to ping for connectivity checks
#define PERIODIC_RECONNECT_INTERVAL_MS 7000000 // Period to periodically force a reconnect
#define ENABLE_PING_CHECK true // If true, will periodically check connection via pings
#define ENABLE_PERIODIC_RECONNECT false // If true, will periodically force a reconnect to avoid TCP keepalive issue
#define PING_FAILURES_UNTIL_RECONNECT 3 // Number of failed pings in a row before triggering a reconnect
#define WEBSERVER_PORT 8067
#define CLIENT_TIMEOUT_MS 3000 // Number of MS before timing out a client connection

#define CLK A6
#define OE  10
#define LAT 11
#define A   A0
#define B   A1
#define C   A2
#define D   A3

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
IPAddress ip(192, 168, 1, 126);  // static IP

int status = WL_IDLE_STATUS;
WiFiServer server(WEBSERVER_PORT);

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true, 64);

#define COLOR_BASE matrix.Color333(4,0,0); // Old, unused
#define HUE_START_BASE -1535
#define HUE_MAX_BASE 0
#define COLOR_MODIFIER matrix.Color333(0,0,4); // Old, unused
#define HUE_START_MODIFIER -511
#define HUE_MAX_MODIFIER 1024
#define COLOR_SEASONING matrix.Color333(0,4,0); // Old, unused
#define HUE_START_SEASONING -1023
#define HUE_MAX_SEASONING 512

enum Role {
  base,
  modifier,
  seasoning,
  nonerole
};

enum Type {
  spirit,
  juice,
  wine,
  liqueur,
  bitters,
  syrup,
  carbonation,
  spice,
  other,
  nonetype
};

struct ingredient {
  String id;
  Role role;
  Type type;
  bool instock;
};


/* The main ingredient list. Add inventory here! */
/* { NAME, ROLE, TYPE, IN_STOCK? } */
ingredient ingredients[] = {
  {"absinthe", seasoning, liqueur, true},
  {"agave syrup", seasoning, syrup, true},
  {"allspice dram", seasoning, liqueur, true},
  {"amaretto", modifier, liqueur, true},
  {"amaro nonino", modifier, liqueur, true},
  {"angostura", seasoning, bitters, true},
  {"aperol", modifier, liqueur, true},
  {"benedictine", modifier, liqueur, true},
  {"bourbon", base, spirit, true},
  {"brandy", base, spirit, true},
  {"bruto americano", modifier, liqueur, true},
  {"butter brown sugar syrup", seasoning, bitters, true},
  {"campari", modifier, liqueur, true},
  {"cardamom bitters", seasoning, bitters, true},
  {"champagne", modifier, wine, true},
  {"cherry heering", modifier, liqueur, true},
  {"chocolate bitters", seasoning, bitters, true},
  {"cinnamon", seasoning, spice, true},
  {"citrus vodka", base, spirit, true},
  {"club soda", modifier, carbonation, true},
  {"cocchi americano", modifier, liqueur, true},
  {"coffee liqueur", modifier, liqueur, true},
  {"cointreau", modifier, liqueur, true},
  {"coke", modifier, carbonation, false},
  {"cranberry juice", modifier, juice, true},
  {"creme de banane", modifier, liqueur, true},
  {"creme de cacao", modifier, liqueur, true},
  {"creme de cassis", modifier, liqueur, true},
  {"creme de menthe", modifier, liqueur, false},
  {"curacao", modifier, liqueur, true},
  {"cynar", modifier, liqueur, true},
  {"dry rye gin", base, spirit, true},
  {"dry vermouth", modifier, wine, true},
  {"egg white", seasoning, other, true},
  {"falernum", modifier, liqueur, true},
  {"genepy", modifier, liqueur, true},
  {"gin", base, spirit, true},
  {"gin (terroir)", base, spirit, true},
  {"ginger ale", modifier, carbonation, false},
  {"ginger beer", modifier, carbonation, true},
  {"grand marnier", modifier, liqueur, true},
  {"grapefruit bitters", seasoning, bitters, true},
  {"grapefruit juice", modifier, juice, true},
  {"green chartreuse", modifier, liqueur, true},
  {"green chili vodka", base, spirit, true},
  {"grenadine", seasoning, juice, true},
  {"heavy cream", modifier, other, true},
  {"honey syrup", seasoning, syrup, true},
  {"irish cream", modifier, liqueur, true},
  {"lemon juice", modifier, juice, true},
  {"lavender bitters", seasoning, bitters, true},
  {"lillet blanc", modifier, liqueur, true},
  {"lime juice", modifier, juice, true},
  {"luxardo bitter bianco", modifier, liqueur, true},
  {"maraschino liqueur", modifier, liqueur, true},
  {"mezcal", base, spirit, true},
  {"milk", modifier, other, true},
  {"nutmeg", seasoning, spice, true},
  {"orange bitters", seasoning, bitters, true},
  {"orange juice", modifier, juice, true},
  {"orgeat", modifier, syrup, true},
  {"pear brandy", base, spirit, true},
  {"peychaud's", seasoning, bitters, true},
  {"pineapple juice", modifier, juice, true},
  {"port", modifier, wine, true},
  {"root beer", modifier, carbonation, true},
  {"rum", base, spirit, true},
  {"rye", base, spirit, true},
  {"saline solution", seasoning, other, true},
  {"scotch", base, spirit, true},
  {"sherry", modifier, wine, true},
  {"shochu", base, spirit, true},
  {"simple syrup", seasoning, syrup, true},
  {"spiced pear liqueur", modifier, liqueur, true},
  {"st. germain", modifier, liqueur, true},
  {"sweet vermouth", modifier, wine, true},
  {"tequila", base, spirit, true},
  {"tonic water", modifier, carbonation, true},
  {"vanilla bitters", seasoning, bitters, true},
  {"vanilla extract", seasoning, syrup, true},
  {"velvet falernum", modifier, syrup, true},
  {"vodka", base, spirit, true},
  {"vodka (sichuan peppercorn)", base, spirit, true},
  {"wine", modifier, wine, true},
  {"yellow chartreuse", modifier, liqueur, true}
};

enum State {
  init_s,
  idle_s,
  displaying_s
};

struct textrow {
  String text;
  int16_t textX;
  int16_t textMin;
  bool scroll;
  long hue_current;
  long hue_start;
  long hue_max;
  bool enable;
};

textrow textrows[4];

State current_state = init_s;
bool four_row = DEFAULT_FOUR_ROW;
bool poll_mode = false;

int scroll_counter = 0;
unsigned long display_start_time;
int num_ingredients = NITEMS(ingredients);
unsigned long reconnect_timer = 0;
unsigned long periodic_reconnect_timer = 0;
unsigned long ping_timer = 0;
uint16_t ping_failure_counter = 0;
unsigned long client_timer = 0;

int client_timeout_stat = 0;
int ping_failure_stat = 0;
int dropped_connection_stat = 0;
int client_connection_stat = 0;

static const int8_t PROGMEM sinetab[256] = {
     0,   2,   5,   8,  11,  15,  18,  21,
    24,  27,  30,  33,  36,  39,  42,  45,
    48,  51,  54,  56,  59,  62,  65,  67,
    70,  72,  75,  77,  80,  82,  85,  87,
    89,  91,  93,  96,  98, 100, 101, 103,
   105, 107, 108, 110, 111, 113, 114, 116,
   117, 118, 119, 120, 121, 122, 123, 123,
   124, 125, 125, 126, 126, 126, 126, 126,
   127, 126, 126, 126, 126, 126, 125, 125,
   124, 123, 123, 122, 121, 120, 119, 118,
   117, 116, 114, 113, 111, 110, 108, 107,
   105, 103, 101, 100,  98,  96,  93,  91,
    89,  87,  85,  82,  80,  77,  75,  72,
    70,  67,  65,  62,  59,  56,  54,  51,
    48,  45,  42,  39,  36,  33,  30,  27,
    24,  21,  18,  15,  11,   8,   5,   2,
     0,  -3,  -6,  -9, -12, -16, -19, -22,
   -25, -28, -31, -34, -37, -40, -43, -46,
   -49, -52, -55, -57, -60, -63, -66, -68,
   -71, -73, -76, -78, -81, -83, -86, -88,
   -90, -92, -94, -97, -99,-101,-102,-104,
  -106,-108,-109,-111,-112,-114,-115,-117,
  -118,-119,-120,-121,-122,-123,-124,-124,
  -125,-126,-126,-127,-127,-127,-127,-127,
  -128,-127,-127,-127,-127,-127,-126,-126,
  -125,-124,-124,-123,-122,-121,-120,-119,
  -118,-117,-115,-114,-112,-111,-109,-108,
  -106,-104,-102,-101, -99, -97, -94, -92,
   -90, -88, -86, -83, -81, -78, -76, -73,
   -71, -68, -66, -63, -60, -57, -55, -52,
   -49, -46, -43, -40, -37, -34, -31, -28,
   -25, -22, -19, -16, -12,  -9,  -6,  -3
};

const float radius1 =16.3, radius2 =23.0, radius3 =40.8, radius4 =44.2,
            centerx1=16.1, centerx2=11.6, centerx3=23.4, centerx4= 4.1,
            centery1= 8.7, centery2= 6.5, centery3=14.0, centery4=-2.9;
float       angle1  = 0.0, angle2  = 0.0, angle3  = 0.0, angle4  = 0.0;
long        hueShift= 0;
uint32_t prevTime = 0; // For frame-to-frame interval timing
bool idle_on = DEFAULT_IDLE_ON;

void idle_animation() {
  int           x1, x2, x3, x4, y1, y2, y3, y4, sx1, sx2, sx3, sx4;
  unsigned char x, y;
  long          value;

  // To ensure that animation speed is similar on AVR & SAMD boards,
  // limit frame rate to FPS value (might go slower, but never faster).
  // This is preferable to delay() because the AVR is already plenty slow.
  uint32_t t = millis();

  if ((t - prevTime) < 1000 / IDLE_FPS) {
    return;
  }
  prevTime = t;

  sx1 = (int)(cos(angle1) * radius1 + centerx1);
  sx2 = (int)(cos(angle2) * radius2 + centerx2);
  sx3 = (int)(cos(angle3) * radius3 + centerx3);
  sx4 = (int)(cos(angle4) * radius4 + centerx4);
  y1  = (int)(sin(angle1) * radius1 + centery1);
  y2  = (int)(sin(angle2) * radius2 + centery2);
  y3  = (int)(sin(angle3) * radius3 + centery3);
  y4  = (int)(sin(angle4) * radius4 + centery4);

  for(y=0; y<matrix.height(); y++) {
    x1 = sx1; x2 = sx2; x3 = sx3; x4 = sx4;
    for(x=0; x<matrix.width(); x++) {
      value = hueShift
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x1 * x1 + y1 * y1) >> 2))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x2 * x2 + y2 * y2) >> 2))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x3 * x3 + y3 * y3) >> 3))
        + (int8_t)pgm_read_byte(sinetab + (uint8_t)((x4 * x4 + y4 * y4) >> 3));
      matrix.drawPixel(x, y, matrix.ColorHSV(value * 3, IDLE_SATURATION, IDLE_VALUE, true));
      x1--; x2--; x3--; x4--;
    }
    y1--; y2--; y3--; y4--;
  }

  angle1 += 0.03;
  angle2 -= 0.07;
  angle3 += 0.13;
  angle4 -= 0.15;
  hueShift += 2;
}

void get_random_numbers_without_repeat(int range, int arr_size, int arr[], bool check_stock) {
  for (int i = 0; i < arr_size; i++) {
    bool repeat;
    do {
      repeat = false;
      arr[i] = random(range);

      // Check stock
      if (check_stock) {
        while (!ingredients[arr[i]].instock) {
          arr[i] = random(range);
        }
      }

      // Check for repeats
      for (int j = 0; j < i; j++) {
        if (arr[j] == arr[i]) {
          repeat = true;
          break;
        }
      }
    } while (repeat);
  }
}

ingredient get_ingredient_with_requirements(Role required_role, Type required_type, bool check_stock) {
  bool match;
  ingredient ing;
  do {
    ing = ingredients[random(num_ingredients)];
    match = true;
    // Check stock
    if (check_stock) {
      if (!ing.instock) {
        match = false;
      }
    }
      
    if (required_role != nonerole) {
      if (ing.role != required_role) {
        match = false;
      }
    }

    if (required_type != nonetype) {
      if (ing.type != required_type) {
        match = false;
      }
    }
  } while (!match);
  return ing;
}

void random_ingredients(int n, ingredient ing_arr[]) {
  int nums[n];
  get_random_numbers_without_repeat(num_ingredients, n, nums, true);
  for (int i = 0; i < n; i++) {
    ing_arr[i] = ingredients[nums[i]];
  }
}

void print_rows() {
  int max_index = (four_row) ? 4 : 3;
  bool to_scroll = scroll_counter >= SCROLL_COUNTER_MAX;
  for (int i = 0; i < max_index; i++) {
    if (!textrows[i].enable) {
      continue;
    }
    matrix.setTextColor(matrix.ColorHSV(textrows[i].hue_current, TEXT_SATURATION, TEXT_VALUE, true));
    textrows[i].hue_current += HUE_INCREMENT;
    if (textrows[i].hue_current >= textrows[i].hue_max) {
      textrows[i].hue_current = textrows[i].hue_start;
    }
    int starty = (four_row) ? (CHAR_HEIGHT) * i : (CHAR_HEIGHT + 2) * i + 2;
    if (textrows[i].scroll && to_scroll) {
      textrows[i].textX--;
      if(textrows[i].textX < textrows[i].textMin) {
        textrows[i].textX = -1;
      }
    }
    matrix.setCursor(textrows[i].textX, starty);
    matrix.print(textrows[i].text);
  }
  scroll_counter = (to_scroll) ? 0 : scroll_counter + 1;
}

// Old, unused
void print_text_to_row(int rownum, uint16_t color, String text) {
  matrix.setTextColor(color);
  int pixel_width = text.length() * (CHAR_WIDTH + 1);
  int startx = (PANEL_WIDTH - pixel_width) / 2;
  if (startx < 0) { // overflow
    startx = 0;
  }
  int starty;
  if (four_row) {
    starty = (CHAR_HEIGHT) * rownum;
  } else {
    starty = (CHAR_HEIGHT + 2) * rownum + 2;
  }
  matrix.setCursor(startx, starty);
  for (int i = 0; i < text.length(); i++) {
    matrix.print(text[i]);
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void try_reconnect() {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Attempting reconnect...");
  WiFi.disconnect();
  //delay(2000);
  WiFi.end();
  delay(1000);
  WiFi.config(ip);
  while (status != WL_CONNECTED) {
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }
  Serial.println("Reconnected.");
  server.begin();                           // start the web server
  ArduinoOTA.begin(WiFi.localIP(), "Arduino", OTA_PASS, InternalStorage);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  current_state = init_s;
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  randomSeed(analogRead(0));
  Serial.begin(9600);
  
  while (!Serial && millis()<3000) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  matrix.begin();
  uint8_t r=0, g=0, b=0;

  matrix.setTextSize(1);     // size 1 == 8 pixels high
  matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  WiFi.config(ip);
  WiFi.noLowPowerMode();

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }
  reconnect_timer = millis();
  periodic_reconnect_timer = millis();
  ping_timer = millis();
  server.begin(); // start the web server

  // start the WiFi OTA library with internal (flash) based storage
  ArduinoOTA.begin(WiFi.localIP(), "Arduino", OTA_PASS, InternalStorage);
  
  printWifiStatus();  // you're connected now, so print out the status

  current_state = idle_s;

  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  // Check for disconnect
  if ((millis() - reconnect_timer > CONNECTION_CHECK_INTERVAL_MS) || (millis() < reconnect_timer)){
    status = WiFi.status();
    if (status != WL_CONNECTED) {
      dropped_connection_stat++;
      try_reconnect();
    }
    reconnect_timer = millis();
  }

  // Periodically force a reconnect
  if (ENABLE_PERIODIC_RECONNECT) {
    if (current_state == idle_s) {
      if ((millis() - periodic_reconnect_timer > PERIODIC_RECONNECT_INTERVAL_MS) || (millis() < periodic_reconnect_timer)){
        try_reconnect();
        periodic_reconnect_timer = millis();
      }
    }
  }

  // Check WAN periodically via ping (not while displaying)
  if (ENABLE_PING_CHECK) {
    if (current_state == idle_s) {
      if ((millis() - ping_timer > PING_CHECK_INTERVAL_MS) || (millis() < ping_timer)){
        int ping_result = WiFi.ping(PING_HOSTNAME);
        if (ping_result < 0) {
          ping_failure_counter++;
          if (ping_failure_counter >= PING_FAILURES_UNTIL_RECONNECT) {
            ping_failure_stat++;
            try_reconnect();
            ping_failure_counter = 0;
          }
        }
        else {
          ping_failure_counter = 0;
        }
        ping_timer = millis();
      }
    }
  }

  if (poll_mode) {
    // check for WiFi OTA updates
    ArduinoOTA.poll();
  } else {
    if ((current_state == displaying_s) && ((millis() > display_start_time + DISPLAY_DURATION_MS) || (millis() < display_start_time))) {
      current_state = idle_s;
  
      // Clear background
      matrix.fillScreen(0);
      
      // Update display
      matrix.swapBuffers(false);
    } else if (current_state == displaying_s) { // update screen
      // Clear background
      matrix.fillScreen(0);
  
      // Write ingredients
      print_rows();
      
      // Update display
      matrix.swapBuffers(false);
      
    } else if (current_state == idle_s) {
      if (idle_on) {
        // Play idle animation
        idle_animation();
      } else {
        matrix.fillScreen(0);
      }
     
      // Update display
      matrix.swapBuffers(false);
    }
    
    WiFiClient client = server.available();   // listen for incoming clients
  
    if (client) {
      //Serial.println("new client");           // print a message out the serial port
      String currentLine = "";
      client_timer = millis();
      client_connection_stat++;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          if (c == '\n') {
            if (currentLine.length() == 0) {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();
  
              client.print("Click <a href=\"/random_rolebalance\">here</a> to seek inspiration!<br>");
              client.print("Click here for <a href=\"/random_1\">1</a>, <a href=\"/random_2\">2</a>, or <a href=\"/random_3\">3</a> random ingredients (no role balancing).<br>");
              client.print("Click here to turn idle animation <a href=\"/idle_on\">on</a> or <a href=\"/idle_off\">off</a>.<br>");
              client.print("Click here to turn on <a href=\"/pollmode\">OTA polling mode</a>.<br>");
              client.print("Click here to <a href=\"/reset\">reset</a> the system, or to <a href=\"/reconnect\">reconnect the WiFi</a>.<br>");
              
              client.print("<br><br>Stats:");
              client.print("<br>Uptime: ");
              client.print(String(millis()));
              client.print("<br>Client connections: ");
              client.print(String(client_connection_stat));
              client.print("<br>Client timeouts: ");
              client.print(String(client_timeout_stat));
              client.print("<br>Dropped connections: ");
              client.print(String(dropped_connection_stat));
              client.print("<br>Ping failures: ");
              client.print(String(ping_failure_stat));
              client.print("<br>");
  
              client.println();
              break;
            } else {
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
  
          if (currentLine.endsWith("GET /random_rolebalance")) {
            four_row = false;
            ingredient ing1 = get_ingredient_with_requirements(base, nonetype, true);
            ingredient ing2 = get_ingredient_with_requirements(modifier, nonetype, true);
            ingredient ing3 = get_ingredient_with_requirements(seasoning, nonetype, true);
  
            if ((CHAR_WIDTH + 1) * ing1.id.length() > PANEL_WIDTH) { // scroll
              String text1 = ing1.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text1 += " ";
              }
              text1 += ing1.id;
              textrows[0] = {text1, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing1.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            } else {
              textrows[0] = {ing1.id, (PANEL_WIDTH - (ing1.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            }
            if ((CHAR_WIDTH + 1) * ing2.id.length() > PANEL_WIDTH) { // scroll
              String text2 = ing2.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text2 += " ";
              }
              text2 += ing2.id;
              textrows[1] = {text2, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing2.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            } else {
              textrows[1] = {ing2.id, (PANEL_WIDTH - (ing2.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            }
            if ((CHAR_WIDTH + 1) * ing3.id.length() > PANEL_WIDTH) { // scroll
              String text3 = ing3.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text3 += " ";
              }
              text3 += ing3.id;
              textrows[2] = {text3, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing3.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            } else {
              textrows[2] = {ing3.id, (PANEL_WIDTH - (ing3.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            }
  
            current_state = displaying_s;
            display_start_time = millis();
          } else if (currentLine.endsWith("GET /idle_on")) {
            idle_on = true;
          } else if (currentLine.endsWith("GET /idle_off")) {
            idle_on = false;
          } else if (currentLine.endsWith("GET /random_1")) {
            four_row = false;
            ingredient ing2 = get_ingredient_with_requirements(nonerole, nonetype, true);
  
            if ((CHAR_WIDTH + 1) * ing2.id.length() > PANEL_WIDTH) { // scroll
              String text2 = ing2.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text2 += " ";
              }
              text2 += ing2.id;
              textrows[1] = {text2, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing2.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            } else {
              textrows[1] = {ing2.id, (PANEL_WIDTH - (ing2.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            }
            textrows[0] = {"", 0, 0, false, 0, 0, 0, false};
            textrows[2] = {"", 0, 0, false, 0, 0, 0, false};
            
            current_state = displaying_s;
            display_start_time = millis();
          } else if (currentLine.endsWith("GET /random_2")) {
            four_row = false;
            ingredient ing1 = get_ingredient_with_requirements(nonerole, nonetype, true);
            ingredient ing3 = get_ingredient_with_requirements(nonerole, nonetype, true);
  
            if ((CHAR_WIDTH + 1) * ing1.id.length() > PANEL_WIDTH) { // scroll
              String text1 = ing1.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text1 += " ";
              }
              text1 += ing1.id;
              textrows[0] = {text1, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing1.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            } else {
              textrows[0] = {ing1.id, (PANEL_WIDTH - (ing1.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            }
            if ((CHAR_WIDTH + 1) * ing3.id.length() > PANEL_WIDTH) { // scroll
              String text3 = ing3.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text3 += " ";
              }
              text3 += ing3.id;
              textrows[2] = {text3, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing3.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            } else {
              textrows[2] = {ing3.id, (PANEL_WIDTH - (ing3.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            }
  
            textrows[1] = {"", 0, 0, false, 0, 0, 0, false};
            
            current_state = displaying_s;
            display_start_time = millis();
          } else if (currentLine.endsWith("GET /random_3")) {
            four_row = false;
            ingredient ing1 = get_ingredient_with_requirements(nonerole, nonetype, true);
            ingredient ing2 = get_ingredient_with_requirements(nonerole, nonetype, true);
            ingredient ing3 = get_ingredient_with_requirements(nonerole, nonetype, true);
  
            if ((CHAR_WIDTH + 1) * ing1.id.length() > PANEL_WIDTH) { // scroll
              String text1 = ing1.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text1 += " ";
              }
              text1 += ing1.id;
              textrows[0] = {text1, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing1.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            } else {
              textrows[0] = {ing1.id, (PANEL_WIDTH - (ing1.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_BASE, HUE_START_BASE, HUE_MAX_BASE, true};
            }
            if ((CHAR_WIDTH + 1) * ing2.id.length() > PANEL_WIDTH) { // scroll
              String text2 = ing2.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text2 += " ";
              }
              text2 += ing2.id;
              textrows[1] = {text2, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing2.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            } else {
              textrows[1] = {ing2.id, (PANEL_WIDTH - (ing2.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_MODIFIER, HUE_START_MODIFIER, HUE_MAX_MODIFIER, true};
            }
            if ((CHAR_WIDTH + 1) * ing3.id.length() > PANEL_WIDTH) { // scroll
              String text3 = ing3.id;
              for (int v = 0; v < SCROLL_LOOP_WHITESPACE; v++) {
                text3 += " ";
              }
              text3 += ing3.id;
              textrows[2] = {text3, PANEL_WIDTH / 2, 0 - (CHAR_WIDTH + 1) * (ing3.id.length() + SCROLL_LOOP_WHITESPACE), true, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            } else {
              textrows[2] = {ing3.id, (PANEL_WIDTH - (ing3.id.length() * (CHAR_WIDTH + 1))) / 2, 0, false, HUE_START_SEASONING, HUE_START_SEASONING, HUE_MAX_SEASONING, true};
            }
            
            current_state = displaying_s;
            display_start_time = millis();
          } else if (currentLine.endsWith("GET /reset")) {
            client.stop();  
            // Reset by timing out watchdog timer
            Watchdog.enable(15, false);
            while (true) {}
          } else if (currentLine.endsWith("GET /reconnect")) {
            client.stop();
            try_reconnect();
          } else if (currentLine.endsWith("GET /pollmode")) {
            poll_mode = true;
          }
        }
        if ((millis() - client_timer > CLIENT_TIMEOUT_MS) || (millis() < client_timer)){
          client_timeout_stat++;
          break;
        }
      }
      client.stop();
    }
  }
}
