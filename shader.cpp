#include "shader.h"

constexpr double MY_PI = 3.1415926;

PhongShader::PhongShader(){
    M_model.identity();
    M_view.identity();
    M_projection.identity();
}

void PhongShader::set_view_matrix(Vector3f eye_pos){
    //camera transformations: M_view = R_view * T_view
    // Matrix4f Mview;
	Vector3f up(0, 1, 0);
	Matrix4f Rview; Rview.identity();
	Vector3f z = eye_pos; z.normalize();
	Vector3f x = up.cross(z).normalize();
	Vector3f y = z.cross(x).normalize();
	Rview << std::vector<float>{
			x[0], x[1], x[2], 0,
			y[0], y[1], y[2], 0,
			z[0], z[1], z[2], 0,
			0, 0, 0, 1
	};
	
	Matrix4f Tview;
	Tview <<
		std::vector<float>{
			1, 0, 0, -eye_pos[0],
			0, 1, 0, -eye_pos[1],
			0, 0, 1, -eye_pos[2],
			0, 0, 0, 1
	};
	M_view = Rview * Tview;
	// return Mview;
}

void PhongShader::set_viewport_matrix(float w, float h){
    M_viewport <<
		std::vector<float>{
		    w/2.f, 0, 0, w/2.f,
			0, h/2.f, 0, h/2.f,
			0, 0, 1, 0,
			0, 0, 0, 1
	};
}

void PhongShader::set_model_matrix(char n, float rotation_angle){
    // Matrix4f model; model.identity();
	float angle = rotation_angle * MY_PI / 180.0f;
	if (n == 'X')
	{
		M_model << std::vector<float>{
				1, 0, 0, 0,
				0, std::cos(angle), -std::sin(angle), 0,
				0, std::sin(angle), std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Y')
	{
		M_model << std::vector<float>{
				std::cos(angle), 0, std::sin(angle), 0,
				0, 1, 0, 0,
				-std::sin(angle), 0, std::cos(angle), 0,
				0, 0, 0, 1
		};
	}
	else if (n == 'Z')
	{
		M_model << std::vector<float>{
				std::cos(angle), -std::sin(angle), 0, 0,
				std::sin(angle), std::cos(angle), 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1
		};
	}
	// return model;
}

Matrix3f v_dot_vT(const Vector3f& n)
{
	Matrix3f n_T;
	n_T << std::vector<float>{
		n.x, n.y, n.z,
			0, 0, 0,
			0, 0, 0
	};
	Matrix3f tmp;
	tmp.m[0][0] = n.x;
	tmp.m[1][0] = n.y;
	tmp.m[2][0] = n.z;
	tmp *= n_T;
	return tmp;
}

// 运用罗德里格斯旋转公式
void PhongShader::set_random_model_matrix(Vector3f n, float rotation_angle){
    // Matrix4f model; model.identity();
    float angle = rotation_angle * MY_PI / 180.;
    float nx = n[0], ny = n[1], nz = n[2];
    Matrix3f N;
    N << std::vector<float>{
                    0., -nz, ny,
                    nz, 0., -nx,
                    -ny, nx, 0.
    };
    Matrix3f I; I.identity();

    Matrix3f R = I * std::cos(angle) + v_dot_vT(n) * (1 - std::cos(angle)) + N * std::sin(angle); 
    M_model << std::vector<float>{
                    R(0, 0), R(0, 1), R(0, 2), 0, 
                    R(1, 0), R(1, 1), R(1, 2), 0,
                    R(2, 0), R(2, 1), R(2, 2), 0,
                    0, 0, 0, 1
    };
    // return model;
}

void PhongShader::set_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)
{
	Matrix4f M_trans;
	Matrix4f M_persp;
	Matrix4f M_ortho;
	M_persp << std::vector<float>{
		zNear, 0, 0, 0,
			0, zNear, 0, 0,
			0, 0, zNear + zFar, -zFar * zNear,
			0, 0, 1, 0
	};

	float alpha = 0.5 * eye_fov * MY_PI / 180.0f;
	float yTop = -zNear * std::tan(alpha); //
	float yBottom = -yTop;
	float xRight = yTop * aspect_ratio;
	float xLeft = -xRight;

	M_trans << std::vector < float>{
		1, 0, 0, -(xLeft + xRight) / 2,
			0, 1, 0, -(yTop + yBottom) / 2,
			0, 0, 1, -(zNear + zFar) / 2,
			0, 0, 0, 1
	};

	M_ortho << std::vector < float>{
		2 / (xRight - xLeft), 0, 0, 0,
			0, 2 / (yTop - yBottom), 0, 0,
			0, 0, 2 / (zNear - zFar), 0,
			0, 0, 0, 1
	};

	M_ortho = M_ortho * M_trans;
	M_projection = M_ortho * M_persp * M_projection;
}

// void PhongShader::set_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar){
//      Matrix4f Ms, Mt, Mp2o; //, projection; projection.identity();
//     // looking at -z
//     zNear = -zNear;
//     zFar = -zFar;
//     float l, r, t, b;
//     // zNear and zFar are positive. Set t -> negetive, get the right cow; 
//     // that means the up and down are reversed because of t and b direction.
//     float alpha = 0.5 * eye_fov * MY_PI / 180.;
//     t = -tan(alpha) * zNear;
//     b = -t;
//     r = aspect_ratio * t;
//     l = -r;    
//     Ms << std::vector<float>{
//         2 / (r-l), 0., 0., 0.,
//         0., 2 / (t - b), 0., 0.,
//         0., 0., 2 / (zNear - zFar), 0.,
//         0., 0., 0., 1.
//     };
//     Mt << std::vector<float>{
//         1., 0., 0., -(r + l) / 2,
//         0., 1., 0., -(b + t) / 2,
//         0., 0., 1., -(zNear + zFar) / 2,
//         0., 0., 0., 1.
//     };
//     Mp2o << std::vector<float>{
//         zNear, 0., 0., 0.,
//         0., zNear, 0., 0., 
//         0., 0., zNear+zFar, -zNear*zFar,
//         0., 0., 1., 0.
//     };

//     // 对z进行取反, 从模型前方看去
//     Matrix4f z;
//     z << std::vector<float>{
//         1, 0, 0, 0,
//         0, 1, 0, 0,
//         0, 0, -1, 0,
//         0, 0, 0, 1
//     };

//     M_projection = z * Ms * Mt * Mp2o; // * projection;
//     // projection = Ms * Mt * Mp2o * projection;
//     // return projection;
// }

Vector3f PhongShader::get_viewport(Vector4f& v, const int& width, const int& height){
    v = M_viewport * v;
    // 透视除法，将原视锥转换到[-1, 1]的NDC空间 （Games101 里貌似没提，但作业框架里有） 
    return Vector3f(v.x/v.w, v.y/v.w, v.z); // 注意这里z没除，后面triangle里会除
    // 用games101 的视口矩阵替代
    // return Vector3f((v.x + 1.) * width * 0.5, (v.y + 1.) * height * 0.5, v.z);
}

Matrix4f PhongShader::get_model_matrix()
{
	return this->M_model;
}

Matrix4f PhongShader::get_view_matrix()
{
	return this->M_view;
}

Matrix4f PhongShader::get_projection_matrix()
{
	return this->M_projection;
}

Matrix4f PhongShader::get_viewport_matrix()
{
	return this->M_viewport;
}

Vector4f PhongShader::vertex(int i, int j, Vex& vex){
    vex.world_coords[j] = model->vert(i, j);
    vex.texture_coords[j] = model->uv(i, j);
    vex.normal_coords[j] = model->normal(i, j);

    uniform_M = M_projection * M_view * M_model;
    uniform_MIT = M_model.inverse_transpose();

    Vector4f v(vex.world_coords[j].x, vex.world_coords[j].y, vex.world_coords[j].z, 1.f);
    v = uniform_M * v;
    vex.w[j] = v.w;
    return v;
}

Vector4f DepthShader::vertex(int i, int j, Vex& vex){
    vex.world_coords[j] = model->vert(i, j);
    Vector4f v(vex.world_coords[j].x, vex.world_coords[j].y, vex.world_coords[j].z, 1.f);
	uniform_M = M_projection * M_view;
    v = uniform_M * v;
    vex.w[j] = v.w;
    return v;
}

Matrix4f DepthShader::get_MS(){
    return M_viewport * M_projection * M_view;
}

void PhongShader::set_shadowMatrix(Matrix4f MS){
	MS_view = MS;
}