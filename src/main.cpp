#include "raylib.h"
#include <bits/stdc++.h>
#include <cmath> //
#include <vector>

using namespace std;

float SCREEN_CENTER_X = 0.0f;
float SCREEN_CENTER_Y = 0.0f;

struct Point {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
};

struct Face {
  vector<Point> points;
  Point center = {0.0, 0.0, 0.0};
};

// Perspective: farther points (bigger z) shrink toward the screen center
Vector2 getVector(const Point &point) {
  const float f = 400.0f; // focal length (zoom)
  const float d =
      400.0f; // camera distance from z = 0 (f == d keeps z=0 at normal size)

  float scale = f / ((float)point.z + d);

  float screen_x = SCREEN_CENTER_X + (float)point.x * scale;
  float screen_y =
      SCREEN_CENTER_Y -
      (float)point.y * scale; // minus because raylib's y points down

  return {screen_x, screen_y};
}

// the shape class is fundamentally a list of faces, each being a list of points
class Shape {
private:
  string name;
  int id;
  vector<Face> faces; // a list of points
  bool selected;

public:
  // Constructor
  Shape(string shapeName, int id, bool selected)
      : name(shapeName), id(id), selected(selected) {}

  // Add an empty face and return its index
  int addFace() {
    faces.push_back(Face{});
    return (int)faces.size() - 1;
  }

  // Sort faces from highest average Z (farthest) to lowest average Z (closest)
  void sortFacesByZ() {
    sort(faces.begin(), faces.end(), [](const Face &a, const Face &b) {
      // Calculate average Z for face 'a'
      double sumA = 0.0;
      for (const auto &p : a.points)
        sumA += p.z;
      double avgA = a.points.empty() ? 0.0 : sumA / a.points.size();

      // Calculate average Z for face 'b'
      double sumB = 0.0;
      for (const auto &p : b.points)
        sumB += p.z;
      double avgB = b.points.empty() ? 0.0 : sumB / b.points.size();

      // Sort descending (highest Z first)
      return avgA > avgB;
    });
    getFaceCenter();
  }

  void getFaceCenter() {
    for (Face &face : faces) {
      Point sum = {0, 0, 0};
      int i = 0;
      for (Point &point : face.points) {
        i++;
        sum.x += point.x;
        sum.y += point.y;
        sum.z += point.z;
      }
      face.center.x = sum.x / i;
      face.center.y = sum.y / i;
      face.center.z = sum.z / i;
    }
  }

  // Add a single face
  void addPoint(const Face &f) { faces.push_back(f); }

  // Add a single point to the faces's list
  void addPoint(const Point &p, int face_index) {
    faces[face_index].points.push_back(p);
  }

  // Add a single point to a face using explicit coordinates
  void addPoint(double x, double y, double z, int face_index) {
    faces[face_index].points.push_back({x, y, z});
  }

  // Print every face and its points
  void printShapeDetails() const {
    cout << "Shape: " << name << "\n";
    cout << "Selected: " << selected << "\n";
    cout << "Number of faces: " << faces.size() << "\n";

    for (size_t i = 0; i < faces.size(); ++i) {
      cout << " Face " << i + 1 << ":\n";
      for (const Point &p : faces[i].points) {
        cout << " (" << p.x << ", " << p.y << ", " << p.z << ")\n";
      }
    }
  }

  Point get_shape_center() {
    float cx = 0, cy = 0, cz = 0;

    int i = 0;
    for (Face &face : faces) {
      for (Point &p : face.points) {
        cx += p.x;
        cy += p.y;
        cz += p.z;
        i++;
      }
    }
    cx /= i;
    cy /= i;
    cz /= i;
    return {cx, cy, cz};
  }

  // Rotates all vertices around the origin (0,0,0)
  void rotate(float angleX, float angleY) {
    Point center = get_shape_center();
    float radX = angleX * (PI / 180.0f);
    float radY = angleY * (PI / 180.0f);

    for (Face &face : faces) {
      for (Point &p : face.points) {

        float dx = p.x - center.x;
        float dy = p.y - center.y;
        float dz = p.z - center.z;

        // Y rotation
        double x1 = dx * cos(radY) + dz * sin(radY);
        double z1 = -dx * sin(radY) + dz * cos(radY);

        // X rotation
        double y2 = dy * cos(radX) - z1 * sin(radX);
        double z2 = dy * sin(radX) + z1 * cos(radX);

        p.x = x1 + center.x;
        p.y = y2 + center.y;
        p.z = z2 + center.z;
      }
    }
  }

  void changeS(int dir) {
    Point center = get_shape_center();
    float min_z = -350.0f; // closest to screen possible
    float max_z = 600.0f;
    float step = 0.0f;
    float max_step = 7.0f;

    if (dir == 1) {
      // Distance from center to the lower boundary
      float distance = center.z - min_z;
      if (distance <= 0.0f)
        return;

      // Calculate a uniform step size
      step = 10.0f * (distance / 100.0f);
      if (step < 0.1f)
        step = 0.1f;

      if (center.z - step < min_z)
        step = center.z - min_z;

      // Apply a uniform downward shift
      if (step > max_step) {
        step = max_step;
      }

      step = -step;
    } else {
      float distance = max_z - center.z;
      if (distance <= 0.0f)
        return;

      step = 10.0f * (distance / 100.0f);
      if (step < 0.1f)
        step = 0.1f;

      if (center.z + step > max_z)
        step = max_z - center.z;

      if (step > max_step) {
        step = max_step;
      }
    }

    // Apply the exact same translation to every point so it never distorts
    for (Face &face : faces) {
      for (Point &p : face.points) {
        p.z += step;
      }
    }
  }

  Color GetFaceColorByZ(const Face &face, Color baseColor, double closer_z,
                        double farther_z) {
    if (face.points.empty())
      return baseColor;

    // 1. Calculate average Z for the face
    double sumZ = 0.0;
    for (const auto &p : face.points) {
      sumZ += p.z;
    }
    double avgZ = sumZ / face.points.size();

    // 2. Clamp avgZ between closer_z and farther_z
    if (avgZ < min(closer_z, farther_z))
      avgZ = min(closer_z, farther_z);
    if (avgZ > max(closer_z, farther_z))
      avgZ = max(closer_z, farther_z);

    // 3. Calculate a percentage factor (0.0 = farthest/darkest, 1.0 =
    // closest/brightest)
    float factor = (float)((avgZ - farther_z) / (closer_z - farther_z));

    // 4. Scale RGB channels based on the factor (keep alpha at 255)
    return Color{(unsigned char)(baseColor.r * factor),
                 (unsigned char)(baseColor.g * factor),
                 (unsigned char)(baseColor.b * factor), baseColor.a};
  }

  void drawShape() {
    sortFacesByZ();

    double max_visible_depth = 150.0;
    double min_visible_depth = -50.0;

    for (const Face &face : faces) {
      int s = face.points.size();

      if (s < 3)
        continue;

      Color shadedColor = BLUE;
      // GetFaceColorByZ(face, BLUE, min_visible_depth, max_visible_depth);

      for (int i = 1; i < s - 1; ++i) {
        Vector2 v0 = getVector(face.points[0]);
        Vector2 v1 = getVector(face.points[i]);
        Vector2 v2 = getVector(face.points[i + 1]);

        DrawTriangle(v0, v1, v2, shadedColor);
        DrawTriangle(v0, v2, v1, shadedColor);
      }
      for (int i = 0; i < s; i++) {
        Vector2 v0 = getVector(face.points[i]);
        Vector2 v1 = getVector(face.points[(i + 1) % s]);

        DrawLineV(v0, v1, WHITE);
      }
    }
  }
};

int main() {

  /*
  // Lines & points
  DrawPixel(int x, int y, Color c)
  DrawLine(int x1, int y1, int x2, int y2, Color c)
  -DrawLineV(Vector2 start, Vector2 end, Color c)
  -DrawLineEx(Vector2 start, Vector2 end, float thickness, Color c)

  // Triangles (vertices must be counter-clockwise or it draws nothing)
  -DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color c) //
  filled DrawTriangleLines(Vector2 v1, Vector2 v2, Vector2 v3, Color c) //
  outline

  // Other shapes
  DrawCircle(int x, int y, float radius, Color c)
  DrawCircleV(Vector2 center, float radius, Color c)
  DrawCircleLines(int x, int y, float radius, Color c)
  DrawRectangle(int x, int y, int w, int h, Color c)
  DrawRectangleLines(int x, int y, int w, int h, Color c)
  DrawPoly(Vector2 center, int sides, float radius, float rotation, Color
  c)

  // Text
  DrawText(const char* text, int x, int y, int fontSize, Color c)
  DrawFPS(int x, int y)
  TextFormat("x: %f", x) // printf-style, returns a string for DrawText

  // Colors
  BLACK, WHITE, RAYWHITE, GRAY, LIGHTGRAY, DARKGRAY, RED, MAROON,
  ORANGE, YELLOW, GOLD, GREEN, LIME, DARKGREEN, SKYBLUE, BLUE, DARKBLUE,
  PURPLE, VIOLET, PINK, MAGENTA, BROWN, BEIGE

  // Timing & screen
  GetFrameTime() // gives seconds since the last frame. Multiply your
  rotation speed by it. GetTime() // gives seconds since start.
  GetScreenWidth() / GetScreenHeight()

  Extra
  GetMousePosition()
  */

  int width = 1000;
  int height = 1000;
  vector<Shape> shapes;
  InitWindow(width, height, "title");
  SetTargetFPS(60);

  SCREEN_CENTER_X = (float)GetScreenWidth() / 2;
  SCREEN_CENTER_Y = (float)GetScreenHeight() / 2;

  // Create a Cube shape
  Shape shape("Cube", 1, false);
  shapes.push_back(shape);
  int f = shape.addFace(); // front (z = -25)
  shape.addPoint(-25.0, -25.0, -25.0, f);
  shape.addPoint(25.0, -25.0, -25.0, f);
  shape.addPoint(25.0, 25.0, -25.0, f);
  shape.addPoint(-25.0, 25.0, -25.0, f);

  int a = shape.addFace(); // back (z = 25)
  shape.addPoint(-25.0, -25.0, 25.0, a);
  shape.addPoint(25.0, -25.0, 25.0, a);
  shape.addPoint(25.0, 25.0, 25.0, a);
  shape.addPoint(-25.0, 25.0, 25.0, a);

  int f1 = shape.addFace(); // top (y = 25)
  shape.addPoint(-25.0, 25.0, -25.0, f1);
  shape.addPoint(25.0, 25.0, -25.0, f1);
  shape.addPoint(25.0, 25.0, 25.0, f1);
  shape.addPoint(-25.0, 25.0, 25.0, f1);

  int f2 = shape.addFace(); // right (x = 25)
  shape.addPoint(25.0, -25.0, -25.0, f2);
  shape.addPoint(25.0, 25.0, -25.0, f2);
  shape.addPoint(25.0, 25.0, 25.0, f2);
  shape.addPoint(25.0, -25.0, 25.0, f2);

  int f3 = shape.addFace(); // left (x = -25)
  shape.addPoint(-25.0, -25.0, -25.0, f3);
  shape.addPoint(-25.0, 25.0, -25.0, f3);
  shape.addPoint(-25.0, 25.0, 25.0, f3);
  shape.addPoint(-25.0, -25.0, 25.0, f3);

  int f4 = shape.addFace(); // bottom (y = -25)
  shape.addPoint(-25.0, -25.0, -25.0, f4);
  shape.addPoint(25.0, -25.0, -25.0, f4);
  shape.addPoint(25.0, -25.0, 25.0, f4);
  shape.addPoint(-25.0, -25.0, 25.0, f4);

  // Display the shape data
  shape.printShapeDetails();

  float dX = 0.0f,
        dY = 0.0f; // declare OUTSIDE the loop so they keep their value

  while (!WindowShouldClose()) {
    // 1. update
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();
      // Pass the changes directly to rotate the matrix coordinates
      // delta.y rotates around X-axis; delta.x rotates around Y-axis
      shape.rotate(delta.y * -0.5f, delta.x * -0.5f);
    }
    if (float wheelMove = GetMouseWheelMove(); wheelMove != 0.0f) {
      // Pass the changes directly to your scroll or position logic
      // Negating wheelMove ensures scrolling down moves content up
      float scrollDelta = wheelMove * 35.0f;

      int d = 1 ? scrollDelta > 0 : -1;

      shape.changeS(d);
    }

    // 2. draw
    BeginDrawing();
    ClearBackground(BLACK);
    shape.drawShape();
    EndDrawing();
  }
  CloseWindow();
  return 0;
}