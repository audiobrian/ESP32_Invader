#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include "wifi_config.h"

// Game Configuration
#define LED_PIN    4
#define LED_COUNT  168

// Game Timing Constants (milliseconds)
#define ENEMY_MOVE_INTERVAL   2000
#define BULLET_MOVE_INTERVAL  1   // Maximum possible speed - update every game cycle
#define GAME_UPDATE_DELAY     0   // No delay - maximum speed possible
#define WEB_REFRESH_INTERVAL  2000

// Game Configuration
#define MAX_ENEMIES           20
#define MAX_BULLETS           5
#define ENEMY_WAVE_SIZE       8
#define ENEMY_SPAWN_START     (LED_COUNT - 15)
#define LED_BRIGHTNESS        150

// WiFi Configuration
#define WIFI_CONNECT_TIMEOUT  20000  // 20 seconds

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
WebServer server(80);

// HTML/CSS/JS templates in PROGMEM to save RAM
const char HTML_HEAD[] PROGMEM = R"(
<!DOCTYPE html><html><head>
<title>LED Space Invaders</title>
<meta name='viewport' content='width=device-width, initial-scale=1.0'>
<style>
body{font-family:Arial,sans-serif;max-width:900px;margin:0 auto;padding:20px;background:#000;color:#0f0}
.container{background:#111;padding:30px;border-radius:10px;border:2px solid #0f0}
h1{text-align:center;color:#0f0;text-shadow:0 0 10px #0f0}
.led-strip{display:flex;justify-content:space-between;margin:20px 0;height:40px}
.led{width:8px;height:30px;border-radius:2px;border:1px solid #333}
.player{background:#0f0;box-shadow:0 0 10px #0f0}
.enemy-red{background:#f00;box-shadow:0 0 10px #f00}
.enemy-green{background:#0f0;box-shadow:0 0 10px #0f0}
.enemy-blue{background:#00f;box-shadow:0 0 10px #00f}
.bullet{background:#fff;box-shadow:0 0 8px #fff}
.empty{background:#222}
.game-info{display:flex;justify-content:space-between;margin:20px 0;font-size:18px}
.button{background:#0f0;color:#000;border:none;padding:20px 40px;font-size:20px;font-weight:bold;border-radius:5px;cursor:pointer;margin:10px}
.button:hover{background:#0a0}
.button.danger{background:#f00;color:#fff}
.button.danger:hover{background:#a00}
</style></head><body><div class='container'>
<h1>👾 LED SPACE INVADERS 👾</h1>
)";

const char HTML_CONTROLS_PLAYING[] PROGMEM = R"(
<div style='text-align:center'>
<button class='button' onclick='shoot()'>🔫 SHOOT!</button>
</div>
)";

const char HTML_CONTROLS_GAMEOVER[] PROGMEM = R"(
<div style='text-align:center'>
<h2>GAME OVER!</h2>
<p>Final Score: %d</p>
<button class='button danger' onclick='restart()'>🔄 RESTART</button>
</div>
)";

const char HTML_FOOTER[] PROGMEM = R"(
<div style='margin-top:30px;padding:15px;background:#222;border-radius:5px'>
<h3>How to Play:</h3>
<p>🟢 Green = You (position 0)</p>
<p>🔴🟢🔵 Red/Green/Blue = Enemy waves</p>
<p>⚪ White = Your bullets</p>
<p>Click SHOOT to fire at enemies!</p>
<p>Each shot kills one enemy!</p>
</div>
</div>
<script>
let shooting=false;
function shoot(){ if(shooting) return; shooting=true; fetch('/shoot').then(()=>{shooting=false;updateGame();}); }
function restart(){ fetch('/restart').then(()=>setTimeout(()=>location.reload(), 500)); }
function updateGame(){
  fetch('/state').then(r=>r.json()).then(d=>{
    document.getElementById('score').textContent=d.score;
    document.getElementById('lives').textContent=d.lives;
    document.getElementById('enemies').textContent=d.enemies;
    document.getElementById('bullets').textContent=d.bullets;
    if(d.gameState==2){location.reload();}
  });
}
setInterval(updateGame,%d);
</script></body></html>
)";


// Function prototypes
void handleRoot();
void handleShoot();
void handleRestart();
void handleGameState();
void handleNotFound();
void initGame();
void createEnemyWave();
void updateGame();
void renderLEDs();
int countActiveBullets();
bool checkAuth();

// Enemy colors
enum EnemyColor { RED, GREEN, BLUE };

// Game variables
int playerPos = 0;  // Player at first LED
struct Enemy {
  int position;
  EnemyColor color;
};
Enemy enemies[MAX_ENEMIES];
int numEnemies = 0;
int bullets[MAX_BULLETS];
bool bulletActive[MAX_BULLETS];
unsigned long lastEnemyMove = 0;
unsigned long lastBulletMove = 0;
int gameScore = 0;
int lives = 3;
int gameState = 0; // 0=ready, 1=playing, 2=gameOver
bool ledsDirty = true; // Track if LEDs need updating

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Space Invaders Starting...");
  
  // Initialize LEDs
  strip.begin();
  strip.clear();
  strip.show();
  strip.setBrightness(LED_BRIGHTNESS);
  
  // Random seed for enemy colors
  randomSeed(analogRead(0));
  
  // Connect to WiFi with timeout
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttempt >= WIFI_CONNECT_TIMEOUT) {
      Serial.println("\nWiFi connection failed! Starting in offline mode.");
      Serial.println("Game will run on LED strip only (no web interface).");
      break;
    }
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi Connected! IP: ");
    Serial.println(WiFi.localIP());
  }
  
  // Set up web server
  server.on("/", handleRoot);
  server.on("/shoot", handleShoot);
  server.on("/restart", handleRestart);
  server.on("/state", handleGameState);
  server.onNotFound(handleNotFound);
  
  server.begin();
  Serial.println("Game server started!");
  
  // Initialize game
  initGame();
}

void initGame() {
  gameState = 1;
  playerPos = 0;
  numEnemies = 0;
  gameScore = 0;
  lives = 3;
  ledsDirty = true; // Game state reset

  // Initialize all bullets as inactive
  for (int i = 0; i < MAX_BULLETS; i++) {
    bulletActive[i] = false;
    bullets[i] = -1;
  }

  // Clear all enemy slots (prevent ghost data)
  for (int i = 0; i < MAX_ENEMIES; i++) {
    enemies[i].position = -999;
    enemies[i].color = RED;
  }

  // Create initial enemy wave - solid formation with random colors
  createEnemyWave();
}

void createEnemyWave() {
  // Create a solid wave of enemies at right side
  for (int i = 0; i < ENEMY_WAVE_SIZE; i++) {
    if (numEnemies < MAX_ENEMIES) {
      enemies[numEnemies].position = ENEMY_SPAWN_START + i;
      enemies[numEnemies].color = (EnemyColor)random(0, 3); // Random: RED=0, GREEN=1, BLUE=2
      numEnemies++;
      ledsDirty = true; // New enemy created
    }
  }
}

void handleRoot() {
  String html = "";

  // Add head from PROGMEM
  html += FPSTR(HTML_HEAD);

  // Game info (dynamically generated)
  html += "<div class='game-info'>";
  html += "<span>Score: <strong id='score'>" + String(gameScore) + "</strong></span>";
  html += "<span>Lives: <strong id='lives'>" + String(lives) + "</strong></span>";
  html += "<span>Enemies: <strong id='enemies'>" + String(numEnemies) + "</strong></span>";
  html += "<span>Bullets: <strong id='bullets'>" + String(countActiveBullets()) + "</strong></span>";
  html += "</div>";

  // LED strip visualization (shows full strip with sampling)
  html += "<div class='led-strip'>";
  int displayLEDs = 84; // Show 84 LEDs (half of strip for better visibility)
  for (int i = 0; i < displayLEDs; i++) {
    int actualLed = map(i, 0, displayLEDs-1, 0, LED_COUNT-1);
    String ledClass = "empty";

    if (actualLed == playerPos) ledClass = "player";
    else {
      bool hasEnemy = false;
      bool hasBullet = false;
      EnemyColor enemyColor = RED; // Default

      for (int e = 0; e < numEnemies; e++) {
        if (enemies[e].position == actualLed) {
          hasEnemy = true;
          enemyColor = enemies[e].color;
          break;
        }
      }

      // Check for active bullets
      for (int b = 0; b < MAX_BULLETS; b++) {
        if (bulletActive[b] && bullets[b] == actualLed) {
          hasBullet = true;
          break;
        }
      }

      if (hasBullet) ledClass = "bullet";
      else if (hasEnemy) {
        switch(enemyColor) {
          case RED: ledClass = "enemy-red"; break;
          case GREEN: ledClass = "enemy-green"; break;
          case BLUE: ledClass = "enemy-blue"; break;
        }
      }
    }

    html += "<div class='led " + ledClass + "'></div>";
  }
  html += "</div>";

  // Controls
  if (gameState == 1) {
    html += FPSTR(HTML_CONTROLS_PLAYING);
  } else if (gameState == 2) {
    char gameOverBuf[200];
    snprintf_P(gameOverBuf, sizeof(gameOverBuf), HTML_CONTROLS_GAMEOVER, gameScore);
    html += gameOverBuf;
  }

  // Footer with script
  char footerBuf[1000];
  snprintf_P(footerBuf, sizeof(footerBuf), HTML_FOOTER, WEB_REFRESH_INTERVAL);
  html += footerBuf;

  server.send(200, "text/html", html);
}

void handleShoot() {
  if (!checkAuth()) return;

  if (gameState == 1) {
    // Find an inactive bullet slot
    for (int i = 0; i < MAX_BULLETS; i++) {
      if (!bulletActive[i]) {
        bullets[i] = playerPos + 1; // Start bullet just ahead of player
        bulletActive[i] = true;
        ledsDirty = true; // New bullet created
        break;
      }
    }
  }

  String response = "{\"success\":true,\"reload\":false}";
  server.send(200, "application/json", response);
}

void handleRestart() {
  if (!checkAuth()) return;

  initGame();
  String response = "{\"success\":true,\"reload\":true}";
  server.send(200, "application/json", response);
}

void handleGameState() {
  String json = "{";
  json += "\"score\":" + String(gameScore) + ",";
  json += "\"lives\":" + String(lives) + ",";
  json += "\"enemies\":" + String(numEnemies) + ",";
  json += "\"bullets\":" + String(countActiveBullets()) + ",";
  json += "\"gameState\":" + String(gameState);
  json += "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void updateGame() {
  if (gameState != 1) return;
  
  unsigned long currentTime = millis();
  
  // Move enemies
  if (currentTime - lastEnemyMove > ENEMY_MOVE_INTERVAL) {
    lastEnemyMove = currentTime;
    ledsDirty = true; // Enemies moved

    for (int i = 0; i < numEnemies; i++) {
      enemies[i].position--; // Move left
      
      // Check if enemy reached player
      if (enemies[i].position <= playerPos) {
        lives--;
        Serial.println("Enemy hit player! Lives: " + String(lives));
        if (lives <= 0) {
          gameState = 2; // Game over
        } else {
          // Remove the enemy that hit player
          for (int j = i; j < numEnemies - 1; j++) {
            enemies[j] = enemies[j + 1];
          }
          numEnemies--;

          // Clear the last slot to prevent ghost enemies
          enemies[numEnemies].position = -999;
          enemies[numEnemies].color = RED;

          i--; // Adjust index after removal
        }
      }
    }
  }
  
  // Move bullets
  if (currentTime - lastBulletMove > BULLET_MOVE_INTERVAL) {
    lastBulletMove = currentTime;

    // Process each bullet separately
    for (int bulletIndex = 0; bulletIndex < MAX_BULLETS; bulletIndex++) {
      if (!bulletActive[bulletIndex]) continue; // Skip inactive bullets

      bullets[bulletIndex]++; // Move right one position at a time through all LEDs
      ledsDirty = true; // Bullet moved

      // Check if this bullet hit any enemy
      bool hit = false;
      for (int enemyIndex = 0; enemyIndex < numEnemies; enemyIndex++) {
        // Skip invalid enemy positions (ghost prevention)
        if (enemies[enemyIndex].position < 0 || enemies[enemyIndex].position >= LED_COUNT) {
          continue;
        }

        if (bullets[bulletIndex] == enemies[enemyIndex].position) { // Exact position match for precise hit
          gameScore += 100;

          // Remove enemy by shifting array
          for (int j = enemyIndex; j < numEnemies - 1; j++) {
            enemies[j] = enemies[j + 1];
          }
          numEnemies--;

          // Clear the last slot to prevent ghost enemies
          enemies[numEnemies].position = -999;
          enemies[numEnemies].color = RED;

          // Deactivate this specific bullet
          bulletActive[bulletIndex] = false;

          hit = true;
          break; // Stop checking enemies for this bullet
        }
      }

      // Remove bullets that went off screen
      if (!hit && bullets[bulletIndex] >= LED_COUNT) {
        bulletActive[bulletIndex] = false;
      }
    }
  }
  
  // Add new enemy wave occasionally
  if (random(100) < 3 && numEnemies < 12) { // 3% chance per update
    createEnemyWave();
  }
}

void renderLEDs() {
  strip.clear();
  
  // Draw player (green)
  strip.setPixelColor(playerPos, strip.Color(0, 255, 0));
  
  // Draw enemies (red, green, blue)
  for (int i = 0; i < numEnemies; i++) {
    // Skip invalid enemy positions (ghost prevention)
    if (enemies[i].position < 0 || enemies[i].position >= LED_COUNT) {
      continue;
    }

    uint32_t color;
    switch(enemies[i].color) {
      case RED:    color = strip.Color(255, 0, 0); break;
      case GREEN:  color = strip.Color(0, 255, 0); break;
      case BLUE:   color = strip.Color(0, 0, 255); break;
      default:     color = strip.Color(255, 0, 0); break;
    }
    strip.setPixelColor(enemies[i].position, color);
  }
  
  // Draw active bullets (white)
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bulletActive[i] && bullets[i] >= 0 && bullets[i] < LED_COUNT) {
      strip.setPixelColor(bullets[i], strip.Color(255, 255, 255));
    }
  }
  
  strip.show();
}

int countActiveBullets() {
  int count = 0;
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bulletActive[i]) {
      count++;
    }
  }
  return count;
}

bool checkAuth() {
  #if ENABLE_WEB_AUTH
    if (!server.authenticate(WEB_USERNAME, WEB_PASSWORD)) {
      server.requestAuthentication();
      return false;
    }
  #endif
  return true;
}

void loop() {
  server.handleClient();
  updateGame();

  // Only update LEDs if game state changed
  if (ledsDirty) {
    renderLEDs();
    ledsDirty = false;
  }

  delay(GAME_UPDATE_DELAY);
}