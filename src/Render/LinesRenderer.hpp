#ifndef NEURONANNOTATION_LINESRENDERER_H
#define NEURONANNOTATION_LINESRENDERER_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SWCP.hpp>
#include "IRenderer.hpp"
// #include <UserInfo.hpp>
#ifdef _WINDOWS
#include <windows.h>
#endif
#include <Common/utils.hpp>
#include <unordered_set>
#include "ShaderProgram.hpp"

class LinesRenderer final: public IRenderer{
public:
    LinesRenderer(int w=1200,int h=900);
    LinesRenderer(const LinesRenderer&)=delete;
    LinesRenderer(LinesRenderer&&)=delete;
    ~LinesRenderer();

private:
    int SCR_WIDTH;
    int SCR_HEIGHT;

public:
    void set_volume(const char* path) override;

    void set_camera(Camera camera) noexcept override;

    void set_transferfunc(TransferFunction tf) noexcept override;

    void set_mousekeyevent(MouseKeyEvent event) noexcept override;

    void set_querypoint(std::array<uint32_t,2> screen_pos) noexcept override;

    void render_frame() override;

    void set_mode(int mode) noexcept override;

    auto get_frame()->const Image& override;

    auto get_querypoint()->const std::array<float,8> override;

    void clear_scene() override;

    void set_user(std::shared_ptr<NeuronPool> pool);

private:
    std::unique_ptr<sv::Shader> line_shader;
public:
    void createGLShader();
    void initResourceContext();
    void initGL();
    void initCUDA();
    void setupShaderUniform();
private:
    uint32_t window_width,window_height;

    Image frame;
	glm::mat4 viewport;
	glm::mat4 projection;
	glm::mat4 model;

private: //用户相关
    Camera camera;
    std::shared_ptr<NeuronPool> userPool;
    std::shared_ptr<NeuronGraph> neuronGraph;
private:    
    //CUcontext cu_context;

#ifdef _WINDOWS
    HDC window_handle;
    HGLRC gl_context;
#endif
};

extern "C"{
WIN_API   IRenderer* get_renderer();
}
#endif
