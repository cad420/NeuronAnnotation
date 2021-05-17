#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <queue>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Parser.h>
using namespace glm;
using namespace std;

typedef int infoType;
typedef vec3 vertexType;
typedef int weightType;


typedef struct arcInfo{

    vector<vertexType> lineVex; //用于绘制的点集合
    weightType weight; //路径长度

}arcInfo;


typedef struct arcBox{

    int tailVex, headVex;
    struct arcBox *hLink, *tLink;
    infoType *info;
    Poco::JSON::Object toJson(){

    }
    
}arcBox; //弧节点


class vexNode{//顶点节点

    private:
    bool b_islocked; //用于调度 TODO
    bool b_isdeleted; //假删除

    public:
    vertexType data;//数据
    arcBox *firstIn, *firstOut; //入弧、出弧
    string lastEditTime;

    vexNode(){
        b_isdeleted = false;
    }

    Poco::JSON::Object toJson(){
        Poco::JSON::Object vex;
        Poco::JSON::Array arc;
        vex.set("lastEditTime",lastEditTime);
        arcBox *p = firstIn;
        while(p){
            arc.add(p->toJson());
            p = p->hLink;
        }
        p = firstOut;
        while(p){
            arc.add(p->toJson());
            p = p->tLink;
        }
        vex.set("arc",arc);
        return vex;
    }

    bool IsLocked(){
        std::cout << "顶点的上锁情况为" << b_islocked << endl;
        return b_islocked;
    }
    bool Lock(){
        std::cout << "顶点的上锁情况为" << b_islocked << endl;
        if(b_islocked) return false;
        return b_islocked = true;
    }
    bool UnLock(){
        std::cout << "顶点的上锁情况为" << b_islocked << endl;
        if(!b_islocked) return true;
        return b_islocked = false;
    }
    void DeleteVex(){
        b_isdeleted = true;
    }
    bool IsDeleted(){
        return b_isdeleted;
    }
};


class OLGraph{

    private:
    bool b_isVisible;
    int vexNum;
    int arcNum;
    string color;
    string name;
    int key;

    public:
    vector<vexNode> xList;
    uint32_t vbo;
    uint32_t ebo;
    uint32_t vao;
    OLGraph(){
        vexNum = 0;
        arcNum = 0;
        vbo = 0;
        ebo = 0;
        vao = 0;
        b_isVisible = true;
        color = {1.0f,1.0f,1.0f};
    };

    Poco::JSON::Object toJson(){
        Poco::JSON::Object graph;
        Poco::JSON::Array vexArray;
        for( int i = 0 ; i < xList.size() ; i ++ ){ //不记录删除的点，按照当前顺序赋值，但是index才是实际数组序列
            if ( !xList[i].IsDeleted() ){
                Poco::JSON::Object v= xList[i].toJson();
                v.set("index",i);
                vexArray.add(v);
            }
        }
        graph.set("vertexArray",vexArray);
        graph.set("vexNum",vexNum);
        graph.set("color",color);
        graph.set("name",name);
        graph.set("index",key);
        return graph; 
    }

    //点定位
    int locateVertex( vexNode node ){
        int index = -1;
        for( int i = 0; i < this->vexNum ; i ++ ){
            if( node.data == this->xList[i].data ){
                index = i;
                break;
            }
        }
        return index;
    }

    pair<int,int> locateArcBox( vexNode node ){
        int index1 = -1;
        int index2 = -1;
        arcBox *p;
        for( int i = 0; i < this->vexNum ; i ++ ){
            p = this->xList[i].firstOut;
            // while( p ){
            //     if( node.data )
            //     p = p->hLink;
            // }
        }
    }

    //分开一个图，原本的图中删去以第二个点为聚集的点
    OLGraph *DivideGraph( vexNode node ){
        pair<int,int> arc = locateArcBox(node);
        deleteArc(arc.first,arc.second);
        deleteArc(arc.second,arc.first);
        vector<int> visited(this->vexNum,0);
        BFS_Traverse(arc.first,visited);
        OLGraph *nOLGraph = new OLGraph(*this);
        //做假删除，删除所有连接的边，但是不删除点
        for( int i = this->vexNum - 1 ; i >= 0 ; i --){
            if( !visited[i] ){ //属于第二个图的
                arcBox* p = this->xList[i].firstOut;
                arcBox* n;
                while(p){
                    n = p->tLink;
                    this->deleteArc(p->headVex,p->tailVex);
                    p = n;
                }
                this->xList[i].DeleteVex();
                this->vexNum--;
            } else {
                arcBox* p = nOLGraph->xList[i].firstOut;
                arcBox* n;
                while(p){
                    n = p->tLink;
                    nOLGraph->deleteArc(p->headVex,p->tailVex);
                    p = n;
                }
                nOLGraph->xList[i].DeleteVex();
                nOLGraph->vexNum--;
            }
        }
        return nOLGraph;
    }

    //bfs遍历
    void BFS_Traverse(int index, vector<int> &visited){
        OLGraph grp = *this;
        queue<int> q;
        for( int i = 0 ; i < grp.vexNum ; i ++ ){
            if( !visited[i] ){
                printf("vis %d\n",i);
                visited[i] = 1;
                q.push(i);
                while( q.size() != 0 ){
                    while( NULL != grp.xList[q.front()].firstOut && !visited[this->xList[q.front()].firstOut->headVex]){
                        q.push(grp.xList[q.front()].firstOut->headVex);
                        visited[grp.xList[q.front()].firstOut->headVex] = 1;
                        grp.xList[q.front()].firstOut = grp.xList[q.front()].firstOut->tLink;
                    }
                    q.pop();
                }
            }
        }
    }

    //找端点，即出度为0的点
    // vector<vertexType> getTerminalList(){
    //     vector<vertexType> terminalList;
    //     for( int i = 0; i < this->vexNum ; i ++ ){
    //         if( !this->xList[i].firstOut )
    //             terminalList.push_back(this->xList[i].data);
    //     }
    //     return terminalList;
    // }

    //插入弧
    arcBox* insertArc(int index1, int index2){
        // int index1 = locateVertex(node1);
        // int index2 = locateVertex(node2);
        if( index1 == -1 || index2 == -1 ){
            printf("顶点不存在！\n");
            return NULL;
        }
        arcBox* pArc = new arcBox[1];
        pArc->tailVex = index1;
        pArc->headVex = index2;
        pArc->info = NULL;

        arcBox *ptail = this->xList[index1].firstOut;
        arcBox *phead = this->xList[index2].firstIn;
        
        pArc->tLink = ptail ? ptail : NULL;
        pArc->hLink = phead ? phead : NULL;

        this->xList[index1].firstOut = pArc;
        this->xList[index2].firstIn = pArc;
        this->arcNum++;
        return pArc;
    }

    //删除1->2的弧
    int deleteArc(int index1, int index2 ){

        //删除2中的1
        arcBox *cur = this->xList[index2].firstIn;
        arcBox *pre = cur;
        int count = 0;
        while(cur){
            count++;
            if(cur->tailVex == index1) break;
            pre = cur;
            cur = cur->hLink;
        }
        if(cur){
            if( count <= 1 ){
                this->xList[index2].firstIn = pre->hLink;
            }
            else{
                pre->hLink = cur->hLink;
            }
        } else { //没有找到该弧
            return -1;
        }
        free(cur);//释放内存

        //删除1中的2
        cur = this->xList[index1].firstOut;
        pre = cur;
        count = 0;
        while(cur){
            count++;
            if(cur->headVex == index2) break;
            pre = cur;
            cur = cur->hLink;
        }
        if(cur){
            if( count <= 1 ){
                this->xList[index1].firstOut = pre->tLink;
                //删除第一个弧节点
            }
            else{
                pre->tLink = cur->tLink;
                //删除非第一个弧节点
            }
        } else { //没有找到该弧
            return -1;
        }

        free(cur);//释放内存
        this->arcNum--;
        return 1;
    }

    //插入点
    int insertNode(vertexType data){
        vexNode n;
        n.data = data;
        n.firstIn = NULL;
        n.firstOut = NULL;
        this->xList.push_back(n);
        this->vexNum++;
        return 1;
    }

    //创建一个新的神经元，自动生成十字链表；
    //对每一个新插入的弧，先插入点再插入弧；

    //绘制视角
    // void setVisible(bool val){
    //     b_isVisible = val;
    // }

    // bool IsVisible(){
    //     return b_isVisible;
    // }

    //顶点数
    int getVexNum(){
        return vexNum;
    }

    //弧数
    int getArcNum(){
        return arcNum;
    }
};
