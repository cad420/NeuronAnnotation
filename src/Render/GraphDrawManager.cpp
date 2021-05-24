#include <AnnotationDS.hpp>
#include <glm/glm.hpp>
#define MAX_VERTEX_NUMBER (int)1000000
#include <glad/gl.h>
#include <glad/wgl.h>
#include <iostream>

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

void GraphDrawManager::UpdateSWC(){
    for( int i = v_count ; i < graph->list_swc.size() ; i ++ ){
        vec3 color = tocolor(graph->list_swc[i].color);
        float v[] = {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z,color.r,color.g,color.b};
        glNamedBufferSubData(vbo, i * sizeof(float) * 6,
                         6 * sizeof(float), v);
    }
    v_count = graph->list_swc.size();

    for( auto swc : rebuild_swc_id ){
        int i = swc.first;
        vec3 color = tocolor(graph->list_swc[i].color);
        float v[] = {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z,color.r,color.g,color.b};
        glNamedBufferSubData(vbo, i * sizeof(float) * 6,
                         6 * sizeof(float), v);
    }//修改颜色
    rebuild_swc_id.clear();
}

void GraphDrawManager::InitGraphDrawManager(){

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER, MAX_VERTEX_NUMBER * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);
    for( int i = 0 ; i < graph->list_swc.size() ; i ++  ){
        vec3 color = tocolor(graph->list_swc[i].color);
        // float v[] = {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z};
        // glNamedBufferSubData(vbo, i * sizeof(float) * 3,
        //                  3 * sizeof(float), v);
        float v[] = {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z,color.r,color.g,color.b};
        glNamedBufferSubData(vbo, i * sizeof(float) * 6,
                         6 * sizeof(float), v);
    }
    for( auto line : graph->lines ){
        line_num_of_path[line.first] = 0; //初始化路径的线数
        vector_num_of_path[line.first] = 0;

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
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 100000 * sizeof(uint32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
        hash_lineid_vertex_vao_ebo[line.first] = std::make_pair(vao,ebo);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        for( auto vec : line.second.hash_vertexes ){
            glNamedBufferSubData(ebo,
                                vector_num_of_path[line.first] * sizeof(uint32_t),
                                sizeof(uint32_t), &graph->hash_swc_ids[vec.first]);
            vector_num_of_path[line.first]++;
        }
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
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 100000 * 2 * sizeof(uint32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
            hash_lineid_vao_ebo[seg->second.line_id] = std::make_pair(vao,ebo);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glBindVertexArray(0);
        }
        GLuint ebo = hash_lineid_vao_ebo[seg->second.line_id].second;
        uint32_t head;
        for( auto v : seg->second.segment_vertex_ids){
            if (v.first == 0){
                head = graph->hash_swc_ids[v.second];
                continue;
            }
            // uint32_t idx[2] = {head,graph->hash_swc_ids[v.second]};
            uint32_t idx[2] = {graph->hash_swc_ids[v.second],graph->hash_swc_ids[graph->list_swc[graph->hash_swc_ids[v.second]].pn]};
            head = graph->hash_swc_ids[v.second];
            glNamedBufferSubData(ebo,
                                line_num_of_path[seg->second.line_id] * 2 * sizeof(uint32_t),
                                2 * sizeof(uint32_t), idx);
            line_num_of_path[seg->second.line_id]++;
        }
    }//遍历所有segment

    inited = true;
}

void GraphDrawManager::RebuildLine(){
    UpdateSWC();
    for( auto line_id : rebuild_line_id ){
        Delete(line_id);
        // int line_id = rebuild_line_id;
        if ( graph->lines.find(line_id) == graph->lines.end() ) continue;
        line_num_of_path[line_id] = 0;
        vector_num_of_path[line_id] = 0;
        {
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
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 100000 * sizeof(uint32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
            hash_lineid_vertex_vao_ebo[line_id] = std::make_pair(vao,ebo);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            glBindVertexArray(0);
        }
        for( auto vec : graph->lines[line_id].hash_vertexes ){
            glNamedBufferSubData(hash_lineid_vertex_vao_ebo[line_id].second,
                                vector_num_of_path[line_id] * sizeof(uint32_t),
                                sizeof(uint32_t), &graph->hash_swc_ids[vec.first]);
            vector_num_of_path[line_id]++;
        }

        for( auto seg = graph->segments.begin() ; seg != graph->segments.end() ; seg ++ ){
            if( line_id != seg->second.line_id) continue; //只重建该条路径
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
                glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 100000 * 2 * sizeof(uint32_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
                hash_lineid_vao_ebo[seg->second.line_id] = std::make_pair(vao,ebo);
                glBindBuffer(GL_ARRAY_BUFFER,0);
                glBindVertexArray(0);
            }
            GLuint ebo = hash_lineid_vao_ebo[line_id].second;
            for( auto v : seg->second.segment_vertex_ids){
                if (v.first == 0){
                    continue;
                }
                uint32_t idx[2] = {graph->hash_swc_ids[graph->list_swc[graph->hash_swc_ids[v.second]].pn],graph->hash_swc_ids[v.second]};
                std::cout << idx[0] << " " << idx[1] << std::endl;
                glNamedBufferSubData(ebo,
                                    line_num_of_path[seg->second.line_id] * 2 * sizeof(uint32_t),
                                    2 * sizeof(uint32_t), idx);
                line_num_of_path[seg->second.line_id]++;
            }
        }//遍历与该路径相关的所有segment
    }
    rebuild_line_id.clear();
}

void GraphDrawManager::Delete( int line_id ){
    if( hash_lineid_vao_ebo.find(line_id) != hash_lineid_vao_ebo.end() ){
        
        glDeleteVertexArrays(1, &hash_lineid_vao_ebo[line_id].first); //deleteVAO
        glDeleteBuffers(1, &hash_lineid_vao_ebo[line_id].second); //deleteEBO

        glDeleteVertexArrays(1, &hash_lineid_vertex_vao_ebo[line_id].first);
        glDeleteBuffers(1, &hash_lineid_vertex_vao_ebo[line_id].second);

        hash_lineid_vao_ebo.erase(line_id);
        line_num_of_path.erase(line_id);
        hash_lineid_vertex_vao_ebo.erase(line_id);
        vector_num_of_path.erase(line_id);
    }
}