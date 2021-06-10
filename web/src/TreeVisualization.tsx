import {Tabs, Radio, Space, Tooltip, Card, InputNumber, message, Button, Row, Col, Collapse, Menu, Dropdown, Checkbox} from 'antd';
import React from "react";
import * as d3 from "d3";
import $ from "jquery";
const { Panel } = Collapse;
const _SOCKETLINK = "ws://127.0.0.1:12121/info";


const color={
    CHECKED_FORK: "#86DBCB",
    CHECKED_LEAF: "#0992A8",
    CHECKED_FORMAL:"#72DBD4",
    UNCHECKED:"#0B2736"
};

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
        // console.log("visite"+key);
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
        if( parsedStr != "" ) parsedStr = `, "children":[ ` + parsedStr + `]`;
        return `{"name":"` + graph.sub[key].index + `", "checked":` + graph.sub[key].checked + parsedStr + "}";
    }
    // console.log(root,dicMap.get(root))
    let x = dfs(graph,root);
    return x;
}


class TreeVisualization extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            rootIndex : this.props.data.selectedVertexIndex,
            preData: {},
            width: 500,
            height: 300,
            depth:180,
            menuIndex:-1,
            menuVisible: false,
            checked:false,
        };
    }

    componentDidUpdate(){
        // console.log(this.state.preData)
        // console.log(this.props.data)
        if( JSON.stringify(this.state.preData) != JSON.stringify(this.props.data) ){
            new Promise((resolve,reject) => {
                this.setState({preData:this.props.data });
                this.setState({rootIndex:this.props.data.selectedVertexIndex});
                resolve(this.setState({rootIndex:this.props.data.selectedVertexIndex}));
            }).then(()=>{
                this.TreePlot();
            })            
        }
    }

    componentDidMount() {
        if( this.props.data.graphs && this.props.data.graphs[this.props.selectedMapKey].sub[this.props.selectedVertexKey] ){
            new Promise((resolve,reject) => {
                this.setState({preData:this.props.data });
                this.setState({rootIndex:this.props.data.selectedVertexIndex});
                resolve(this.setState({rootIndex:this.props.data.selectedVertexIndex}));
            }).then(()=>{
                this.TreePlot();
            })            
        }
    }

    
    TreePlot = () => {
        const self = this;
        d3
        .select("#Rectangle")
        .select("svg")
        .selectAll("*")
        .remove()
        //console.log(this.props.data.graphs[this.props.selectedMapKey].sub);
        if(this.props.data.graphs.length != 0 && this.props.data.graphs[this.props.selectedMapKey].sub ){
            if( ! this.props.data.graphs[this.props.selectedMapKey].sub ) return;
            // console.log(getTreeData( this.props.data.graphs[this.props.selectedMapKey] ,self.state.rootIndex));
            var treeData = JSON.parse(getTreeData( this.props.data.graphs[this.props.selectedMapKey] ,self.state.rootIndex));


            // Set the dimensions and margins of the diagram
            var margin = ({top: 10, right: 50, bottom: 10, left: 50});

            self.setState({width:1200 - margin.left - margin.right}) ;
            self.setState({height:300 - margin.top - margin.bottom});

            var svg = d3.select("#Rectangle").select("svg")
            .attr("width", self.state.width + margin.right + margin.left)
            .attr("height", self.state.height + margin.top + margin.bottom)
            .append("g")
            .attr("transform", "translate("
                + margin.left + "," + margin.top + ")");

            const fontStyle = "15px -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, Helvetica, Arial, sans-serif"
            var div = d3.select("body").append("div")
            .attr("class", "svg-tooltip")
            .style("visibility", "hidden")
            .style("position", "absolute")
            .style("z-index", "10")
            .style("background", "#fff")
            .text("")
            .style("padding", "3px")
            .style("border", "0.5px solid black")
            .style("font", fontStyle);
            // .style("opacity", 1e-6)
            // append the svg object to the body of the page
            // appends a 'group' element to 'svg'
            // moves the 'group' element to the top left margin

            var i = 0,
                duration = 750;

            // declares a tree layout and assigns the size
            var treemap = d3.tree().size([self.state.height, self.state.width]);

            var root;
            // Assigns parent, children, height, depth
            root = d3.hierarchy(treeData, function(d) { return d.children; });
            root.x0 = self.state.height / 2;
            root.y0 = 0;

            console.log(self.state.rootIndex);

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

            function mouseover() {
                div.transition()
                  .duration(300)
                  .style("visibility", "visible");
                //   .style("opacity", 1);
              }
            
              function mousemove(event, data) {
                var d = data;
                div
                .html(function(){
                   if( !d.data.children || (d.data.name == root.data.name && d.data.children.length == 1) ){
                        return "LEAF" + ":  " + d.data.name;
                   }else if(d.data.children.length == 1){
                        return "NORMAL" + ":  " + d.data.name;
                   }else
                        return "FORK" + ":  " + d.data.name;
                })
                .style("left", (event.pageX+10) + "px")
                .style("top", (event.pageY-10) + "px");
            }
            
            function mouseout() {
                div.transition()
                  .duration(300)
                  .style("visibility", "hidden");
                
                
            //     div.transition()
            //       .duration(300)
            //       .style("opacity", 1e-6);
              }

            function update(source) {

            // Assigns the x and y position for the nodes
            var treeData = treemap(root);

            // Compute the new tree layout.
            var nodes = treeData.descendants(),
                links = treeData.descendants().slice(1);

            // Normalize for fixed-depth.
                nodes.forEach(function (d) {
                    d.y = d.depth * 180;
                    d.y = d.depth * self.state.depth;
                    if (d.y > self.state.width) {
                        self.setState({width:d.y + 100});
                    }
                });
                d3.select("#Rectangle").select("svg")
                .attr("width", self.state.width + margin.right + margin.left)
                .attr("height", self.state.height + margin.top + margin.bottom)
            // ****************** Nodes section ***************************

            // Update the nodes...
            var node = svg.selectAll('g.node')
                .data(nodes, function(d) {return d.id || (d.id = ++i); });

            const onClickJump = (e,d) => {
                self.setState({menuVisible:false});
                div.transition()
                .duration(300)
                .style("visibility", "hidden");
                self.props.data.graphs[self.props.selectedMapKey].sub.forEach( function(v,i) {
                    console.log(v.index);
                    console.log(d.id);
                    if( v.index == d.data.name ){
                        let record = {
                            index :d.data.name,
                            key:i
                        }
                        self.props.onClickJumpToVex(record);
                    }
                })
                
            }
            var menu = (e,i) =>{
                console.log(e);
                console.log(i);
                self.setState({menuIndex:i.data.name,menuVisible:true,checked:i.data.checked});
            }
    
            // Enter any new modes at the parent's previous position.
            var nodeEnter = node.enter().append('g')
                .attr('class', 'node')
                .attr("transform", function(d) {
                    return "translate(" + source.y0 + "," + source.x0 + ")";
                })
                .on('click', click)
                .on('dblclick',onClickJump)
                .on("mouseover", mouseover)
                .on("mousemove", function(event, d){
                mousemove(event, d);})
                .on("mouseout", mouseout)
                .on("contextmenu", menu)

            // Add Circle for the nodes
            nodeEnter.append('circle')
                .attr('class', 'node')
                .attr('r', 1e-6)
                .attr("stroke", function(d){
                    if (d.data.checked){
                        if( !d.data.children || (d.data.name == root.data.name && d.data.children.length == 1) ){
                            return color.CHECKED_LEAF;
                        }
                        else if(d.data.children.length == 1 ){
                            return color.CHECKED_FORMAL;
                        }
                        else{
                            return color.CHECKED_FORK;
                        }
                    }
                    else{
                        return color.UNCHECKED;
                    }
                })
                .attr("stroke-width",2)
                .style("fill", function(d){
                    if (d.data.checked){
                        if( !d.data.children || (d.data.name == root.data.name && d.data.children.length == 1) ){
                            return color.CHECKED_LEAF;
                        }
                        else if(d.data.children.length == 1 ){
                            return color.CHECKED_FORMAL;
                        }
                        else {
                            return color.CHECKED_FORK;
                        }
                    }
                    else{
                        return color.UNCHECKED;
                    }
                });
                
                // function(d) {
                //     return d._children ? "#B0C4DE" : "#fff";
                // });


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
                        // console.log("root"+d.data.name);
                        return "Root";
                    }
                    else if(d.data.name == self.props.data.selectedVertexIndex)
                    return "Selected:" + d.data.name;
                    else
                    return "";
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
                .attr('r', 7)
                .style("fill",function(d){
                    if( !d.data.children || (d.data.name == root.data.name && d.data.children.length == 1) ){
                        return color.CHECKED_LEAF;
                    }
                    else if(d.data.children.length == 1 ){
                        return color.CHECKED_FORMAL;
                    }
                    else{
                        return color.CHECKED_FORK;
                    }
                })
                // .style("fill", function(d) {
                //     return d._children ? "#B0C4DE" : "#ffffff";
                // })
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
            function click(e,d) {
                
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
            const expand = (d) => {   
                var children = (d.children)?d.children:d._children;
                if (d._children) {        
                    d.children = d._children;
                    d._children = null;       
                }
                if(children)
                  children.forEach(expand);
            }

            const expendAll = () => {
                expand(root);
                update(root);
            }
            expendAll();
        }

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


    // 事件监听方式添加事件绑定
    
    handleMenuClick = ( event ) => {
        
        let key = event.key;
        console.log(key);
        let self = this;
        if( key == '1' ){
            console.log(self.state.menuIndex);
            self.props.data.graphs[self.props.selectedMapKey].sub.forEach( function(v,i) {
            if( v.index == self.state.menuIndex ){
                let record = {
                    index :Number(self.state.menuIndex),
                    key:i
                }
                self.props.onClickJumpToVex(record);
            }
            })
        }else if( key == '2' ){
            let self = this;
            const ws = new WebSocket(_SOCKETLINK);
            ws.onopen = () => {
                console.log("连接成功，准备发送更新数据");
                console.log("self.state.menuIndex"+self.state.menuIndex);
                ws.send(
                    JSON.stringify({
                        modify : {
                        index : Number(self.state.menuIndex),
                        checked : !self.state.checked
                        }
                    })
                );
                console.log(                    JSON.stringify({
                    modify : {
                    index : Number(self.state.menuIndex),
                    checked : !self.state.checked
                    }
                }))
            };
            ws.onmessage = (msg) => {
                const {data} = msg;
                ws.close();
                try {
                    const obj = JSON.parse(data);
                    if (obj.type == "error") {
                        console.log(obj.message);
                        message.error(obj.message);
                    } else if (obj.type == "success") {
                        console.log(obj.message);
                        message.success(obj.message);
                    } else {
                        let p = new Promise(resolve => {
                            resolve(self.props.setData(obj));
                        }).then(() => {
                            self.props.initSelectedKey();
                        })
                    }
                } catch  {
                    console.log(data);
                }
            }
        }else if( key == '3' ){
            this.onChange(this.state.menuIndex);
        }
        this.setState({menuVisible:false});
    }

    render() {

        const self = this;
        const menu = (
            <Menu onClick={self.handleMenuClick}>
              <Menu.Item key="1">跳转</Menu.Item>
              <Menu.Item key="2">修改检查状态</Menu.Item>
              <Menu.Item key="3">以该顶点为根重置</Menu.Item>
            </Menu>
          );
        return (
            <>
            <div>
            <Collapse defaultActiveKey={['1']}>
                <Panel header="二维可视化" key="1">
                <Row>
                    <Col>
                    选择端点Index： <InputNumber defaultValue={this.props.data.selectedVertexIndex} onChange={(v)=>this.onChange(v)} />
                    </Col>
                </Row>
                <Dropdown overlay={menu} trigger={['contextMenu']} visible={self.state.menuVisible}>
                <div className="Rectangle" id="Rectangle" ref="Rectangle" onClick={()=>self.setState({menuVisible:false})}>
                    <div className="wuliwala">
                            <svg></svg>
                    </div>
                </div>
                </Dropdown>
                </Panel>
            </Collapse>
            </div>
    </>
        );
    }
}

export default TreeVisualization;