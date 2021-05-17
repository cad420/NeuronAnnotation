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
#include "AnnotationDS.hpp"

namespace SWCP 
{
	using namespace std;
	class Parser
	{
	public:
		// Reads SWC from filespecified by *filename*. 
		// Output is written to *graph*, old content is errased. 
		// If no error have happened returns true
		bool ReadSWCFromFile(const char *filename, NeuronGraph& graph, int type);

		// Reads SWC from filestream. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(std::istream &inStream, NeuronGraph& graph, int type);

		// Reads SWC from string. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(const char *string, NeuronGraph& graph, int type);
		
		// Returns error message for the last parsing if error have happened.
		std::string GetErrorMessage();
	private:
		void NextSymbol();

		vector<std::string> Split(const std::string& in, const std::string& delim);
		bool Accept(char symbol);
		bool AcceptWhightSpace();
		bool AcceptLine(NeuronGraph& graph);
		bool AcceptSWCLine(NeuronGraph& graph);
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
		bool WriteToFile(const char *filename, const NeuronGraph& graph);
		bool Write(std::ostream &outStream, const NeuronGraph& graph);
		bool Write(std::string& outString, const NeuronGraph& graph);
		std::string GetErrorMessage();
	private:
		enum 
		{
			MaxLineSize = 4096
		};
		std::stringstream m_errorMessage;
	};

	inline bool Parser::ReadSWCFromFile(const char *filename, NeuronGraph& graph, int type = 0)
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
		bool result = ReadSWC(content, graph, type);

		delete[] content;

		return result;
	}

	inline bool Parser::ReadSWC(std::istream &inStream, NeuronGraph& graph, int type)
	{
		std::string str;
		char buffer[4096];
		while (inStream.read(buffer, sizeof(buffer)))
		{
			str.append(buffer, sizeof(buffer));
		}
		str.append(buffer, static_cast<unsigned int>(inStream.gcount()));
		return ReadSWC(str.c_str(),graph,type);
	}

	inline bool Parser::ReadSWC(const char *string, NeuronGraph& graph, int type){	
		m_errorMessage.clear();
		graph.list_swc.clear();
		graph.hash_swc_ids.clear();
		graph.lines.clear();
		graph.segments.clear();
		graph.meta.clear();

		m_line = 1;
		m_iterator = string;

		if( type == 0 ){
			while (AcceptSWCLine(graph))
			{
				++m_line;
			}

			if (*m_iterator == '\0')
			{
				graph.formatGraphFromSWCList();
				return true;
			}
			else
			{
				m_errorMessage << "Error at line: " << m_line << ", unexpected symbol:" << *m_iterator << '\n';
				return false;
			}
		}else{
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
	}

	inline vector<std::string> Parser::Split(const std::string& in, const std::string& delim){
		std::regex re{ delim };
		return std::vector<std::string> {
			std::sregex_token_iterator(in.begin(), in.end(), re, -1),
			std::sregex_token_iterator()
		};
	}

	inline bool Parser::AcceptSWCLine(NeuronGraph& graph){
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
		}
		graph.list_swc.push_back(swc);
		graph.hash_swc_ids[swc.id] = graph.list_swc.size()-1;
		if( swc.id > graph.getCurMaxVertexId() ) graph.setMaxVertexId(swc.id);
		if( swc.line_id > graph.getCurMaxLineId() ) graph.setMaxLineId(swc.line_id);
		if( swc.seg_id > graph.getCurMaxSegmentId() ) graph.setMaxSegmentId(swc.seg_id);
		while (AcceptWhightSpace()){}
		AcceptEndOfLine();
		return true;
	}

	inline bool Parser::AcceptLine(NeuronGraph& graph){
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
					if( swc.timestamp == -1 ){
						time_t timestamp;
						swc.timestamp = time(&timestamp);
						std::cout << time(&timestamp) << std::endl;//秒级时间戳
					}
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
			graph.list_swc.push_back(swc);
			return true;
			//return false;
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
				v.timestamp = swc.timestamp;
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
		if( swc.id > graph.getCurMaxVertexId() ) graph.setMaxVertexId(swc.id);
		if( swc.line_id > graph.getCurMaxLineId() ) graph.setMaxLineId(swc.line_id);
		if( swc.seg_id > graph.getCurMaxSegmentId() ) graph.setMaxSegmentId(swc.seg_id);
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

	inline bool Generator::Write(std::ostream &outStream, const NeuronGraph& graph)
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

	inline bool Generator::Write(std::string& outString, const NeuronGraph& graph)
	{
		std::stringstream sstream;
		bool result = Write(sstream, graph);
		outString = sstream.str();
		return result;
	}

	inline bool Generator::WriteToFile(const char *filename, const NeuronGraph& graph)
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