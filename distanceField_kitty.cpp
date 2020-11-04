#pragma warning(disable:4996)

#include<iostream>
#include<igl/readOBJ.h>
#include<igl/opengl/glfw/Viewer.h>
#include<igl/png/readPNG.h>
#include <igl/unproject_onto_mesh.h>
// distance field algorithm
#include<stdafx.h>
#include<geodesic_mesh.h>
#include<geodesic_algorithm_exact.h>

// #define INF 0x3f3f3f

igl::opengl::glfw::Viewer viewer;
Eigen::MatrixXd V;
Eigen::MatrixXi F;
char file_name[] = "kitten_simplified.obj";

// compute distance
void update_distance(const int vid) {
	// computing used
	std::vector<double> points;
	std::vector<unsigned> faces;
	std::vector<int> realIndex;
	int originalVertNum = 0;
	int source_vertex_index = vid;

	std::cout << "Computing geodesic distance to vertex " << vid << "..." << std::endl;
	bool success = geodesic::read_mesh_from_file(file_name, points, faces, realIndex, originalVertNum);
	// Build Mesh
	geodesic::Mesh mesh;
	mesh.initialize_mesh_data(points, faces);		//create internal mesh data structure including edges
	geodesic::GeodesicAlgorithmExact algorithm(&mesh);
	// Propagation
	algorithm.propagate(source_vertex_index);	//cover the whole mesh
	// Output Geodesic Distances
	Eigen::VectorXd d(mesh.vertices().size());
	for (unsigned i = 0; i < mesh.vertices().size(); ++i)
	{
		double distance = mesh.vertices()[i].geodesic_distance();
		d[i] = distance;
	}

	// Allocate temporary buffers
	Eigen::Matrix<unsigned char, Eigen::Dynamic, Eigen::Dynamic> R, G, B, A;
	// Read the PNG
	igl::png::readPNG("MyColorBar2.png", R, G, B, A);
	// Plot the mesh
	viewer.data().clear();
	viewer.data().set_mesh(V, F);
	viewer.core().align_camera_center(V);
	viewer.data().show_texture = true;
	// Use the image as a texture
	viewer.data().set_texture(R, G, B);
	// Set the distances
	viewer.data().set_data(d);
}
// mouse callback
bool mouse_down(igl::opengl::glfw::Viewer& viewer, int button, int modifier) {
	int fid;
	Eigen::Vector3f bc;
	// Cast a ray in the view direction starting from the mouse position
	double x = viewer.current_mouse_x;
	double y = viewer.core().viewport(3) - viewer.current_mouse_y;

	if (igl::unproject_onto_mesh( Eigen::Vector2f(x, y), 
		viewer.core().view, viewer.core().proj, viewer.core().viewport,
		V, F, fid, bc) ) 
	{
		int max;
		bc.maxCoeff(&max);
		int vid = F(fid, max);
		update_distance(vid);
		return true;
	}
	return false;
}



int main(int argc, char *argv[]) {
	igl::readOBJ(file_name, V, F);

	// Plot the mesh
	viewer.callback_mouse_down = &mouse_down;
	viewer.data().set_mesh(V, F);
	viewer.data().show_lines = false;

	viewer.launch();

	return 0;
}

