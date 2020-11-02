#pragma once

#define LNC_PI 3.14159265358979323846
#define LNC_RADIANS(deg) (deg * LNC_PI / 180.0)

#include "Vec3.h"
#include "Mat4.h"

class Camera {
    Vec3 position;
    Vec3 direction;
    Vec3 right, up;
    Vec3 worldUp;

    float yaw{ -90.0f };
    float pitch{ 0.0f };

    float moveSpeed;
    bool movingFront{ false };
    bool movingBack{ false };
    bool movingLeft{ false };
    bool movingRight{ false };

    float mouseSens;
    float mouseX{ -1.0f }, mouseY{ -1.0f };
    bool mouseEnabled{ false };

    void updateDirection() {
        float dirX = cos(LNC_RADIANS(yaw)) * cos(LNC_RADIANS(pitch));
        float dirY = sin(LNC_RADIANS(pitch));
        float dirZ = sin(LNC_RADIANS(yaw)) * cos(LNC_RADIANS(pitch));
        direction = Vec3::normalize(Vec3(dirX, dirY, dirZ));
        right     = Vec3::normalize(Vec3::cross(direction, worldUp));
        up        = Vec3::normalize(Vec3::cross(right, direction));
    }

public:

    Camera(Vec3 position, float moveSpeed = 0.015f, float mouseSens = 0.15f, Vec3 worldUp = Vec3(0.0f, 1.0f, 0.0f)) {
        this->position = position;
        this->moveSpeed = moveSpeed;
        this->mouseSens = mouseSens;
        this->worldUp = worldUp;
        updateDirection();
    }

    void moveFront(bool b) { movingFront = b; }

    void moveBack(bool b) { movingBack = b; }

    void moveRight(bool b) { movingRight = b; }

    void moveLeft(bool b) { movingLeft = b; }

    void enableMouse() { mouseEnabled = true; }

    void disableMouse() {
        mouseEnabled = false;
        mouseX = mouseY = -1.0f;
    }

    void mouseMove(float xPosition, float yPosition) {
        if (!mouseEnabled) {
            return;
        }
        else if (mouseX == -1.0f && mouseY == -1.0f) {
            mouseX = xPosition;
            mouseY = yPosition;
            return;
        }

        float xOffset = (xPosition - mouseX) * mouseSens;
        float yOffset = (mouseY - yPosition) * mouseSens;

        mouseX = xPosition;
        mouseY = yPosition;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f) pitch = 89.0f;
        else if (pitch < -89.0f) pitch = -89.0f;

        updateDirection();
    }

    void updatePosition() {
        if (movingFront) position = position + (direction * moveSpeed);
        if (movingBack)  position = position - (direction * moveSpeed);
        if (movingRight) position = position + (right * moveSpeed);
        if (movingLeft)  position = position - (right * moveSpeed);
    }

    Mat4 viewMatrix() {
        return Mat4::lookAt(position, position + direction, up);
    }
};
