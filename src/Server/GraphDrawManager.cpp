#include <AnnotationDS.hpp>
#include <glm/glm.hpp>
#define MAX_VERTEX_NUMBER (int)1000000
#include <glad/gl.h>
#include <glad/wgl.h>

using vec3 = glm::vec3;

vec3 tocolor( std::string color ){
    string r = color.substr(1,2);
    string g = color.substr(3,2);
    string b = color.substr(5,2);
    float r_f = (double)stol(r.c_str(),NULL,16)/(double)256;
    float g_f = (double)stol(g.c_str(),NULL,16)/(double)256;
    float b_f = (double)stol(b.c_str(),NULL,16)/(double)256;
    return vec3(r_f,g_f,b_f);
}

void GraphDrawManager::InitGraphDrawManager(){

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER, MAX_VERTEX_NUMBER * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);
    for( int i = 0 ; i < graph->list_swc.size() ; i ++  ){
        vec3 color = tocolor(graph->list_swc[i].color);
        float v[] = {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z,color.r,color.g,color.b};
        glNamedBufferSubData(vbo, i * sizeof(float) * 6,
                         6 * sizeof(float), v);
    }
    for( auto line : graph->lines ){
        line_num_of_path[line.first] = 0; //初始化路径的线数
    }
    for( auto seg = graph->segments.begin() ; seg != graph->segments.end() ; seg ++ ){
        if( hash_lineid_vao_ebo.find(seg->second.line_id) == hash_lineid_vao_ebo.end() ){ //该条线未初始化
            unsigned int vao, ebo;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &ebo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo); //绑定同一个图的vbo
            //pos
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),(void *)0);
            glEnableVertexAttribArray(0);
            //color
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),(void *)(3* sizeof(float)));
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 1024 * 2 * sizeof(uint64_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
            hash_lineid_vao_ebo[seg->second.line_id] = std::make_pair(vao,ebo);
        }
        GLuint ebo = hash_lineid_vao_ebo[seg->second.line_id].second;
        uint64_t head;
        for( auto v : seg->second.segment_vertex_ids){
            if (v.first == 0){
                head = graph->hash_swc_ids[v.second];
                continue;
            }
            uint64_t idx[2] = {head,graph->hash_swc_ids[v.second]};
            head = graph->hash_swc_ids[v.second];
            glNamedBufferSubData(ebo,
                                line_num_of_path[seg->second.line_id] * 2 * sizeof(uint64_t),
                                2 * sizeof(uint64_t), idx);
            line_num_of_path[seg->second.line_id]++;
        }
    }//遍历所有segment
    
    inited = true;
}
