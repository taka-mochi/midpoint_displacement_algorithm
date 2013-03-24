
#include <list>
#include <vector>
#include <random>
#include <set>
#include <map>
#include <cmath>
#include <ctime>
#include <memory>
#include "GL\glut.h"

// n: 分割数を増やす
// p: 分割数を減らす
// ←→: カメラ回転

using namespace std;


// 3次元座標を表すデータ
struct pointf {
	double x,y,z;
	pointf():x(0),y(0),z(0){}
	pointf(double x_, double y_):x(x_), y(y_), z(0){}
	pointf(double x_, double y_, double z_):x(x_), y(y_), z(z_){}
};

// 4頂点によって示されるポリゴン
struct quad_mesh {
	shared_ptr<pointf> points[4];
	pointf normal;
	
	quad_mesh()
		: normal(0, 1, 0) {
		points[0] = points[1] = points[2] = points[3] = NULL;
	}

	// 現在のpointsから法線情報を計算する。points[0-2]が張る平面上にpoints[3]が乗っているという仮定のもと計算
	void calculateNormal() {
		double a11 = this->points[0]->x;
		double a12 = this->points[0]->y;
		double a13 = this->points[0]->z;
		double a21 = this->points[1]->x;
		double a22 = this->points[1]->y;
		double a23 = this->points[1]->z;
		double a31 = this->points[2]->x;
		double a32 = this->points[2]->y;
		double a33 = this->points[2]->z;
		double det = a11*a22*a33+a21*a32*a13+a31*a12*23 - a11*a32*a23-a31*a22*a13-a21*a12*a33;

		double nx, ny, nz;
		if (fabs(det)<0.01) {
			nx = nz = 0;
			ny = 1;
		} else {
			nx = (a22*a33 - a23*a32 + a13*a32 - a12*a33 + a12*a23 - a13*a22);
			ny = (a23*a31 - a21*a33 + a11*a33 - a13*a31 + a13*a21 - a11*a23);
			nz = (a21*a32 - a22*a31 + a12*a31 - a11*a32 + a11*a22 - a12*a21);
			double sqrtsum = sqrt(nx*nx+ny*ny+nz*nz);
			nx /= sqrtsum;
			ny /= sqrtsum;
			nz /= sqrtsum;
			if (ny<0) {
				nx = -nx;
				nz = -nz;
				ny = -ny;
			}
		}
		normal.x = nx;
		normal.y = ny;
		normal.z = nz;
	}
};

// 中点変位法のパラメータ
double sigma_val = 1;
int div_count = 0;

std::vector<quad_mesh> mesh_list;

// カメラ、ビューポート、ライティングのパラメータ
GLfloat light0pos[] = { 0.0, 5.0, 5.0, 1.0 };	// ライトの位置 (ポイントライト)
GLfloat green[] = { 0.0, 1.0, 1.0, 1.0 };		// ライトの色 (緑)
GLfloat cameraPos[] = {6.0, 8.0, 10.0};
int width = 750, height = 600;

// generate a random value according to uniform distribution
double rand_uniform()
{
	return ((double)rand()+1.0)/((double)RAND_MAX+2.0);
}

// generate a random value according to normal distribution
double rand_normal(double mu, double sigma)
{
	double z = sqrt(-2.0*log(rand_uniform())) * sin(2.0*3.141592*rand_uniform());
	return mu+sigma*z;
}


void create_new_meshes_by_midpoint_displacement_algorithm()
{

	map<shared_ptr<pointf>, map<shared_ptr<pointf>, shared_ptr<pointf> > > processed_points;	// processed_points[p1][p2] => p1とp2の2点から中点変位法によって生成されたポイントを示す

	vector<quad_mesh> new_meshes;
	vector<quad_mesh>::iterator it,end = mesh_list.end();

	// initialization
	for (it=mesh_list.begin(); it!=end; it++)
	{
		for (int i=0; i<4; i++)
		{
			processed_points[it->points[i]][it->points[(i+1)%4]] = NULL;
			processed_points[it->points[(i+1)%4]][it->points[i]] = NULL;
		}
	}

	// すべてのメッシュに対して中点変位法の実行
	for (it=mesh_list.begin(); it!=end; it++)
	{
		shared_ptr<pointf> new_points[4];

		// 新しい頂点の生成
		for (int i=0; i<4; i++)
		{
			shared_ptr<pointf> p1 = it->points[i];
			shared_ptr<pointf> p2 = it->points[(i+1)%4];

			if (processed_points[p1][p2]) {
				// 既にこの2点からは新しい頂点が生成されていた
				new_points[i] = processed_points[p1][p2];
			} else {
				// 新しい頂点を生成
				new_points[i] = shared_ptr<pointf>(new pointf((p1->x+p2->x)/2, (p1->y+p2->y)/2 + rand_normal(0, sigma_val)*pow(2, -sigma_val), (p1->z+p2->z)/2));
				processed_points[p1][p2] = new_points[i];
				processed_points[p2][p1] = new_points[i];
			}
		}

		// 分割によって生成された4点の中心点を計算
		shared_ptr<pointf> center_point(new pointf((new_points[0]->x+new_points[1]->x+new_points[2]->x+new_points[3]->x)/4, 
			(new_points[0]->y+new_points[1]->y+new_points[2]->y+new_points[3]->y)/4,
			(new_points[0]->z+new_points[1]->z+new_points[2]->z+new_points[3]->z)/4));

		quad_mesh new_mesh;

		// 生成された頂点から新しいメッシュを生成
		new_mesh.points[0] = new_points[0];
		new_mesh.points[1] = it->points[1];
		new_mesh.points[2] = new_points[1];
		new_mesh.points[3] = center_point;
		new_mesh.calculateNormal();
		new_meshes.push_back(new_mesh);

		new_mesh.points[0] = new_points[1];
		new_mesh.points[1] = it->points[2];
		new_mesh.points[2] = new_points[2];
		new_mesh.points[3] = center_point;
		new_mesh.calculateNormal();
		new_meshes.push_back(new_mesh);

		new_mesh.points[0] = new_points[2];
		new_mesh.points[1] = it->points[3];
		new_mesh.points[2] = new_points[3];
		new_mesh.points[3] = center_point;
		new_mesh.calculateNormal();
		new_meshes.push_back(new_mesh);

		it->points[1] = new_points[0];
		it->points[2] = center_point;
		it->points[3] = new_points[3];
		it->calculateNormal();

	}

	for (it=new_meshes.begin(); it!=new_meshes.end(); it++)
	{
		mesh_list.push_back(*it);
	}

	sigma_val /= 2.0;
}

void combine_meshes()
{
	int decreased_count = mesh_list.size()/4;

	int offset = decreased_count;

	for (int i=0; i<decreased_count; i++) {
		// 隣接している4つのメッシュを取得
		quad_mesh &m1 = mesh_list.at(i);
		quad_mesh &m2 = mesh_list.at(i+offset);
		quad_mesh &m3 = mesh_list.at(i+offset+1);
		quad_mesh &m4 = mesh_list.at(i+offset+2);

		// 隣接している4つのメッシュから、分割前のメッシュを復元するために必要な頂点を修得
		m1.points[1] = m2.points[1];
		m1.points[2] = m3.points[1];
		m1.points[3] = m4.points[1];
		m1.calculateNormal();

		offset += 2;
	}

	sigma_val *= 2;
	mesh_list.resize(decreased_count);
}

void setViewportMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, (double)width / (double)height, 1.0, 100.0);
	gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}



// 描画関数
void display()
{
	setViewportMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLightfv(GL_LIGHT0, GL_POSITION, light0pos);

	glBegin(GL_QUADS);
	for (unsigned int i=0; i<mesh_list.size(); i++)
	{
		const quad_mesh &mesh = mesh_list.at(i);
		for (int j=0; j<4; j++)
		{
			glColor3d(0,0,0);
			glNormal3d(mesh.normal.x, mesh.normal.y, mesh.normal.z);
			glVertex3d(mesh.points[j]->x, mesh.points[j]->y, mesh.points[j]->z);
		}
	}
	glEnd();

	glFlush();
}

void update_window_title()
{
	char str[1024];
	sprintf(str, "Fractale mountain - Push N: divide, Push P: combine - Div count: %d, Quad polygon count: %d", div_count, mesh_list.size());
	glutSetWindowTitle(str);
}

void rot_camera(double rot_rad)
{
	double c = cos(rot_rad);
	double s = sin(rot_rad);
	double x = c*cameraPos[0] - s*cameraPos[2];
	double z = s*cameraPos[0] + c*cameraPos[2];

	cameraPos[0] = x;
	cameraPos[2] = z;
}

void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
	// next
	case 'n':
	case 'N':
		create_new_meshes_by_midpoint_displacement_algorithm();
		div_count++;
		update_window_title();
		glutPostRedisplay();
		break;

	// prev
	case 'p':
	case 'P':
		if (mesh_list.size()<4) break;
		combine_meshes();
		div_count--;
		update_window_title();
		glutPostRedisplay();
		break;
	}
}

void keyboardSpecial(int key, int x, int y)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		rot_camera(15.0*3.141592/180);
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		rot_camera(-15.0*3.141592/180);
		glutPostRedisplay();
		break;
	}
}

void resize(int w, int h)
{
	width = w;
	height = h;
	setViewportMatrix();
}

// initialization
void init(void)
{
	glClearColor(1.0, 1.0, 1.0f, 1.0f);

	// push a default mesh
	quad_mesh base_mesh;
	base_mesh.points[0] = shared_ptr<pointf>(new pointf(-3, 0, -3));
	base_mesh.points[1] = shared_ptr<pointf>(new pointf(-3, 0, 3));
	base_mesh.points[2] = shared_ptr<pointf>(new pointf(3, 0, 3));
	base_mesh.points[3] = shared_ptr<pointf>(new pointf(3, 0, -3));
	mesh_list.push_back(base_mesh);
	
	// gl settings
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, green);
	glLightfv(GL_LIGHT0, GL_SPECULAR, green);

	srand((unsigned)time(NULL));
}

/***
 * Program Entry Point
 ***/
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowPosition(200,200);
	glutInitWindowSize(width, height);
	glutCreateWindow(argv[0]);
	glutDisplayFunc(display);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	init();
	update_window_title();
	glutMainLoop();

	return 0;
}
