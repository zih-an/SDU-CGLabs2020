/*INTRODUCTION
// Mouse: 
//	- LEFT button: 
//		1. draw a new line
//		2. choose the endpoint to modify a line
//	- RIGHT button:
//		clip the lines
// Keyboard:
//	- SPACE:
//		clear the window
//  - DELETE:
//      keep the clipped lines(black ones) stay, remove the redundant parts
//      this operation also erase these unclipped lines
*/

#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>
#include<cmath>
using namespace std;

// hide the console window
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )

#define POS_P 0.5f  //clipping window pos.
#define POS_N -0.5f  // clipping window pos.
#define WIDTH 960.f  // width of main window
#define HEIGHT 600.f  // height of main window

struct Color {
	float r, g, b;
	Color(float _r, float _g, float _b)
		:r(_r), g(_g), b(_b) { }
} YELLOW(1, 1, 0), RED(1, 0, 0), BLACK(0.2, 0.2, 0.2);

struct Point {
	double x, y;
	Point() {}
	Point(double a, double b)
		:x(a), y(b) {}
};
vector<Point> points;  // drawing/processing points
vector<vector<Point> > Line, clipLine;  // original lines and cliped lines
bool Modifying = false;  // is modifying lines


// To choose the end-point: dis<=0.03
bool near(double x1, double y1, double x2, double y2) {
	// calculate the distance to judge whether chhoose the end-point or not
	double dis = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	if (dis <= 0.03) return true;
	return false;
}

// Draw line with 2 points and color
void drawLine(double x1, double y1, double x2, double y2, Color c) {
	// Draw A Line
	glBegin(GL_LINES);
	glColor3f(c.r, c.g, c.b);    // color: YELLOW
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

// Pressing left button to draw/modify a line with position of cursordynamically
void leftButtonOP(double x, double y) {
	int s = points.size();
	// transfer the coordinates
	x = (x - WIDTH / 2) / WIDTH * 2;
	y = (HEIGHT / 2 - y) / HEIGHT * 2;

	// move the line: when the line is existed and choose the end-points
	for (int i = 0; !Modifying && i < Line.size(); i++) {
		Point tmp;
		bool start_near = near(x, y, Line[i][0].x, Line[i][0].y);
		bool end_near = near(x, y, Line[i][1].x, Line[i][1].y);

		// move the start point
		if (!Modifying && start_near)
			tmp.x = Line[i][1].x, tmp.y = Line[i][1].y;
		// move the end point
		else if (!Modifying && end_near)
			tmp.x = Line[i][0].x, tmp.y = Line[i][0].y;

		if (start_near || end_near) { // can move: modify the coresponding point
			Line.erase(Line.begin() + i);  // earse the old line
			clipLine.clear();
			points.push_back(tmp);  // add the start point; not drawing, tmp is the first point
			break;
		}
	}

	// preserve the drawing points
	points.push_back(Point(x, y));
	Modifying = true;  // drawing new line or moving a line
}

// Get the position of cursor, and process with it
void cursorPosCallBack(GLFWwindow* window, double x, double y) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == 1) { 
		// Press the left button: add or modify line 
		leftButtonOP(x, y);
	}
}

// Display the lines 
void displayLine() {
	// display modifying line dynamically
	if (!points.empty()) {
		int s = points.size();
		double x1 = points[0].x, y1 = points[0].y;
		double x2 = points[s - 1].x, y2 = points[s - 1].y;
		// display the painting line
		drawLine(x1, y1, x2, y2, RED);
	}

	// display the certain lines
	for (int i = 0; i < Line.size(); i++)
		drawLine(Line[i][0].x, Line[i][0].y, Line[i][1].x, Line[i][1].y, YELLOW);
	// display the clip lines
	for (int i = 0; i < clipLine.size(); i++)
		drawLine(clipLine[i][0].x, clipLine[i][0].y, clipLine[i][1].x, clipLine[i][1].y, BLACK);
}

// Liang-Barsky algorithm
bool ClipT(double p, double q, double &u1, double &u2) {
	double r = q / p;
	if (p < 0) {
		if (r > u2) return false;
		if (r > u1) u1 = r;
	}
	else if (p > 0) {
		if (r < u1) return false;
		if (r < u2) u2 = r;
	}
	else return (q >= 0);
	return true;
}
void liang_barsky() {
	double XR = POS_P, XL = POS_N, YT = POS_P, YB = POS_N;
	// for each edges
	for (int i = 0; i < Line.size(); i++) {
		double x1 = Line[i][0].x, y1 = Line[i][0].y;
		double x2 = Line[i][1].x, y2 = Line[i][1].y;
		double u1 = 0, u2 = 1;
		double dx = x2 - x1, dy = y2 - y1;

		if (ClipT(-dx, x1 - XL, u1, u2)) {
			if (ClipT(dx, XR - x1, u1, u2)) {
				if (ClipT(-dy, y1 - YB, u1, u2)) {
					if (ClipT(dy, YT - y1, u1, u2)) {
						double x1_tmp = x1 + u1 * dx, y1_tmp = y1 + u1 * dy;
						double x2_tmp = x1 + u2 * dx, y2_tmp = y1 + u2 * dy;
						vector<Point> tmp;
						tmp.push_back(Point(x1_tmp, y1_tmp));
						tmp.push_back(Point(x2_tmp, y2_tmp));
						clipLine.push_back(tmp);
					}
				}
			}
		}

	}
}

// Handle with the mouse button input
void getMouseButton(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		/* 
		// need to do sth. with the position of cursor:
		// handled by the (cursorPosCallBack) function
		*/
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		Modifying = false;  // stop modifying the end-point
		if (!points.empty()) {  // just finish drawing lines
			int s = points.size();
			// update the lines: keep old; add new
			vector<Point> tmp;
			tmp.push_back(points[0]);
			tmp.push_back(points[s - 1]);
			Line.push_back(tmp);

			points.clear();
		}
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		// Colip the lines
		liang_barsky();
	}
}

// Handle with the keyboard input
void key_CallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		// Press space to clear all lines in the window
		Line.clear();
		clipLine.clear();
		points.clear();
	}
	else if (key == GLFW_KEY_DELETE && action == GLFW_PRESS) {
		// keep the clipped lines stay, erase the redundant parts
		Line.clear();
		Line.assign(clipLine.begin(), clipLine.end());
		//copy(clipLine.begin(), clipLine.end(), Line.begin());
	}
}

int main(void) {
	GLFWwindow* window;
	glfwInit();  // initialize the library
	window = glfwCreateWindow(WIDTH, HEIGHT, "Liang-Barsky Algorithm", NULL, NULL);  // create a window
	glfwSetWindowPos(window, 600, 200);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glLineWidth(3);

		// Draw a rectangle
		glColor3f(0.95f, 0.95f, 0.95f);    // color: WHITE
		glRectf(POS_N, POS_N, POS_P, POS_P);

		// mouse
		glfwSetCursorPosCallback(window, cursorPosCallBack);  // Cursor
		glfwSetMouseButtonCallback(window, getMouseButton);  // Button

		// Display current line dynamically
		displayLine();

		// keyboard
		glfwSetKeyCallback(window, key_CallBack);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

