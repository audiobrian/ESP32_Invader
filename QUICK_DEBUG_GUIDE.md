# Quick Debugging Guide

## Test Collision Detection

### 1. Add Logging (10 minutes)
Edit `src/main.cpp` line 385, add debug print:

```cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 3) {
    Serial.printf("HIT! Bullet=%d Enemy=%d Distance=%d\n", 
                  bullets[bulletIndex], 
                  enemies[enemyIndex].position,
                  abs(bullets[bulletIndex] - enemies[enemyIndex].position));
    gameScore += 100;
    // ... rest of code
}
```

Upload and monitor:
```bash
pio run -e esp32dev --target upload && pio device monitor
```

### 2. Try Wider Tolerance (5 minutes)
Change line 385 from `<= 3` to `<= 5`:
```cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 5) {
```

### 3. Slow Down Bullets (5 minutes)
Change line 374 from `+= 3` to `+= 1`:
```cpp
bullets[bulletIndex] += 1;  // Was += 3
```

And tighten collision at line 385:
```cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 1) {
```

### 4. More Frequent Bullet Updates (2 minutes)
Change line 12 from 200 to 100:
```cpp
#define BULLET_MOVE_INTERVAL  100  // Was 200
```

## Watch Serial Output

```bash
pio device monitor --filter direct | grep -E "HIT|Enemy|Bullet"
```

## Test Single Shot

```bash
# Restart game
curl http://192.168.0.109/restart

# Wait 10 seconds for enemy to move close
sleep 10

# Fire one bullet
curl http://192.168.0.109/shoot

# Watch result for 10 seconds
for i in {1..10}; do
    curl -s http://192.168.0.109/state | grep -oP '"(enemies|bullets|score)":\K\d+'
    sleep 1
done
```

## Expected Behavior

Good collision:
```
Bullet=1 Enemy=150 Distance=1 → HIT
enemies=15 bullets=0 score=100
```

Bad collision (miss):
```
Bullet=148 Enemy=150 Distance=2 → NO HIT
enemies=15 bullets=0 score=0  ← Bullet gone, no kill
```

## If Nothing Works

The issue may be:
1. **Race condition** between bullet movement and collision check
2. **Array shifting** invalidating loop index
3. **Timing** - bullet update happens before/after enemy position check

Consider rewriting collision detection to use a different approach (see COLLISION_BUG_REPORT.md).
