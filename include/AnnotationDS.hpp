//
// Created by wyz on 2021/5/3.
//

#ifndef NEURONANNOTATION_ANNOTATIONDS_HPP
#define NEURONANNOTATION_ANNOTATIONDS_HPP

#include<vector>
#include<list>
#include<memory>
#include<map>
#include<Poco/Mutex.h>
#include<Camera.hpp>
#include<seria/deserialize.hpp>
using namespace std;

class NeuronPool;
class GraphDrawManager;
class NeuronGraph;

enum Tools
{
    Drag,
    Insert,
    Cut,
    Select,
    Delete
};

enum Type
{
    Undefined = 0,
    Soma, //胞体 1
    Axon, //轴突 2
    Dendrite, //树突 3
    ApicalDendrite, //顶树突 4
    ForkPoint, //分叉点 5
    EndPoint, //端点 6
    Custom,
};

// struct Vertex{
//     glm::vec3 pos;
//     int vertex_index;
//     NeuronGraph* graph;
// };
// struct Edge{
//     Vertex *v0,*v1;
//     NeuronGraph* graph;
//     int edge_index;
//     float length;
// };
// struct Line{
//     std::list<Edge*> edges;
//     int line_index;
//     NeuronGraph* graph;
// };

typedef struct BasicObj
{
    int64_t id; //index
    std::string color; //color
    bool visible; //是否可见
    bool selected; //是否被选择
    std::string name;
    BasicObj(){
        id=0;
        color="#000000";
        selected=false;
        visible=true;
        name="";
    }
}BasicObj;

typedef struct NeuronSWC : public BasicObj
{
    //继承id
    Type type;
    double x,y,z;
    union{
        double r;
        double radius;
    };
    union
    {
        int64_t pn;
        int64_t parent;
    };
    int64_t line_id; //属于的路径id
    int64_t seg_size; //线段的swc大小
    int64_t seg_id; //线段的id
    int64_t seg_in_id; //该点在线段内的id
    int64_t user_id; //点所在脑数据中的block
    int64_t timestamp; //时间戳
    bool deleted; //已在数据库删除 假删除用
    NeuronSWC(){
        id=0;
        type=Undefined;
        x=y=z=0;
        r=1;
        pn=-1;
        line_id=-1;
        seg_id=-1;
        seg_size=-1;
        seg_in_id=-1;
        user_id=-1;
        timestamp=-1;
        deleted=false;
    }
} NeuronSWC;

typedef struct Vertex : public BasicObj //记录关键节点和它们之下的节点的SWC索引
{

    Vertex( Type type, double x, double y, double z, float radius) : type(type), radius(radius), x(x), y(y), z(z)
    {
        line_id = -1;
        hash_linked_seg_ids.clear();
        linked_vertex_ids.clear();
    };
    
    Vertex() {};

    double x,y,z;
    float radius;
    Type type;
    int64_t line_id;
    int64_t timestamp;
    map<int, bool> hash_linked_seg_ids; //相关的线id
    map<int, bool> linked_vertex_ids; //相连的点id
} Vertex;

typedef struct Segment : public BasicObj //路径中的单个线段
{
    int size;
    int line_id;
    int start_id;
    int end_id;
    map<int,int> segment_vertex_ids; //在线中的顶点在SWC中的索引id
    Segment(){
        size = 0;
        line_id = -1;
        start_id = -1;
        end_id = -1;	
    }
    Segment(int s, int l){
        size = s;
        line_id = l;
        start_id = -1;
        end_id = -1;
    };

} Segment;
		
struct Line : public BasicObj //Line是有关关键Vertex的集合
{
    map< int, Vertex > hash_vertexes;
    int user_id;
    Line(){
        id=0;
        color="#000000";
        selected=false;
        visible=true;
        name="";
        user_id = -1;
    }
    unsigned int vao, ebo;
};

class GraphDrawManager{
    public:
        NeuronGraph * graph;
        bool inited;
        unsigned int vbo; //顶点集合
        std::map<int, std::pair<unsigned int, unsigned int> > hash_lineid_vertex_vao_ebo; //line_顶点vao
        std::map<int,int> vector_num_of_path; //line 点数
        long long v_count; //当前顶点数量
        std::map<int, std::pair<unsigned int, unsigned int> > hash_lineid_vao_ebo;
        std::map<int,int> line_num_of_path;
        std::vector<int> rebuild_line_id; //-1 default
        std::map<int,int> rebuild_swc_id;
    public:
        GraphDrawManager( NeuronGraph *g ){
            graph = g;
            inited = false;
            rebuild_line_id.clear();
            rebuild_swc_id.clear();
        }
        void setRebuildLine(int id){
            rebuild_line_id.push_back(id);
        }
        void RebuildLine();
        void InitGraphDrawManager();
        void Delete( int line_id );
        void UpdateSWC();
};


class NeuronGraph : public BasicObj{
public:
    NeuronGraph(const char * string, int type=0); //0=DataBase 1=File
    NeuronGraph(const char * filePath, const char * tableName);
    NeuronGraph(){};
    // explicit NeuronGraph(int idx):graph_index(idx){}
    long selectVertex( int x, int y, NeuronPool *n );
    bool selectVertices(std::vector<int> idxes);
    bool selectEdges(std::vector<int> idxes);
    bool selectLines(std::vector<int> idxes);
    bool deleteCurSelectVertices();
    bool deleteCurSelectEdges();
    bool deleteCurSelectLines(    );

    
    bool addVertex(Vertex* v);
    bool addSegment(int id, Vertex* v);
    long long addSegment(int id,std::vector<std::array<float,4>> *path); //return final id

    bool devidedInto2Lines(long id);

    long int addLine();
    long int getNewVertexId();
    long int getNewSegmentId();
    long int getNewLineId();
    long int getCurMaxVertexId();
    long int getCurMaxLineId();
    long int getCurMaxSegmentId();
    
    bool deleteLine(int line_id);
    bool deleteVertex(int x, int y, NeuronPool *neuron_pool, std::string &error);

    void setMaxVertexId(long int id);
    void setMaxLineId(long int id);
    void setMaxSegmentId(long int id);

    bool changeName(int line_id, string name);
    bool changeColor(int line_id, string color);

    double getDistance(int seg_id);
    double getDistance( float x1, float y1, float z1, float x2, float y2, float z2 );
    string getLinestoJson(NeuronPool * np);

    int getDefaultSelectedLineIndex();
    int getDefaultSelectedVertexIndex(int line_id);

    string file; //文件源
    string tableName; //保存在数据库的表名
    vector<NeuronSWC> list_swc; //list_swc中间删除时，需要对hash_swc_id重新计算
    map<int,int> hash_swc_ids; //方便查询，点与相关联SWC文件的映射索引
    map<int,Line> lines; //路径合集（点合辑）
    map<int,Segment > segments; //关键点及非关键点的线段合集
    vector<string> meta; //memo

private:
    long int cur_max_line_id;
    long int cur_max_seg_id;
    long int cur_max_vertex_id;

    Poco::Mutex max_line_id_mutex;
    Poco::Mutex max_seg_id_mutex;
    Poco::Mutex max_vertex_id_mutex;
    Poco::Mutex list_and_hash_mutex;
    // int graph_index;
    // std::map<int,Segment*> segments;
    // std::map<int,Vertex*> vertices;
    // std::map<int,Line*> lines;
    // std::vector<Vertex*> cur_select_vertices;//last add or current pick
    // std::vector<Segment*> cur_select_segments;//last add edge or current select edge
    // std::vector<Line*> cur_select_lines;
    // int select_obj;//0 for nothing, 1 for point, 2 for edge, 3 for points, 4 for edges,5 for line,6 for lines

public:
    bool formatGraphFromSWCList();
    int formatSegments(std::map<int,vector<int>> &vertexLinkedCount, int index);
    long findNearestVertex(int cx, int cy, NeuronPool * neuron_pool, double &best_dist);
public:
    GraphDrawManager *graphDrawManager;
};


class NeuronPool{
public:
    void selectVertex(int id);
    long selectVertex(int x, int y);
    void selectLine(int id);
    NeuronPool(){
        m_selected_vertex_index = -1;
        m_selected_line_index = -1;
        b_set_camera = false;
        m_mode = 0; //默认DVR
        //m_mode = 1;
        m_tool = 0; //默认拖拽

        window_width = 1200;
        window_height = 700;
    }
    string getLinestoJson();
    bool getLineVisible(int id);
    bool addVertex(Vertex *v);
    bool addVertex(float x, float y, float z);
    bool addLine();
    bool deleteLine(int line_id);
    bool jumpToVertex(int id);
    bool addSegment(std::vector<std::array<float,4>> *path); //return final id
    bool dividedInto2Lines(int x, int y);
    bool deleteVertex(int x, int y, std::string &error);
    bool changeMode(string modeName);
    bool changeTable(string tableName);
    bool changeVisible(int line_id, bool visible);
    bool changeColor(int line_id, string color);
    bool changeName(int line_id, string name);
    bool hasCamera();
    std::array<int,2> getSelectedVertexXY();
    void setGraph( std::shared_ptr<NeuronGraph> pN){
        graph = pN;
    };

    int getWindowWidth();
    int getWindowHeight();
    void setWindowWidth(int w);
    void setWindowHeight(int h);

    void setUserId( int id ){
        user_id = id;
    }
    std::shared_ptr<NeuronGraph> getGraph(){
        return graph;
    }

private:
    bool b_set_camera;
    Camera m_camera; //视角信息
    int m_selected_vertex_index; //当前编辑顶点
    int m_selected_line_index; //当前选择路径
    map<int,bool> line_id_visible; //路径可视映射
    int m_mode; //渲染模式
    int m_tool; //工具的index

    int window_width;
    int window_height;

public:
    void setTool(int toolIndex);
    int getTool();
    void setCamera(Camera c);
    Camera getCamera();
    int getRenderMode();
    int getSelectedLineIndex();
    int getSelectedVertexIndex();
    void initSelectedLineIndex();
    void initSelectedVertexIndex();
    void setGraphPool( std::map<string,std::shared_ptr<NeuronGraph> > *graphs){
        graphs_pool = graphs;
    }
private:
    int user_id;
    std::shared_ptr<NeuronGraph> graph;
    std::map<string,std::shared_ptr<NeuronGraph> > *graphs_pool;

};


#endif //NEURONANNOTATION_ANNOTATIONDS_HPP
