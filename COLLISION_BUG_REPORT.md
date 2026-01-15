# Collision Detection Bug Report

## Problem Statement
Bullets inconsistently kill enemies - sometimes they work, sometimes they don't. The collision detection is unreliable.

## Original User Report
"If shoot once, and wait to the most closed enemy to be killed, it works. Then I shoot again, and the shoot does not kill the next enemy and disappears just on the same place where the last killed enemy were."

## Root Cause Analysis

### Bullet Movement Speed vs Collision Detection
- **Bullets move**: 3 LEDs per update (`bullets[bulletIndex] += 3;` at line 374)
- **Collision checks**: Every 200ms (`BULLET_MOVE_INTERVAL`)
- **Enemy positions**: Change every 2000ms (`ENEMY_MOVE_INTERVAL`)

### Attempted Fix
Changed collision detection tolerance from `< 2` to `<= 3` to match bullet speed:
```cpp
// Line 385 in src/main.cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 3) {
```

### Test Results
Automated testing showed bullets killing enemies, but user reports inconsistent behavior:
- Sometimes kills: ✓
- Sometimes misses: ✗

## Potential Remaining Issues

### 1. Timing Race Condition
Bullets and enemies update at different rates:
- Bullets: every 200ms
- Enemies: every 2000ms

If enemy moves between bullet position updates, collision window may be missed.

### 2. Collision Window Too Small
Current tolerance is ±3, but with both objects moving:
- Bullet moves +3 right
- Enemy moves -1 left (same cycle)
- Combined movement could be 4 positions

### 3. Array Shifting During Collision Check
When enemy is removed, array shifts while loop is running:
```cpp
// Line 389-391
for (int j = enemyIndex; j < numEnemies - 1; j++) {
    enemies[j] = enemies[j + 1];
}
```

## Recommended Next Steps

### Debug Strategy
1. **Add position logging**:
```cpp
if (bulletActive[bulletIndex]) {
    Serial.printf("Bullet %d at pos %d\n", bulletIndex, bullets[bulletIndex]);
}
```

2. **Log collision attempts**:
```cpp
int distance = abs(bullets[bulletIndex] - enemies[enemyIndex].position);
if (distance <= 5) { // Wider logging window
    Serial.printf("Near miss: bullet=%d enemy=%d distance=%d\n",
                  bullets[bulletIndex], enemies[enemyIndex].position, distance);
}
```

3. **Watch for edge cases**:
   - Bullet at position 167, enemy at 165 (distance = 2, should hit)
   - Bullet at position 50, enemy at 48 (distance = 2, should hit)
   - Multiple enemies clustered together

### Possible Fixes to Try

#### Option 1: Increase Collision Tolerance
```cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 5) {
```

#### Option 2: Check Previous Position
Track last bullet position and check range:
```cpp
int lastPos = bullets[bulletIndex] - 3; // Where bullet was
int currentPos = bullets[bulletIndex];  // Where bullet is now
int enemyPos = enemies[enemyIndex].position;

if (enemyPos >= lastPos && enemyPos <= currentPos) {
    // Enemy is in path of bullet travel
}
```

#### Option 3: Slow Down Bullets
Change bullet speed from 3 to 1:
```cpp
bullets[bulletIndex] += 1; // Line 374
```
And adjust collision tolerance:
```cpp
if (abs(bullets[bulletIndex] - enemies[enemyIndex].position) <= 1) {
```

#### Option 4: Synchronize Update Rates
Make bullets update more frequently:
```cpp
#define BULLET_MOVE_INTERVAL  100  // Was 200ms
```

## Code References

### Main collision detection logic:
`src/main.cpp:366-411` - Bullet movement and collision detection

### Key variables:
- `BULLET_MOVE_INTERVAL` (line 12): 200ms
- `ENEMY_MOVE_INTERVAL` (line 11): 2000ms
- Bullet speed (line 374): `+= 3`
- Collision tolerance (line 385): `<= 3`

## Current State
- Code compiled and flashed successfully
- Game running at http://192.168.0.109/
- Collision detection unreliable (inconsistent hits)
- All debug logging removed

## Testing Commands

### Monitor serial output:
```bash
pio device monitor --filter direct
```

### Test via web API:
```bash
# Fire bullet
curl http://192.168.0.109/shoot

# Check game state
curl http://192.168.0.109/state

# Restart game
curl http://192.168.0.109/restart
```

### Watch live game state:
```bash
watch -n 1 'curl -s http://192.168.0.109/state'
```

## Notes
- User reports cost concerns - keep debugging minimal
- Issue is intermittent, suggesting timing/race condition
- Physical LED strip observation needed to confirm behavior
- Automated testing showed success but doesn't match user experience
