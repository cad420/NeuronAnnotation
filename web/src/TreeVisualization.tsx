import {Tabs, Radio, Space, Tooltip, Card, InputNumber, message} from 'antd';
import React from "react";
import * as lw from "@euphrasiologist/lwphylo";
import * as d3 from "d3";
import { subset } from 'd3';

const {TabPane} = Tabs;
const w = 1024;
const h = 200;
const scaleRect = 15;
const xScaleRect = d3
    .scaleLinear()
    .domain([
        -scaleRect * 0.03,
        scaleRect * 0.5
    ])
    .range([0, w]);
const yScaleRect = d3
    .scaleLinear()
    .domain([
        -scaleRect * 0.1,
        scaleRect * 0.5
    ])
    .range([h,0]);
const scaleUnroot = 3.5;
const xScaleUnroot = d3
    .scaleLinear()
    .domain([
        -scaleUnroot,
        scaleUnroot
    ])
    .range([0, w]);
const yScaleUnroot = d3
    .scaleLinear()
    .domain([
        -scaleUnroot,
        scaleUnroot
    ])
    .range([h, 0]);

//数据部分
interface DataType {
    key: number;
    name: String;
    color: String;
    status: Boolean;
    index: number;
    sub: Array<SubDataType>;
}
    
interface SubDataType {
    key: number;
    index: number;
    name: String;
    arc: Array<ArcType>;
    lastEditTime: String;
}

interface ArcType{
    tailVex: number;
    headVex: number;
    distance: number;
}

const GraphToNewick = ( graph:DataType, pkey:number ) =>{

    //console.log(graph);
    // var maxLength = 0;
    // if( pkey == -1 ) return "";
    // for( let i = 0 ; i < graph.sub.length ; i ++ ){
    //     for( let j = 0 ; j < graph.sub[i].arc.length ; j ++ ){
    //         if( maxLength < graph.sub[i].arc[j].distance ){
    //             maxLength = graph.sub[i].arc[j].distance; //找到最大值，权重为1
    //         }
    //     }
    // }
    // let dicMap = new Map(); //index和key转换
    // var visitedArray = new Array(graph.sub.length);
    // for( let i = 0; i < graph.sub.length ; i ++ ){
    //     visitedArray[i] = false;
    //     dicMap.set(graph.sub[i].index,i); //服务器的index,对应客户端的key
    // }
    // const dfs = (graph:DataType,key:number,length:number) =>{
    //     let parsedStr = "";
    //     console.log("visite:",key);
    //     visitedArray[key] = true;
    //     for( let i = 0 ; i < graph.sub[key].arc.length ; i ++ ){
    //         if( graph.sub[key].arc[i].headVex == graph.sub[key].index ){
    //             let vexKey = dicMap.get(graph.sub[key].arc[i].tailVex);
    //             if( !visitedArray[vexKey] ){
    //                 let subStr = dfs(graph,vexKey,graph.sub[key].arc[i].distance);
    //                 if( parsedStr.charAt(parsedStr.length - 1) >= '0' && parsedStr.charAt(parsedStr.length - 1) <= '9' ) 
    //                     parsedStr = parsedStr + "," + subStr;
    //                 else{
    //                     parsedStr = parsedStr + subStr;
    //                 }
    //             }
    //         } else if( graph.sub[key].arc[i].tailVex == graph.sub[key].index ){
    //             let vexKey = dicMap.get(graph.sub[key].arc[i].headVex);
    //             if( !visitedArray[vexKey] ){ 
    //                 let subStr = dfs(graph,vexKey,graph.sub[key].arc[i].distance);
    //                 if( parsedStr.charAt(parsedStr.length - 1) >= '0' && parsedStr.charAt(parsedStr.length - 1) <= '9' ) 
    //                     parsedStr = parsedStr + "," + subStr;
    //                 else{
    //                     parsedStr = parsedStr + subStr;
    //                 }
    //             }
    //         }
    //     }
    //     if( parsedStr == "" ){
    //         return key + ":" + length/maxLength;
    //     }
    //     if( length == 0 ){ //最后一个顶点
    //         return "(" + parsedStr + ")" ;
    //     }
    //     else {
    //         return "(" + parsedStr + ")" + key + ":" + length/maxLength;
    //     }
    // }

    // console.log(dicMap.get(pkey));
    // let x = dfs(graph,dicMap.get(pkey),0);
    // console.log(x);
    // return x;
}

const getTreeData = ( graph:DataType, root:number ) =>{
    //console.log(graph);
    var maxLength = 0;
    for( let i = 0 ; i < graph.sub.length ; i ++ ){
        for( let j = 0 ; j < graph.sub[i].arc.length ; j ++ ){
            if( maxLength < graph.sub[i].arc[j].distance ){
                maxLength = graph.sub[i].arc[j].distance; //找到最大值，权重为1
            }
        }
    }
    let dicMap = new Map(); //index和key转换
    var visitedArray = new Array(graph.sub.length);
    for( let i = 0; i < graph.sub.length ; i ++ ){
        visitedArray[i] = false;
        dicMap.set(graph.sub[i].index,i); //服务器的index,对应客户端的key
        if(graph.sub[i].index == root ){
            root = i
        }
    }
    const dfs = (graph:DataType,key:number) =>{
        if( !graph.sub[key] ) return "";
        let parsedStr = "";
        console.log("visite"+key);
        visitedArray[key] = true;
        for( let i = 0 ; i < graph.sub[key].arc.length ; i ++ ){
            if( graph.sub[key].arc[i].headVex == graph.sub[key].index ){
                let vexKey = dicMap.get(graph.sub[key].arc[i].tailVex);
                if( !visitedArray[vexKey] ){
                    let subStr = dfs(graph,vexKey);
                    if( parsedStr.charAt(parsedStr.length - 1) == '}' ) 
                        parsedStr = parsedStr + "," + subStr;
                    else{
                        parsedStr = parsedStr + subStr;
                    }
                }
            } else if( graph.sub[key].arc[i].tailVex == graph.sub[key].index ){
                let vexKey = dicMap.get(graph.sub[key].arc[i].headVex);
                if( !visitedArray[vexKey] ){ 
                    let subStr = dfs(graph,vexKey);
                    if( parsedStr.charAt(parsedStr.length - 1) == '}' ) 
                        parsedStr = parsedStr + "," + subStr;
                    else{
                        parsedStr = parsedStr + subStr;
                    }
                }
            }
        }
        if( parsedStr != "" ) parsedStr = `, "children":[ ` + parsedStr + `]`
        return `{"name":"` + graph.sub[key].index + `"` + parsedStr + "}";
    }
    console.log(root,dicMap.get(root))
    let x = dfs(graph,root);
    return x;
}


class TreeVisualization extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            rootIndex : this.props.data.selectedVertexIndex,
            preData : {}
        };
    }

    componentDidUpdate(){
        console.log(this.state.preData)
        console.log(this.props.data)
        if( JSON.stringify(this.state.preData) != JSON.stringify(this.props.data) ){
            this.TreePlot();
            this.setState({preData:this.props.data });
        }
        
    }

    componentDidMount() {
        this.TreePlot()
        this.setState({preData:this.props.data});
        // if(this.props.data.graphs[this.props.selectedMapKey].sub[this.props.selectedVertexKey]){
        //     this.RectPhyloPlot();
        //     this.UnrootedPhyloPlot();
        // }
    }

    TreePlot = () => {
        const self = this;
        console.log(this.props.data.graphs[this.props.selectedMapKey].sub);
        if(this.props.data.graphs.length != 0 && this.props.data.graphs[this.props.selectedMapKey].sub ){
            var treeData = JSON.parse(getTreeData( this.props.data.graphs[this.props.selectedMapKey] ,self.state.rootIndex));

            // Set the dimensions and margins of the diagram
            var margin = ({top: 50, right: 100, bottom: 50, left: 50});
            //var margin = {top: 20, right: 90, bottom: 30, left: 90},
            var width = 1200 - margin.left - margin.right;
            var height = 300 - margin.top - margin.bottom;

            // append the svg object to the body of the page
            // appends a 'group' element to 'svg'
            // moves the 'group' element to the top left margin
            d3
                .select("#Rectangle")
                .select("svg")
                .selectAll("*")
                .remove()

            var svg = d3.select("#Rectangle").select("svg")
                .attr("width", width + margin.right + margin.left)
                .attr("height", height + margin.top + margin.bottom)
                .append("g")
                .attr("transform", "translate("
                    + margin.left + "," + margin.top + ")");

            var i = 0,
                duration = 750;

            // declares a tree layout and assigns the size
            var treemap = d3.tree().size([height, width]);

            var root;
            // Assigns parent, children, height, depth
            root = d3.hierarchy(treeData, function(d) { return d.children; });
            root.x0 = height / 2;
            root.y0 = 0;

            console.log(this.state.rootIndex);

            // Collapse after the second level
            root.children.forEach(collapse);

            update(root);

            // Collapse the node and all it's children
            function collapse(d) {
            if(d.children) {
                d._children = d.children
                d._children.forEach(collapse)
                d.children = null
            }
            }

            function update(source) {

            // Assigns the x and y position for the nodes
            var treeData = treemap(root);

            // Compute the new tree layout.
            var nodes = treeData.descendants(),
                links = treeData.descendants().slice(1);

            // Normalize for fixed-depth.
            nodes.forEach(function(d){ d.y = d.depth * 180});

            // ****************** Nodes section ***************************

            // Update the nodes...
            var node = svg.selectAll('g.node')
                .data(nodes, function(d) {return d.id || (d.id = ++i); });

            // Enter any new modes at the parent's previous position.
            var nodeEnter = node.enter().append('g')
                .attr('class', 'node')
                .attr("transform", function(d) {
                    return "translate(" + source.y0 + "," + source.x0 + ")";
                })
                .on('click', click);

            // Add Circle for the nodes
            nodeEnter.append('circle')
                .attr('class', 'node')
                .attr('r', 1e-6)
                .attr("stroke","black")
                .attr("stroke-width",2)
                .style("fill", function(d) {
                    return d._children ? "#B0C4DE" : "#fff";
                });



            // Add labels for the nodes
            nodeEnter.append('text')
                .attr("dy", ".35em")
                .attr("x", function(d) {
                    return d.children || d._children ? -13 : 13;
                })
                .attr("text-anchor", function(d) {
                    return d.children || d._children ? "end" : "start";
                })
                .text(function(d) {
                    if(d.data.name == root.data.name){
                        console.log("root"+d.data.name);
                        return "Root";
                    }
                    else if(d.data.name == self.props.data.selectedVertexIndex)
                    return "Selected:" + d.data.name;
                    else
                    return "Index:" + d.data.name;
                    });

            // UPDATE
            var nodeUpdate = nodeEnter.merge(node);

            // Transition to the proper position for the node
            nodeUpdate.transition()
                .duration(duration)
                .attr("transform", function(d) { 
                    return "translate(" + d.y + "," + d.x + ")";
                });

            // Update the node attributes and style
            nodeUpdate.select('circle.node')
                .attr('r', 10)
                .style("fill", function(d) {
                    return d._children ? "#B0C4DE" : "#ffffff";
                })
                .attr('cursor', 'pointer');


            // Remove any exiting nodes
            var nodeExit = node.exit().transition()
                .duration(duration)
                .attr("transform", function(d) {
                    return "translate(" + source.y + "," + source.x + ")";
                })
                .remove();

            // On exit reduce the node circles size to 0
            nodeExit.select('circle')
                .attr('r', 1e-6);

            // On exit reduce the opacity of text labels
            nodeExit.select('text')
                .style('fill-opacity', 1e-6);

            // ****************** links section ***************************

            // Update the links...
            var link = svg.selectAll('path.link')
                .data(links, function(d) { return d.id; });

            // Enter any new links at the parent's previous position.
            var linkEnter = link.enter().insert('path', "g")
                .attr("class", "link")
                .attr('d', function(d){
                    var o = {x: source.x0, y: source.y0}
                    return diagonal(o, o)
                });

            // UPDATE
            var linkUpdate = linkEnter.merge(link);

            // Transition back to the parent element position
            linkUpdate.transition()
                .duration(duration)
                .attr('d', function(d){ return diagonal(d, d.parent) });

            // Remove any exiting links
            var linkExit = link.exit().transition()
                .duration(duration)
                .attr('d', function(d) {
                    var o = {x: source.x, y: source.y}
                    return diagonal(o, o)
                })
                .remove();

            // Store the old positions for transition.
            nodes.forEach(function(d){
                d.x0 = d.x;
                d.y0 = d.y;
            });

            // Creates a curved (diagonal) path from parent to the child nodes
            function diagonal(s, d) {

                let path = `M ${s.y} ${s.x}, ${d.y} ${d.x}`

                return path
            }

            // Toggle children on click.
            function click(i,d) {
                console.log(d);
                if (d.children) {
                    d._children = d.children;
                    d.children = null;
                } else {
                    d.children = d._children;
                    d._children = null;
                }
                update(d);
            }
            }
        }
    }

    RectPhyloPlot = () => {
        const treeString = GraphToNewick(this.props.data.graphs[this.props.selectedMapKey],this.props.selectedVertexKey);
        const parsedTree = lw.readTree(treeString);
        const rectPhylo = lw.rectangleLayout(parsedTree);
        const self = this;

        d3
        .select("#Rectangle")
        .select("svg")
        .selectAll("*")
        .remove()
    
        const svg = d3
            .select("#Rectangle")
            .select("svg")
            .attr("width", w)
            .attr("height", h)
            .attr("font-family", "sans-serif")
            .attr("font-size", 10);

        // create a grouping variable
        const group = svg.append('g');

        const stroke_width = 3;
        // draw horizontal lines
        group
            .append('g')
            .attr('class', 'phylo_lines')
            .selectAll('lines')
            .data(rectPhylo.data)
            .join('line')
            .attr('class', 'lines')
            .attr('x1', d => xScaleRect(d.x0) - stroke_width / 2)
            .attr('y1', d => yScaleRect(d.y0))
            .attr('x2', d => xScaleRect(d.x1) - stroke_width / 2)
            .attr('y2', d => yScaleRect(d.y1))
            .attr('stroke-width', stroke_width)
            .attr('stroke', this.props.data.graphs[this.props.selectedMapKey].color); //线段上色

        // draw vertical lines
        group
            .append('g')
            .attr('class', 'phylo_lines')
            .selectAll('lines')
            .data(rectPhylo.vertical_lines)
            .join('line')
            .attr('class', 'lines')
            .attr('x1', d => xScaleRect(d.x0))
            .attr('y1', d => yScaleRect(d.y0))
            .attr('x2', d => xScaleRect(d.x1))
            .attr('y2', d => yScaleRect(d.y1))
            .attr('stroke-width', stroke_width)
            .attr('stroke', this.props.data.graphs[this.props.selectedMapKey].color); //线段上色

        // draw nodes
        group
            .append('g')
            .attr('class', 'phylo_points')
            .selectAll('.dot')
            // remove rogue dot.
            .data(rectPhylo.data.filter(d => d.x1 > 0))
            .join('circle')
            .attr('class', 'dot')
            .attr('r', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 6;
                else return 4;
            })
            .attr('cx', d => xScaleRect(d.x1))
            .attr('cy', d => yScaleRect(d.y1))
            .attr('stroke', function(d){
              if (d.thisLabel == self.props.selectedVertexKey)
                return 'red';
              else return 'black';
            })
            .attr('stroke-width', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 3;
                if (d.isTip) {
                    return 2;
                } else {
                    return 1;
                }
            })
            .attr('fill', 'white');

        group.selectAll('.dot')
            .on("mouseover", (d,i) => {
                console.log(d)
            d3.select("#tooltip").remove();
            d3.select("#Rectangle")
                .select("svg")
                .append("text")
                .attr("x", d.layerX)
                .attr("y", d.layerY-18)
                .attr("id", "tooltip")
                .attr("class","tooltip")
                .attr("text-anchor", "middle")
                .attr("font-size", "13px")
                .text( function(){
                    let num = i.thisLabel ? i.thisLabel : 0;
                    return "跳转至" + num + "号点";
                });
        })
        .on("click",(d,i)=>{
            let record = {
                index:self.props.data.graphs[self.props.selectedMapKey].sub[i.thisLabel].index,
                key:i.thisLabel
            };
            self.props.onClickJumpToVex(record);
        })
        .on("mouseout", (d) => {
            d3.select("#tooltip").remove();
        })

    }

    UnrootedPhyloPlot = () => {
        const treeString = GraphToNewick(this.props.data.graphs[this.props.selectedMapKey],this.props.selectedVertexKey);
        const parsedTree = lw.readTree(treeString);
        const unrootedPhylo = lw.unrooted(parsedTree);
        const self = this;
        d3
            .select("#EqualAngle")
            .select("svg")
            .selectAll("*")
            .remove()
        
        const svg = d3
            .select("#EqualAngle")
            .select("svg")
            .attr("width", w)
            .attr("height", h)
            .attr("font-family", "sans-serif")
            .attr("font-size", 10);

        var group = svg.append("g");

        // draw lines
        group
            .append("g")
            .attr("class", "phylo_lines")
            .selectAll("lines")
            .data(unrootedPhylo.edges)
            .enter()
            .append("line")
            .attr("class", "lines")
            .attr("x1", d => xScaleUnroot(d.x1))
            .attr("y1", d => yScaleUnroot(d.y1))
            .attr("x2", d => xScaleUnroot(d.x2))
            .attr("y2", d => yScaleUnroot(d.y2))
            .attr("stroke-width", 3)
            .attr("stroke", self.props.data.graphs[self.props.selectedMapKey].color);

        // draw points
        group
            .append("g")
            .attr("class", "phylo_points")
            .selectAll(".dot")
            .data(unrootedPhylo.data)
            .enter()
            .append("circle")
            .attr("class", "dot")
            .attr("r", function (d) {
              if (d.thisLabel == self.props.selectedVertexKey) return 6;
              else return 4;
            })
            .attr("cx", d => xScaleUnroot(d.x))
            .attr("cy", d => yScaleUnroot(d.y))
            .attr('stroke', function(d){
              if (d.thisLabel == self.props.selectedVertexKey)
                return 'red';
              else return 'black';
            })
            .attr('stroke-width', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 3;
                if (d.isTip) {
                    return 2;
                } else {
                    return 1;
                }
            })
            .attr('fill', 'white')
            
        group.selectAll('.dot')
            .on("mouseover", (d,i) => {
                console.log(d)
            d3.select("#tooltip").remove();
            d3.select("#EqualAngle")
                .select("svg")
                .append("text")
                .attr("x", d.layerX)
                .attr("y", d.layerY-18)
                .attr("id", "tooltip")
                .attr("class","tooltip")
                .attr("text-anchor", "middle")
                .attr("font-size", "13px")
                .text( function(){
                    let num = i.thisLabel ? i.thisLabel : 0;
                    return "跳转至" + num + "号点";
                });
        })
        .on("click",(d,i)=>{
            let record = {
                index:self.props.data.graphs[self.props.selectedMapKey].sub[i.thisLabel?i.thisLabel:0].index,
                key:i.thisLabel ? i.thisLabel : 0
            };
            self.props.onClickJumpToVex(record);
        })
        .on("mouseout", (d) => {
            d3.select("#tooltip").remove();
        })

    }

    onChange = (value) => {
        console.log("哦耶！")
        const graph = this.props.data.graphs[this.props.selectedMapKey];
        for( let i = 0; i < graph.sub.length ; i ++ ){
            if( value == graph.sub[i].index ){
                this.setState( {rootIndex:value} )
                message.success("设置成功！");
                this.TreePlot();
                return
            }
        }
        message.error("不存在该节点，请重新输出！");
    }


    render() {

        
        return (
            <>
            <div>
                <p>选择端点Index</p>
                <InputNumber defaultValue={this.props.data.selectedVertexIndex} onChange={(v)=>this.onChange(v)} />
                <div className="Rectangle" id="Rectangle" ref="Rectangle">
                    <svg></svg>
                </div>
            </div>
    </>
        );
    }
}

export default TreeVisualization;