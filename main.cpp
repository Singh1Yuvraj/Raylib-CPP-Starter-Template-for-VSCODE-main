#include <raylib.h>
#include <cmath>
#include <iostream>
#include <vector>

Color DarkGreen = Color{0, 100, 0, 255};  // Dark green for the background
Color TreeCanopyColor = Color{34, 139, 34, 255};  // Green for the tree canopy
Color TreeTrunkColor = Color{139, 69, 19, 255};  // Brown for the tree trunk
Color Yellow = Color{243, 213, 91, 255};  // Ball color
Color PowerBarColor = Color{255, 0, 0, 255};  // Red for power bar
Color BoundaryColor = Color{255, 255, 255, 255};  // White for the boundary line

class Ball {
public:
    Vector2 position;
    Vector2 velocity;
    int radius;
    bool isMoving;
    bool isBeingDragged;

    Ball(float x, float y, int r) : position{(float)x, (float)y}, velocity{0, 0}, radius{r}, isMoving(false), isBeingDragged(false) {}

    void Draw() {
        DrawCircle(position.x, position.y, radius, Yellow);
    }

    void Update(const std::vector<Rectangle>& obstacles) {
        if (isMoving) {
            position.x += velocity.x;
            position.y += velocity.y;

            // Apply friction to stop the ball over time
            velocity.x *= 0.98;
            velocity.y *= 0.98;

            // Stop the ball when velocity is very low
            if (fabs(velocity.x) < 0.1 && fabs(velocity.y) < 0.1) {
                velocity.x = 0;
                velocity.y = 0;
                isMoving = false;
            }

            // Keep the ball within screen boundaries
            if (position.x - radius <= 0 || position.x + radius >= GetScreenWidth()) {
                velocity.x *= -1;
            }
            if (position.y - radius <= 0 || position.y + radius >= GetScreenHeight()) {
                velocity.y *= -1;
            }

            // Handle obstacle collisions
            for (const auto& obstacle : obstacles) {
                if (CheckCollisionCircleRec(position, radius, obstacle)) {
                    // Simple bounce logic: invert velocity upon collision
                    velocity.x *= -1;
                    velocity.y *= -1;
                }
            }
        }
    }
};

class Hole {
public:
    Vector2 position;
    int radius;

    Hole(float x, float y, int r) : position{(float)x, (float)y}, radius{r} {}

    void Draw() {
        DrawCircle(position.x, position.y, radius, BLACK);
    }

    bool CheckIfScored(Ball& ball) {
        return CheckCollisionCircles(position, radius, ball.position, ball.radius);
    }

    // Function to change hole position to a random location
    void ChangePosition() {
        position.x = GetRandomValue(100, GetScreenWidth() - 100);
        position.y = GetRandomValue(100, GetScreenHeight() - 100);
    }
};

int score = 0;
int holeCount = 1;
bool dragging = false;
Vector2 dragStartPos;
float power = 0.0f;
const float maxPower = 50.0f;  // Maximum power limit

// Function to draw a tree at a specific position
void DrawTree(float x, float y) {
    // Tree canopy (circle)
    DrawCircle(x, y, 30, TreeCanopyColor);

    // Tree trunk (rectangle)
    DrawRectangle(x - 10, y + 30, 20, 50, TreeTrunkColor);
}

int main() {
    std::cout << "Starting Golf Game with Obstacles and Trees" << std::endl;

    const int screen_width = 1280;
    const int screen_height = 720;
    InitWindow(screen_width, screen_height, "Golf Game with Obstacles and Trees");
    SetTargetFPS(60);

    Ball ball(screen_width / 4, screen_height / 2, 15);
    Hole hole(3 * screen_width / 4, screen_height / 2, 20);

    // Define obstacles (rectangles)
    std::vector<Rectangle> obstacles = {
        {400, 300, 200, 20},  // Horizontal obstacle
        {600, 500, 20, 200},  // Vertical obstacle
    };

    // Draw some trees at random positions (reduced number of trees)
    std::vector<Vector2> treePositions = {
        {200, 500},
        {800, 150},
    };

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        // Check for mouse press to start dragging
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ball.isMoving) {
            if (CheckCollisionCircleRec(mousePos, 5, {ball.position.x - ball.radius, ball.position.y - ball.radius, ball.radius * 2, ball.radius * 2})) {
                dragging = true;
                dragStartPos = mousePos;
                power = 0.0f; // Reset power
                ball.isBeingDragged = true;
            }
        }

        // While dragging, update the visual direction of the shot
        if (dragging) {
            // Draw an arrow indicating the drag direction and length
            Vector2 direction = {mousePos.x - dragStartPos.x, mousePos.y - dragStartPos.y};
            float length = sqrt(direction.x * direction.x + direction.y * direction.y);

            // Update power bar based on drag length
            power = length / 10.0f;
            if (power > maxPower) power = maxPower; // Cap the power at maxPower

            // Draw the aiming arrow while dragging, now in front of the ball
            if (length > 5) {
                direction.x /= length;
                direction.y /= length;
                
                // Draw arrow from the ball's position towards the drag end (in front of the ball)
                Vector2 arrowEnd = {ball.position.x + direction.x * 50, ball.position.y + direction.y * 50};  // 50 is the length of the aiming line
                DrawLineEx(ball.position, arrowEnd, 5, WHITE);  // Draw arrow from ball to aim direction
            }
        }

        // When mouse button is released, apply the shot
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragging) {
            Vector2 direction = {mousePos.x - dragStartPos.x, mousePos.y - dragStartPos.y};
            float length = sqrt(direction.x * direction.x + direction.y * direction.y);

            if (length > 10) { // Avoid zero-length shots
                // Apply the shot with intensity based on drag distance
                direction.x /= length;
                direction.y /= length;
                ball.velocity.x = direction.x * length / 10; // Adjust the multiplier for strength
                ball.velocity.y = direction.y * length / 10;
                ball.isMoving = true;  // Start moving the ball
            }

            dragging = false;
            ball.isBeingDragged = false;
        }

        // Update ball position and handle obstacles
        ball.Update(obstacles);

        // Check if ball enters the hole
        if (hole.CheckIfScored(ball)) {
            score++;
            holeCount++;  // Move to next hole
            hole.ChangePosition();  // Change hole position after each goal
            ball.position = {screen_width / 4, screen_height / 2};  // Reset ball position
            ball.velocity = {0, 0};  // Stop the ball
        }

        // Draw everything
        BeginDrawing();
        ClearBackground(DarkGreen);  // Use Dark Green as the background for the golf course

        // Draw the boundary line around the screen
        DrawRectangleLines(0, 0, screen_width, screen_height, BoundaryColor);  // Boundary line

        // Draw hole and ball
        hole.Draw();
        ball.Draw();

        // Draw obstacles
        for (const auto& obstacle : obstacles) {
            DrawRectangleRec(obstacle, DARKGRAY);  // Obstacle color
        }

        // Draw trees
        for (const auto& treePos : treePositions) {
            DrawTree(treePos.x, treePos.y);
        }

        // Draw Power Bar
        DrawRectangle(20, screen_height - 40, 300, 20, DARKGRAY);
        DrawRectangle(20, screen_height - 40, (int)(power * 300 / maxPower), 20, PowerBarColor);

        // Display Score and Hole Count
        DrawText(TextFormat("Score: %d", score), 20, 20, 30, WHITE);
        DrawText(TextFormat("Hole: %d", holeCount), 20, 60, 30, WHITE);

        if (ball.isBeingDragged) {
            DrawText("Drag to aim", 20, 100, 30, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
