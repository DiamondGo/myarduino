#include <Arduino.h>
#include <Mouse.h>

const int BUTTON_PIN = 2;
const int BUTTON_PUSH_INTERVAL = 500; // millisecond
const int LOOP_DELAY = 20;
const int MOUSE_MOVE_DISTANCE = 200; // pixel
const int MOUSE_MOVE_SPEED = 1000;   // millisecond

enum MouseMode
{
    Horizontal = 0,
    Vertical = 1,
    Clockwise = 2,
    Counterclockwise = 3,
    ModeMax
};

void setupLed()
{
    pinMode(LED_BUILTIN, OUTPUT);
}

void setupButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.begin(9600);
}

void setupAutoMouse()
{
    Mouse.begin();
}

void setup()
{
    // put your setup code here, to run once:
    setupLed();
    setupButton();
}

void turnLed(bool on)
{
    if (on)
    {
        digitalWrite(LED_BUILTIN, HIGH);
    }
    else
    {
        digitalWrite(LED_BUILTIN, LOW);
    }
}

bool isButtonPushed()
{
    static unsigned long lastTimePushed = 0;
    int buttonStatus = digitalRead(BUTTON_PIN);
    if (buttonStatus == LOW)
    {
        unsigned long currentTime = millis();
        if (currentTime - lastTimePushed >= BUTTON_PUSH_INTERVAL || currentTime < lastTimePushed)
        {
            lastTimePushed = currentTime;
            return true;
        }
    }

    return false;
}

/*
 * Move mouse in a straight line.
 */
void moveMouseStraight(bool on, bool horizontal)
{
    static int delta = int(float(MOUSE_MOVE_DISTANCE) / (float(MOUSE_MOVE_SPEED) / float(LOOP_DELAY)));
    static int x = 0;
    static int y = 0;
    static bool positiveDirection = true;

    if (!on)
    {
        return;
    }

    int xdelta = horizontal ? delta : 0;
    int ydelta = horizontal ? 0 : delta;

    if (positiveDirection)
    {
        Mouse.move(xdelta, ydelta, 0);
        x += xdelta;
        y += ydelta;
        if ((horizontal && x >= MOUSE_MOVE_DISTANCE) || (!horizontal && y >= MOUSE_MOVE_DISTANCE))
        {
            positiveDirection = false;
        }
    }
    else
    {
        Mouse.move(0 - xdelta, 0 - ydelta, 0);
        x -= xdelta;
        y -= ydelta;
        if ((horizontal && x <= 0) || (!horizontal && y <= 0))
        {
            positiveDirection = true;
        }
    }
}

void moveMouseCircle(bool on, bool clockwise)
{
    const float deltaAngle = float(360) / (float(MOUSE_MOVE_SPEED) / float(LOOP_DELAY));
    const int radius = MOUSE_MOVE_DISTANCE / 4;
    static float angle = 0.0;
    static int x = radius;
    static int y = 0;

    if (!on)
    {
        return;
    }

    angle += clockwise ? 0 - deltaAngle : deltaAngle;
    int newX = int(cos(angle) * float(radius));
    int newY = int(sin(angle) * float(radius));

    Mouse.move(newX - x, newY - y, 0);
    x = newX;
    y = newY;
}

void loop()
{
    // put your main code here, to run repeatedly:
    static bool on = true;
    static MouseMode mouseMode = Horizontal;
    static bool once = true;

    if (once)
    {
        once = false;
        turnLed(on);
    }

    if (isButtonPushed())
    {
        on = !on;
        turnLed(on);
        if (on)
        {
            mouseMode = MouseMode((mouseMode + 1) % ModeMax);
        }
    }

    switch (mouseMode)
    {
    case Horizontal:
        moveMouseStraight(on, true);
        break;
    case Vertical:
        moveMouseStraight(on, false);
        break;
    case Clockwise:
        moveMouseCircle(on, true);
        break;
    case Counterclockwise:
        moveMouseCircle(on, false);
        break;
    }

    delay(LOOP_DELAY);
}
