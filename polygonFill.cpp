#include <GLFW/glfw3.h>
#include<iostream>
#include<vector>

#define WIDTH 960.f  // width of main window
#define HEIGHT 600.f  // height of main window
using namespace std;

const int MAXN = 600;  // >= HEIGHT, 扫描线

// hide the console window
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )



struct Color {
	double r, g, b;
	Color() { }
	Color(double x,double y,double z)
		:r(x), g(y), b(z) { }
} YELLOW(1, 1, 0), RED(1, 0.3, 0.2), BLACK(0.3, 0.3, 0.3);

struct Point {
	double x, y;
	Point() { }
	Point(double a, double b)
		:x(a), y(b) { }
};
vector<Point> points;

// edge table
typedef struct ET {
	double x, ymax, m;  // m = 1/k
	ET *next;
	ET() { 
		x = ymax = m = 0;
		next = NULL;
	}
	ET(double a, double b, double c)
		:x(a), ymax(b), m(c) {
		next = NULL;
	}
} NET, AET;  // next edge table, active edge table

NET *pNET[MAXN + 10];  // i=0, y=600

// draw
void drawPolygon(Color c) {
	glBegin(GL_LINE_LOOP);
	glColor3f(c.r, c.g, c.b);
	for (auto point : points) {
		double xt = (point.x - WIDTH / 2) / WIDTH * 2;
		double yt = (HEIGHT / 2 - point.y) / HEIGHT * 2;
		glVertex2f(xt, yt);
	}
	glEnd();
}
void draw(double x, double y, Color c) {
	// transform
	double xt = (x - WIDTH / 2) / WIDTH * 2;
	double yt = (HEIGHT / 2 - y) / HEIGHT * 2;
	//cout << xt << " " << yt << endl;
	glBegin(GL_POINTS);
	glColor3f(c.r, c.g, c.b);
	glVertex2f(xt, yt);
	glEnd();
}

class AetLink {
private:
	AET *root;
	int size;
public:
	AetLink() {
		root = NULL;
		size = 0;
	}

	void insert(AET *node) {
		size++;
		AET *newNode = new AET(node->x, node->ymax, node->m);
		if (root == NULL) {
			root = newNode;
			return;
		}
		if (root->x > newNode->x) {
			newNode->next = root;
			root = newNode;
			return;
		}

		AET *tmp = root;
		while (tmp->next != NULL && tmp->next->x < newNode->x)
			tmp = tmp->next;
		newNode->next = tmp->next;
		tmp->next = newNode;
	}
	void refresh(int idx) {
		if (root == NULL) return;
		AET *tmp = root, *pp = NULL;
		while (tmp != NULL) {
			tmp->x -= tmp->m;  // refresh x
			// erase
			if (tmp->ymax == idx) {
				if (pp == NULL)
					root = tmp->next;
				else
					pp->next = tmp->next;
				size--;
			}
			else pp = tmp;
			tmp = tmp->next;
		}
	}
	void pairDraw(double y, Color color) {
		AET *from = root;
		while (from != NULL) {
			AET *to = from->next;
			for (double i = from->x; i <= to->x; i++)
				draw(i, y, color);
			//
			from = to->next;
		}
	}
	void clear() {
		root = NULL;
		size = 0;
	}
	void sortBubble() {
		if (root == NULL || root->next == NULL) return;

		AET *p = root->next, *pstart = new AET, *pend = root;
		pstart->next = root;
		while (p != NULL)
		{
			AET *tmp = pstart->next, *pre = pstart;
			while (tmp != p && p->x >= tmp->x)
			{
				tmp = tmp->next; pre = pre->next;
			}
			if (tmp == p)pend = p;
			else
			{
				pend->next = p->next;
				p->next = tmp;
				pre->next = p;
			}
			p = pend->next;
		}
		root = pstart->next;
		delete pstart;
	}
	// for test
	void show() {
		AET *tt = root;
		while (tt != NULL) {
			cout << tt->x << " " << tt->ymax << " " << tt->m << endl;
			tt = tt->next;
		}
	}
} Aet;


//1. build pNET   top -> bottom
void netBuild() {
	for (int i = MAXN; i >= 0; i--) {
		for (int j = 0; j < points.size(); j++) {
			if (points[j].y == i) {
				//
				Point p1, p2;
				if (j == 0) p1 = points[points.size() - 1];
				else p1 = points[j - 1];
				if (j == points.size() - 1) p2 = points[0];
				else p2 = points[j + 1];

				double x = points[j].x;
				// each point: connect 2 lines; record >y (in opengl <y )
				if (p1.y < i) {
					double ymax = p1.y;
					double m = double(p1.x - points[j].x) / (p1.y - points[j].y);

					NET *tmp = new NET(x, ymax, m);
					tmp->next = pNET[i];
					pNET[i] = tmp;
				}
				if (p2.y < i) {
					double ymax = p2.y;
					double m = double(p2.x - points[j].x) / (p2.y - points[j].y);

					NET *tmp = new NET(x, ymax, m);
					tmp->next = pNET[i];
					pNET[i] = tmp;
				}

				/*
				NET *ttt = pNET[i];
				cout << "###: " << i << endl;
				while (ttt != NULL) {
					cout << ttt->x << " " << ttt->ymax << " ::" << ttt->m << endl;
					ttt = ttt->next;
				}*/
			}
		}
	}
}

// 2. build pAET and fill:    bottom -> top
void fill() {
	for (int i = MAXN; i >= 0; i--) {
		// 1. isnert&refresh AET
		NET *tmp = pNET[i];
		while (tmp != NULL) {
			Aet.insert(tmp);
			tmp = tmp->next;
		}
		// resort  -> after refresh
		Aet.sortBubble();
		
		// 2. refresh
		Aet.refresh(i);
		// 3. make pairs and fill in the color
		Aet.pairDraw(i, RED);
	}
}

bool FILL = false;


// mouse callback
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		points.emplace_back(xpos, ypos);  // add point
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		// clear first
		Aet.clear();
		FILL = false;
		for (int i = 0; i <= MAXN; i++)
			pNET[i] = NULL;
		// fill in the polygon
		netBuild();
		FILL = true;
	}
}

// keyboard callback
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		points.clear();
		Aet.clear();
		FILL = false;
		for (int i = 0; i <= MAXN; i++)
			pNET[i] = NULL;
	}
}



int main(void)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(WIDTH, HEIGHT, "Polygon Fill", NULL, NULL);
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

		// keyboard
		glfwSetKeyCallback(window, keyCallback);

		// drawing 
		drawPolygon(YELLOW);  // show the outline
		if (FILL) fill();  // show 
		

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

