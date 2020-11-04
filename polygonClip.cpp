/*INTRODUCTION
// Mouse:
//	- LEFT button:
//		1. add point to draw the polygon
//	- RIGHT button:
//		clip the polygon
// Keyboard:
//	- SPACE:
//		clear the window
*/
#include <GLFW/glfw3.h>
#include<iostream>
#include<vector>

#define POS_P 0.5f  // rectangle window pos.
#define POS_N -0.5f  // rectangle window pos.

#define WIDTH 960.f  // width of main window
#define HEIGHT 600.f  // height of main window
using namespace std;

// hide the console window
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )


struct Color {
	double r, g, b;
	Color() { }
	Color(double x, double y, double z)
		:r(x), g(y), b(z) { }
} YELLOW(1, 1, 0), RED(1, 0, 0), BLACK(0.2, 0.2, 0.2);

struct Point {
	double x, y;
	Point() { }
	Point(double a, double b)
		:x(a), y(b) { }
};

vector<Point> points;  // polygon's points


// draw polygon
void drawPolygon(Color C) {
	glBegin(GL_LINE_LOOP);
	glColor3f(C.r, C.g, C.b);
	for (auto p : points) {
		glVertex2f(p.x, p.y);
	}
	glEnd();
}

// judge the relationship between the point and the boundary
bool Inside(Point p, int boundary) {
	switch (boundary) {
	case 0: // left
		if (p.x >= POS_N) return true;
		break;
	case 1: // top
		if (p.y <= POS_P) return true;
		break;
	case 2: // right
		if (p.x <= POS_P) return true;
		break;
	case 3: // bottom
		if (p.y >= POS_N) return true;
		break;
	default:
		return false;
	}
	return false;
}

// get the intersection point
Point intersect(Point &S, Point &P, int boundary) {
	Point tmp;
	double dy = P.y - S.y, dx = P.x - S.x;
	// left
	if (boundary == 0) {
		tmp.x = POS_N;
		tmp.y = S.y + (POS_N - S.x)*(dy / dx);
	}
	// top
	else if (boundary == 1) {
		tmp.y = POS_P;
		tmp.x = S.x + (POS_P - S.y)*(dx / dy);
	}
	// right
	else if (boundary == 2) {
		tmp.x = POS_P;
		tmp.y = S.y + (POS_P - S.x)*(dy / dx);
	}
	// bottom
	else {
		tmp.y = POS_N;
		tmp.x = S.x + (POS_N - S.y)*(dx / dy);
	}
	return tmp;
}

// clip the polygon (sutherland-hodgman algorithm)
void sutherland_hodgman() {
	vector<Point> clipPoints = points;
	// for each window's boundary 
	for (int boundary = 0; boundary < 4; boundary++) {
		vector<Point> remainPoints;  // remaining points
		// each line
		for (int i = 0; i < clipPoints.size(); i++) {
			// S(start point) and P(end point)
			Point S, P;
			S = clipPoints[i];
			if (i == clipPoints.size() - 1) P = clipPoints[0];
			else P = clipPoints[i + 1];

			//
			if (Inside(S, boundary)) {
				// situation 1: remain the P(end point)
				if (Inside(P, boundary)) {
					remainPoints.push_back(P);
				}
				// situation 3: remain the mid of S-P
				else {
					Point tmp = intersect(S, P, boundary);
					remainPoints.push_back(tmp);
				}
			}
			else {
				//situation 4: remain the mid of S-P and P
				if (Inside(P, boundary)) {
					Point tmp = intersect(S, P, boundary);
					remainPoints.push_back(tmp);
					remainPoints.push_back(P);
				}
				// situation 2: remain nothing
			}
		}
		clipPoints = remainPoints;
	}
	points = clipPoints;
}


// mouse callback
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	// draw the points of the polygon
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		// transform the coordinates
		xpos = (xpos - WIDTH / 2) / WIDTH * 2;
		ypos = (HEIGHT / 2 - ypos) / HEIGHT * 2;
		points.emplace_back(xpos, ypos);
	}
	// clip the polygon
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		sutherland_hodgman();
	}
}

// keyboard callback
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	// clear the window
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		points.clear();
	}
}




int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(WIDTH, HEIGHT, "PolygonClip", NULL, NULL);
	glfwSetWindowPos(window, 600, 200);

	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glLineWidth(3);

		// Draw a rectangle(imitate the window)
		glColor3f(0.95f, 0.95f, 0.95f);    // color: WHITE
		glRectf(POS_N, POS_N, POS_P, POS_P);


		// mouse
		glfwSetMouseButtonCallback(window, mouseButtonCallback);

		// keyboard
		glfwSetKeyCallback(window, keyCallback);

		// draw the polygon(only lines, don't filled with color)
		drawPolygon(RED);



		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

