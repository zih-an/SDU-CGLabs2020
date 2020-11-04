/*INTRODUCTION
// Mouse:
//	- LEFT button:
//		1. add/move a control points
//		2. with keyboard DELETE: delete control points
//	- RIGHT button:
//		1. PRESS: select a control points
//		2. RELEASE: add a control point in front of the selected point
// Keyboard:
//  - DELETE:
//      with mouse LEFT button: delete control points
//  - SPACE:
//		compeletely clear the window
//  - ENTER:
//		clear the control points, but remain the beizer curve
//  - numbers: 1--9
//		modify k(degree) directly
//	- INSERT: use the console to input higher degree
//		must use the console
//
//	- CTRL + 1/2/3/4
//		select type of knot-vector: 
//			CTRL+1: uniform
//			CTRL+2: open uniform
//			CTRL+3: bezier
//			CTRL+3: closed curve
*/
#include <GLFW/glfw3.h>
#include<cmath>
#include<vector>
#include<iostream>
using namespace std;

#define WIDTH 900.f
#define HEIGHT 600.f


struct Color {
	double r, g, b;
	Color() { }
	Color(double x, double y, double z)
		:r(x), g(y), b(z) { }
} YELLOW(1, 1, 0), RED(1, 0, 0), BLACK(0.3, 0.3, 0.3);

struct Point {
	double x, y;
	Point() { }
	Point(double a, double b)
		:x(a), y(b) { }

	// scaler * Point
	Point operator* (const double &t) {
		return Point(t*this->x, t*this->y);
	}
	// Point + Point
	Point operator+ (const Point &p) {
		return Point(this->x + p.x, this->y + p.y);
	}
};
vector<Point> points, moving_points, bspline_points;  // drawing/moving/curve
vector<Point>::iterator moveIter, insertIter = points.begin();

vector<double> T;  // knot vector [0,..,tn+k]
int knotV = 2;  // type of knot vector: 1-uniform; 2-open uniform; 3-bezier; default-2
int k = 3;  // b-spline degree: 1-point; 2-line; >=3 curve
bool DEL = false, MOVE = false, CLS = false, INS = false;


// To choose the end-point: dis<=10
bool close_to(double x1, double y1, double x2, double y2) {
	// calculate the distance to judge whether chhoose the end-point or not
	double dis = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
	if (dis <= 10) return true;
	return false;
}

// draw controls
void drawControls(Color c) {
	int beginMode[] = { GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP };
	for (int mode = 0; mode < 2; mode++) {
		// set points size
		if (mode == 0) glPointSize(9);
		// draw points and lines
		if (mode==1 && CLS) mode = 2;  // closed curve
		glBegin(beginMode[mode]);
		glColor3f(c.r, c.g, c.b);
		for (auto iter = points.begin(); iter != points.end(); iter++) {
			double xpos, ypos;
			if (MOVE && !moving_points.empty() && iter == moveIter) {
				xpos = moving_points[moving_points.size() - 1].x;
				ypos = moving_points[moving_points.size() - 1].y;
			}
			else xpos = iter->x, ypos = iter->y;

			xpos = (xpos - WIDTH / 2) / WIDTH * 2;
			ypos = (HEIGHT / 2 - ypos) / HEIGHT * 2;
			glVertex2f(xpos, ypos);
		}
		glEnd();
	}
}
// draw b-spline curve 
void drawCurve(Color c) {
	glPointSize(2);
	glBegin(GL_POINTS);
	glColor3f(c.r, c.g, c.b);
	for (auto p : bspline_points) {
		double x = (p.x - WIDTH / 2) / WIDTH * 2;
		double y = (HEIGHT / 2 - p.y) / HEIGHT * 2;
		glVertex2f(x, y);
	}
	glEnd();
}


// knot vector
inline void uniform_KnotVector(int n) {
	T.clear();
	for (int i = 0; i <= n + k; i++)
		T.push_back(i);
}
inline void openUniform_KnotVector(int n) {
	T.clear();
	double bgn = 0;
	T.insert(T.begin(), k, bgn);
	for (bgn = 1; bgn <= n + 1 - k; bgn++)
		T.push_back(bgn);
	T.insert(T.end(), k, bgn);
}
inline void bezier_KnotVector(int n) {
	T.clear();
	double bgn = 0;
	T.insert(T.begin(), k, bgn);
	bgn++;
	for (int i = 1; i <= n + 1 - k; ) {
		if (k > 1) {
			T.insert(T.end(), k - 1, bgn++);
			i += k - 1;
		}
		else {
			T.push_back(bgn++);
			i++;
		}
	}
	T.insert(T.end(), k, bgn);
}

// de Boor algorithm
void deBoor(int j, double t, vector<Point> wrapPoints) {
	vector<Point> deBo = wrapPoints;
	for (int r = 1; r <= k - 1; r++) {
		for (int i = j; i >= j - k + r + 1; i--) {
			double n1 = (t - T[i]) / (T[i + k - r] - T[i]);
			double n2 = (T[i + k - r] - t) / (T[i + k - r] - T[i]);
			deBo[i] = (deBo[i] * n1) + (deBo[i - 1] * n2);
		}
	}
	bspline_points.push_back(deBo[j]);
}
void bSpline() {
	int n = points.size() - 1;
	if (k < 1) return;  // even can't draw a point
	bspline_points.clear();

	// prepare for closed curve
	int p = k; 
	vector<Point> wrapPoints = points;

	/* generate knot vector here */
	if (knotV == 1) uniform_KnotVector(n);
	else if (knotV == 2) openUniform_KnotVector(n);
	else if(knotV == 3) bezier_KnotVector(n);
	else {
		int p = k + 1;
		if (n < k) p = n;
		// compute closed curve
		
		wrapPoints.insert(wrapPoints.end(), wrapPoints.begin(), wrapPoints.begin() + p);
		n = wrapPoints.size() - 1;
		uniform_KnotVector(n);
	}
	
	// t <- [tj, tj+1)  # k-1<=j<=n
	for (int j = k - 1; j <= n; j++)
		for (double t = T[j]; t < T[j + 1]; t += 0.001)
			deBoor(j, t, wrapPoints);

}


// move points here
void leftButtonOp(double x, double y) {
	for (auto iter = points.begin(); !DEL && !MOVE&&iter != points.end(); iter++) {
		if (close_to(iter->x, iter->y, x, y)) {
			MOVE = true;
			moveIter = iter;
			break;
		}
	}
	// add moving points
	if (MOVE)
		moving_points.emplace_back(x, y);
}

// cursor pos callback
void cursorPosCallback(GLFWwindow *window, double x, double y) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == 1) {
		// move control point
		leftButtonOp(x, y);
	}
}
// mouse button callback
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	if (DEL && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		for (auto iter = points.begin(); iter != points.end(); iter++) {
			if (close_to(iter->x, iter->y, xpos, ypos)) {
				points.erase(iter);
				break;
			}
		}
		// recompute b-spline curves
		bSpline();
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		// modify point
		if (MOVE) {
			moveIter->x = xpos;
			moveIter->y = ypos;
		}
		// add point
		else if (!MOVE && !DEL) {
			points.emplace_back(xpos, ypos);
		}

		MOVE = false;
		moving_points.clear();
		// recompute curve
		bSpline();
	}
	
	// insert node
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		for (insertIter = points.begin(); insertIter != points.end(); insertIter++) {
			if (close_to(xpos, ypos, insertIter->x, insertIter->y)) {
				INS = true;
				break;
			}
		}
	}
	else if (INS && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		points.insert(insertIter, Point(xpos, ypos));
		INS = false;
		bSpline();
	}
}
// key callback
void keyCallback(GLFWwindow *window, int key, int sancode, int action, int mods) {
	if (key == GLFW_KEY_DELETE && action == GLFW_PRESS) {
		DEL = true;
	}
	else if (key == GLFW_KEY_DELETE && action == GLFW_RELEASE) {
		DEL = false;
	}
	// remain the curve
	else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		points.clear();
		moving_points.clear();
		MOVE = DEL = false;
	}
	// clear the window completely
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		points.clear();
		moving_points.clear();
		bspline_points.clear();
		MOVE = DEL = false;
	}
	// input degree k
	else if (key == GLFW_KEY_INSERT && action == GLFW_PRESS) {
		cout << "input k:  ";
		cin >> k;
		bSpline();
	}
	// change knot-vector
	else if (mods == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_1:
			CLS = false;
			knotV = 1;
			bSpline();
			break;
		case GLFW_KEY_2:
			CLS = false;
			knotV = 2;
			bSpline();
			break;
		case GLFW_KEY_3:
			CLS = false;
			knotV = 3;
			bSpline();
			break;
		case GLFW_KEY_4:
			CLS = true;
			knotV = 4;
			bSpline();
			break;
		default:
			break;
		}
	}
	// change degree -> single
	else if (action == GLFW_PRESS) {
		switch (key)
		{
		case GLFW_KEY_1:
			k = 1;
			bSpline();
			break;
		case GLFW_KEY_2:
			k = 2;
			bSpline();
			break;
		case GLFW_KEY_3:
			k = 3;
			bSpline();
			break;
		case GLFW_KEY_4:
			k = 4;
			bSpline();
			break;
		case GLFW_KEY_5:
			k = 5;
			bSpline();
			break;
		case GLFW_KEY_6:
			k = 6;
			bSpline();
			break;
		case GLFW_KEY_7:
			k = 7;
			bSpline();
			break;
		case GLFW_KEY_8:
			k = 8;
			bSpline();
			break;
		case GLFW_KEY_9:
			k = 9;
			bSpline();
			break;
		default:
			break;
		}
	}
}



int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(WIDTH, HEIGHT, "B-Spline", NULL, NULL);
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

		// mouse 
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSetCursorPosCallback(window, cursorPosCallback);

		// key
		glfwSetKeyCallback(window, keyCallback);

		// draw here
		drawControls(YELLOW);
		if (!MOVE) {
			drawCurve(RED);
		}



		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

