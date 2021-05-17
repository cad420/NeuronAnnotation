#ifndef SWCP_HPP
#define SWCP_HPP

#include <stdlib.h>
#include <cstdarg>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <list>
#include <regex>
#include <iostream>

namespace SWCP 
{
	using namespace std;
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

	#if __cplusplus >= 201103L
		#define SWCP_CPP11_COMPATIBLE
	#endif
	#if defined _MSC_VER && _MSC_VER >= 1800
		#define SWCP_CPP11_COMPATIBLE
	#endif

	// If not c++11 and if it is not version of MSVC that is c++11 compatible, 
	// then define own int64_t and uint64_t types in SWCP namespace.
	#if !defined SWCP_CPP11_COMPATIBLE
		typedef long long int int64_t;
		typedef unsigned long long int uint64_t;
	#endif

	// strtoll and strtoull are not part of standard prior c++11, 
	// but are widely supported, except for MSVC that has own names for them
	#if defined _MSC_VER && (_MSC_VER < 1800)
		inline int64_t strtoll(char const* str, char** endPtr, int radix)
		{
			return _strtoi64(str, endPtr, radix);
		}

		inline uint64_t strtoull(char const* str, char** endPtr, int radix)
		{
			return _strtoui64(str, endPtr, radix);
		}
	#endif

	// Safe version of sprintf
	#if !defined SWCP_CPP11_COMPATIBLE && defined _MSC_VER
		template<class T, size_t N>
		inline int64_t sprintf(T(&dstBuf)[N], const char * format, ...)
		{
			va_list args;
			va_start(args, format);
			int result = vsprintf_s(dstBuf, format, args);
			va_end(args);
			return result;
		}
	#elif defined SWCP_CPP11_COMPATIBLE
		template<class T, size_t N>
		inline int sprintf(T(&dstBuf)[N], const char * format, ...)
		{
			va_list args;
			va_start(args, format);
			int result = vsnprintf(dstBuf, N, format, args);
			va_end(args);
			return result;
		}
	#endif

	int GetCurrentTimestamp(){
		time_t timer;
		return timer;
	}

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
			float r;
			float radius;
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
		NeuronSWC(){
			id=0;
			type=Undefined;
			x=y=z=0;
			r=1;
			pn=-1;
			line_id=-1;
			seg_id=1;
			seg_size=-1;
			seg_in_id=-1;
			user_id=1;
			timestamp=-1;
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
		map<int, bool> hash_linked_seg_ids; //相关的线id
		map<int, bool> linked_vertex_ids; //相连的点id
	} Vertex;

	typedef struct Segment //路径中的单个线段
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
	};
	
	struct Graph
	{
		string file; //文件源
		vector<NeuronSWC> list_swc; //list_swc中间删除时，需要对hash_swc_id重新计算
		map<int,int> hash_swc_ids; //方便查询，点与相关联SWC文件的映射索引
		map<int,Line> lines; //路径合集（点合辑）
		map<int,Segment > segments; //关键点及非关键点的线段合集
		vector<string> meta; //memo
	};

	class Parser
	{
	public:
		// Reads SWC from filespecified by *filename*. 
		// Output is written to *graph*, old content is errased. 
		// If no error have happened returns true
		bool ReadSWCFromFile(const char *filename, Graph& graph);

		// Reads SWC from filestream. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(std::istream &inStream, Graph& graph);

		// Reads SWC from string. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(const char *string, Graph& graph);
		
		// Returns error message for the last parsing if error have happened.
		std::string GetErrorMessage();
	private:
		void NextSymbol();

		vector<std::string> Split(const std::string& in, const std::string& delim);
		bool Accept(char symbol);
		bool AcceptWhightSpace();
		bool AcceptLine(Graph& graph);
		bool AcceptEndOfLine();
		bool AcceptInteger(int64_t& integer);
		bool AcceptInteger(uint64_t& integer);
		bool AcceptDouble(double& integer);
		bool AcceptStringWithoutSpace(char &);
		
		const char* m_iterator;
		int m_line;
		std::stringstream m_errorMessage;
	};

	class Generator
	{
	public:
		bool WriteToFile(const char *filename, const Graph& graph);
		bool Write(std::ostream &outStream, const Graph& graph);
		bool Write(std::string& outString, const Graph& graph);
		std::string GetErrorMessage();
	private:
		enum 
		{
			MaxLineSize = 4096
		};
		std::stringstream m_errorMessage;
	};

	inline bool Parser::ReadSWCFromFile(const char *filename, Graph& graph)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (!file.is_open())
		{
			m_errorMessage.clear();
			m_errorMessage << "Error: Can not open file: " << filename << '\n';
			return false;
		}

		char* content = new char[static_cast<size_t>(size) + 1];
		file.read(content, size);

		content[size] = '\0';
		graph.file = filename;
		bool result = ReadSWC(content, graph);

		delete[] content;

		return result;
	}

	inline bool Parser::ReadSWC(std::istream &inStream, Graph& graph)
	{
		std::string str;
		char buffer[4096];
		while (inStream.read(buffer, sizeof(buffer)))
		{
			str.append(buffer, sizeof(buffer));
		}
		str.append(buffer, static_cast<unsigned int>(inStream.gcount()));
		return str.c_str();
	}

	inline bool Parser::ReadSWC(const char *string, Graph& graph){	
		m_errorMessage.clear();
		graph.list_swc.clear();
		graph.hash_swc_ids.clear();
		graph.lines.clear();
		graph.segments.clear();
		graph.meta.clear();

		m_line = 1;
		m_iterator = string;

		while (AcceptLine(graph))
		{
			++m_line;
		}

		if (*m_iterator == '\0')
		{
			return true;
		}
		else
		{
			m_errorMessage << "Error at line: " << m_line << ", unexpected symbol:" << *m_iterator << '\n';
			return false;
		}
	}

	inline vector<std::string> Parser::Split(const std::string& in, const std::string& delim){
		std::regex re{ delim };
		return std::vector<std::string> {
			std::sregex_token_iterator(in.begin(), in.end(), re, -1),
			std::sregex_token_iterator()
		};
	}

	inline bool Parser::AcceptLine(Graph& graph){
		while( AcceptWhightSpace() ){}
		if ( AcceptEndOfLine() ) return true;
		if (Accept('#')) //纯注释行
		{
			const char* commentStart = m_iterator;
			const char* commentEnd = m_iterator;
			while (!AcceptEndOfLine() && *m_iterator != '\0')
			{
				NextSymbol();
				commentEnd = m_iterator;
			}
			graph.meta.push_back(std::string(commentStart, commentEnd));
			return true;
		}
		NeuronSWC swc;
		for( int i = 0 ; i < 7 ; i ++ ){
			char* endp_int = NULL;
			char* endp_double = NULL;
			int64_t result_int = strtoll(m_iterator,&endp_int,0);
			double result_double = strtod(m_iterator, &endp_double);
			if (endp_int > m_iterator || endp_double > m_iterator){
				switch( i ){
					case 0: //id
						swc.id = result_int;
						m_iterator = endp_int;
						break;
					case 1: //type
						swc.type = Type(result_int);
						m_iterator = endp_int;
						break;
					case 2: //x
						swc.x = result_double;
						m_iterator = endp_double;
						break;
					case 3: //y
						swc.y = result_double;
						m_iterator = endp_double;
						break;
					case 4: //z
						swc.z = result_double;
						m_iterator = endp_double;
						break;
					case 5: //r
						swc.r = result_double;
						m_iterator = endp_double;
						break;
					case 6: //pn
						swc.pn = result_int;
						m_iterator = endp_int;
						break;
					default:
						m_errorMessage << "Error at Line:" << m_line;
						return false;
				}
			}
			else{
				return false;
			}
			while (AcceptWhightSpace()){}
		}
		if( Accept('#') ){ //其他相关SWC
			const char* infoStart = m_iterator;
			const char* infoEnd = m_iterator;
			while (!AcceptEndOfLine() && *m_iterator != '\0')
			{
				NextSymbol();
				infoEnd = m_iterator;
			}
			vector<string> infoStr = Split(std::string(infoStart,infoEnd), " ");
			for( int i = 0 ; i < infoStr.size() ; i ++ ){
				if( infoStr[i].find("name") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.name = infoStr[i].substr(startPos+1,infoStr[i].length()-startPos);
					continue;
				}
				if( infoStr[i].find("line_id") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.line_id = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("seg_id") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.seg_id = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("seg_in_id") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.seg_in_id = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("seg_size") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.seg_size = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("user_id") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.user_id = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("timestamp") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.timestamp = atoi(infoStr[i].c_str()+startPos+1);
					continue;
				}
				if( infoStr[i].find("color") != infoStr[i].npos ){
					int startPos = infoStr[i].find(":");
					swc.color = infoStr[i].substr(startPos+1,infoStr[i].length()-startPos);
					continue;
				}
			}
		} else {
			m_errorMessage << "Error! It's not My SWC Type!";
			return false;
		}
		graph.list_swc.push_back(swc);
		graph.hash_swc_ids[swc.id] = graph.list_swc.size()-1;
		graph.segments[swc.seg_id].size = swc.seg_size;
		graph.segments[swc.seg_id].line_id = swc.line_id;
		graph.segments[swc.seg_id].segment_vertex_ids.insert({swc.seg_in_id,graph.list_swc.size()-1});
		if( graph.lines.find(swc.line_id) == graph.lines.end()){
			graph.lines[swc.line_id].id = swc.line_id;
			graph.lines[swc.line_id].color = swc.color;
			graph.lines[swc.line_id].name = swc.name;
			graph.lines[swc.line_id].user_id = swc.user_id;
		}
		if( swc.seg_in_id == 0 || swc.seg_in_id == swc.seg_size - 1 ){ //关键节点
			Vertex v;
			if( graph.lines[swc.line_id].hash_vertexes.find(swc.id) == graph.lines[swc.line_id].hash_vertexes.end()){ //如果该点不在线内
				v = Vertex(swc.type,swc.x,swc.y,swc.z,swc.r);
				v.id = swc.id;
				v.line_id = swc.line_id;
				v.color = swc.color;
				v.name = swc.name;
			}
			else{
				Vertex v = graph.lines[swc.line_id].hash_vertexes[swc.id];
			}
			v.hash_linked_seg_ids[swc.seg_id] = true; //将点与线段相联系
			if( swc.seg_in_id == 0 ){ //起点
				graph.segments[swc.seg_id].start_id = swc.id;
				if( graph.segments[swc.seg_id].end_id != -1 ){
					v.linked_vertex_ids[graph.segments[swc.seg_id].end_id] = true; //将点与点相联系
					graph.lines[swc.line_id].hash_vertexes[graph.segments[swc.seg_id].end_id].linked_vertex_ids[v.id] = true;
				}
			}else{ //终点
				graph.segments[swc.seg_id].end_id = swc.id;
				if( graph.segments[swc.seg_id].start_id != -1 ){
					v.linked_vertex_ids[graph.segments[swc.seg_id].start_id] = true;
					graph.lines[swc.line_id].hash_vertexes[graph.segments[swc.seg_id].start_id].linked_vertex_ids[v.id] = true;
				}
			}
			graph.lines[swc.line_id].hash_vertexes[swc.id] = v;
		}
		else if( swc.seg_in_id == 1 && swc.pn != -1 ){ //关键点的后一个节点
			graph.lines[swc.line_id].hash_vertexes[swc.pn].hash_linked_seg_ids[swc.seg_id] = true;
			graph.segments[swc.seg_id].segment_vertex_ids.insert({0,graph.hash_swc_ids[swc.pn]});
			graph.segments[swc.seg_id].start_id = swc.pn;
		}
		if( swc.seg_in_id == 1 && swc.seg_size == 2){ //一共只有两个节点
			graph.lines[swc.line_id].hash_vertexes[swc.pn].hash_linked_seg_ids[swc.seg_id] = true;
			graph.lines[swc.line_id].hash_vertexes[swc.pn].linked_vertex_ids[swc.id] = true;
			graph.segments[swc.seg_id].segment_vertex_ids.insert({0,graph.hash_swc_ids[swc.pn]});
			graph.segments[swc.seg_id].start_id = swc.pn;
		}
		AcceptEndOfLine();
		return true;
	}

	inline bool Parser::Accept(char symbol)
	{
		if (symbol == *m_iterator)
		{
			NextSymbol();
			return true;
		}
		return false;
	}

	inline bool Parser::AcceptEndOfLine()
	{
		if (Accept('\n'))
		{
			return true;
		}
		else if (Accept('\r'))
		{
			Accept('\n');
			return true;
		}

		return false;
	}

	inline bool Parser::AcceptWhightSpace()
	{
		if (Accept(' '))
		{
			return true;
		}
		if (Accept('\t'))
		{
			return true;
		}
		return false;
	}

	inline void Parser::NextSymbol()
	{
		if (*m_iterator != '\0')
		{
			++m_iterator;
		}
	}

	inline std::string Parser::GetErrorMessage()
	{
		return m_errorMessage.str();
	}

	inline bool Generator::Write(std::ostream &outStream, const Graph& graph)
	{
		for (std::vector<std::string>::const_iterator it = graph.meta.begin(); it != graph.meta.end(); ++it)
		{
			outStream << "#" << (*it) << '\n';
		}

		for (std::vector<NeuronSWC>::const_iterator it = graph.list_swc.begin(); it != graph.list_swc.end(); ++it)
		{
			char buff[MaxLineSize];
			sprintf(buff, " %lld %d %.15g %.15g %.15g %.7g %lld #name:%s color:%s line_id:%d seg_id:%d seg_size:%d seg_in_id:%d user_id:%d timestamp:%lld\n",
					it->id,
					it->type,
					it->x,
					it->y,
					it->z,
					it->radius,
					it->parent,
					it->name.c_str(),
					it->color.c_str(),
					it->line_id,
					it->seg_id,
					it->seg_size,
					it->seg_in_id,
					it->user_id,
					it->timestamp);
			outStream << buff;
		}

		return true;
	}

	inline bool Generator::Write(std::string& outString, const Graph& graph)
	{
		std::stringstream sstream;
		bool result = Write(sstream, graph);
		outString = sstream.str();
		return result;
	}

	inline bool Generator::WriteToFile(const char *filename, const Graph& graph)
	{
		std::ofstream file(filename, std::ios::binary);

		if (!file.is_open())
		{
			m_errorMessage.clear();
			m_errorMessage << "Error: Can not open file: " << filename << '\n';
			return false;
		}

		std::stringstream sstream;
		bool result = Write(sstream, graph);
		file << sstream.rdbuf();
		return result;
	}
}

#endif