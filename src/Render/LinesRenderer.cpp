#include"LinesRenderer.hpp"
#include<Camera.hpp>

#ifdef _WINDOWS
#include <Common/wgl_wrap.hpp>
#define WGL_NV_gpu_affinity
#else
#include<EGL/egl.h>
#include<EGL/eglext.h>
const EGLint egl_config_attribs[] = {EGL_SURFACE_TYPE,
                                     EGL_PBUFFER_BIT,
                                     EGL_BLUE_SIZE,
                                     8,
                                     EGL_GREEN_SIZE,
                                     8,
                                     EGL_RED_SIZE,
                                     8,
                                     EGL_DEPTH_SIZE,
                                     8,
                                     EGL_RENDERABLE_TYPE,
                                     EGL_OPENGL_BIT,
                                     EGL_NONE};



void EGLCheck(const char *fn) {
    EGLint error = eglGetError();

    if (error != EGL_SUCCESS) {
        throw runtime_error(fn + to_string(error));
    }
}
#endif

#include <cudaGL.h>
#include "IRenderer.hpp"
#include <Common/transferfunction_impl.h>
#include <Common/help_gl.hpp>
#include <random>
#include <SWCP.hpp>
#include <list>

extern "C"
{
    WIN_API IRenderer* get_renderer(){
        return reinterpret_cast<IRenderer*>(new LinesRenderer());
    }
}

void LinesRenderer::createGLShader() {
    line_shader=std::make_unique<sv::Shader>("../../src/Render/Shaders/markedpath_v.glsl",
                                                   "../../src/Render/Shaders/markedpath_f.glsl");
}

LinesRenderer::LinesRenderer(int w, int h)
:window_width(w),window_height(h)
{
    if(w>2048 || h>2048 || w<1 || h<1){
        throw std::runtime_error("bad width or height");
    }
}

void LinesRenderer::setupShaderUniform(){
    line_shader->setInt("window_width",window_width);
    line_shader->setInt("window_height",window_height);
    glm::vec3 camera_pos={camera.pos[0],camera.pos[1],camera.pos[2]};
    glm::vec3 view_direction={camera.front[0],camera.front[1],camera.front[2]};
    glm::vec3 up={camera.up[0],camera.up[1],camera.up[2]};
    glm::vec3 right=glm::normalize(glm::cross(view_direction,up));
    line_shader->setVec3("camera_pos",camera_pos);
    line_shader->setVec3("view_pos",camera_pos+view_direction*camera.n);
    line_shader->setVec3("view_direction",view_direction);
    line_shader->setVec3("view_right",right);
    line_shader->setVec3("view_up",up);
    line_shader->setFloat("view_depth",camera.f);
    float space=camera.f*tanf(glm::radians(camera.zoom/2))*2/window_height;
    line_shader->setFloat("view_right_space",space);
    line_shader->setFloat("view_up_space",space);
    line_shader->setFloat("step",1.f);
}

void LinesRenderer::set_mode(int mode) noexcept {

}

void LinesRenderer::set_camera(Camera camera) noexcept {
    this->camera=camera;
    glm::vec3 camera_pos={camera.pos[0],camera.pos[1],camera.pos[2]};
    glm::vec3 view_direction={camera.front[0],camera.front[1],camera.front[2]};
    glm::vec3 up={camera.up[0],camera.up[1],camera.up[2]};
    glm::vec3 right=glm::normalize(glm::cross(view_direction,up));
    auto center_pos=camera_pos+view_direction*(camera.f+camera.n)/2.f;
    auto fov =
      2 * atan(tan(45.0f * glm::pi<float>() / 180.0f / 2.0f) / camera.zoom);
    this->projection = glm::perspective(
      (double)fov, (double)SCR_WIDTH / (double)SCR_HEIGHT, 0.0001, 5.0);
    this->model = glm::mat4(1.0f);
    this->viewport = glm::lookAt(camera_pos,view_direction,up);
}

//找到屏幕上拾取的一点距离最近的已标注的点
// long LinesRenderer::findNearestNeuronNode_WinXY(int cx, int cy, NeuronTree * ptree, double &best_dist) //find the nearest node in a neuron in XY project of the display window
// {
// 	if (!ptree) return -1;
// 	list <NeuronSWC> *p_listneuron = &(ptree->listNeuron);
// 	if (!p_listneuron) return -1;

// 	GLdouble px, py, pz, ix, iy, iz;
// 	long best_ind=-1; best_dist=-1;
// 	for (long i=0;i<p_listneuron->size();i++)
// 	{
// 		ix = p_listneuron->at(i).x, iy = p_listneuron->at(i).y, iz = p_listneuron->at(i).z;
// 		GLint res = gluProject(ix, iy, iz, model, projection, viewport, &px, &py, &pz);// note: should use the saved modelview,projection and viewport matrix
// 		py = viewport[3]-py; //the Y axis is reversed
// 		if (res==GL_FALSE) {
//            std::cout <<"gluProject() fails for NeuronTree [" << i << "] node";
//            return -1;
//         }
// 		double cur_dist = (px-cx)*(px-cx)+(py-cy)*(py-cy);

// #ifdef _NEURON_ASSEMBLER_
// 		if (cur_dist < this->radius * this->radius) this->indices.insert(i);
// #endif

// 		if (i==0) {	best_dist = cur_dist; best_ind=0; }
// 		else 
// 		{	
// 			if (cur_dist<best_dist) 
// 			{
// 				best_dist=cur_dist; 
// 				best_ind = i;
// 			}
// 		}
// 	}

// 	return best_ind; 
// }

void LinesRenderer::set_user( std::shared_ptr<NeuronPool> user){
    this->userPool = user;
    this->neuronGraph = this->userPool->getGraph();
}

void LinesRenderer::render_frame(){
    line_shader->use();
    line_shader->setMat4("MVPMatrix", model*projection*viewport);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(0.5,0.5,0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    for( auto pl = neuronGraph->lines.begin(); pl != neuronGraph->lines.end() ; pl ++ ){
        if( !userPool->getLineVisible(pl->first) ){
            continue; //不绘制非可视的线
        }
            glColor3f(0.5,0.5,0.5);
            glLineWidth(3);
            glBindVertexArray(neuronGraph->graphDrawManager->hash_lineid_vao_vbo[pl->first].first); // vao
            // glDrawElements(GL_LINES, line_num_of_path_,
            //             GL_UNSIGNED_INT, nullptr);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LinesRenderer::set_volume(const char* path){

}

void LinesRenderer::set_mousekeyevent(MouseKeyEvent event) noexcept {

}

auto LinesRenderer::get_frame() -> const Image & {
    frame.width=window_width;
    frame.height=window_height;
    frame.channels=3;
    frame.data.resize(window_width*window_height*3);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glReadPixels(0, 0, frame.width, frame.height, GL_RGB, GL_UNSIGNED_BYTE,
                 reinterpret_cast<void *>(frame.data.data()));
    return frame;
}

void LinesRenderer::set_transferfunc(TransferFunction tf) noexcept {
}


void LinesRenderer::clear_scene() {

}

void LinesRenderer::set_querypoint(std::array<uint32_t,2> screen_pos) noexcept {

}

auto LinesRenderer::get_querypoint() -> const std::array<float, 8> {
    // return std::array<float, 8>{query_point_result[0],
    //                             query_point_result[1],
    //                             query_point_result[2],
    //                             query_point_result[3],
    //                             query_point_result[4],
    //                             query_point_result[5],
    //                             query_point_result[6],
    //                             query_point_result[7]};
    return std::array<float, 8>();
}