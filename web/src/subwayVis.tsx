import { InputNumber, message,  Row, Col, Collapse, Menu, Dropdown, Switch, Space } from 'antd';
import React from "react";
import * as d3 from "d3";
import { Graph }  from "./format";
const { Panel } = Collapse;
const _SOCKETLINK = "ws://127.0.0.1:12121/info";

const color={
    CHECKED_FORK: "#86DBCB",
    CHECKED_LEAF: "#0992A8",
    CHECKED_FORMAL:"#72DBD4",
    UNCHECKED:"#0B2736"
};

//服务器端index映射到数组的索引
const indexMap = new Map();
const expandMap = new Map();
const leafMap = new Map();

/* 处理图的数据结构，给定根节点，将其转化成树 */
const graph2Tree = (graph:Graph, key:number, firstPlot: boolean)  => {
    indexMap.clear();
    if (!graph || !graph.sub || graph.sub.length === 0) return undefined;
    const visited = new Array(graph.sub.length).fill(false);
    let root = undefined;
    for (let i = 0; i < graph.sub.length; i++) {
        const { index } = graph.sub[i];
        indexMap.set(index, i);
        if(firstPlot) expandMap.set(index, true);
        if (index === key) {
            root = i; //记录根节点
        }
    }
    if (root === undefined) return;

    // dfs图, 生成树的结构
    const dfs = (v:number) => {
        visited[v] = true;
        const tree = {};
        const { checked, index } = graph.sub[v];
        Object.assign(tree, { index, checked, children: [] });
        let otherVex;
        // 遍历孩子节点
        const arcs = [];
        const children = [];
        for (let i = 0; i < graph.sub[v].arc.length; i++) {
            const arc = graph.sub[v].arc[i];
            const { distance } = arc;
            otherVex = (arc.headVex === index ? arc.tailVex : arc.headVex);
            const otherIndex = indexMap.get(otherVex);
            if (!visited[otherIndex]) {
                children.push(dfs(otherIndex));
                arcs.push({ distance, critical: false });
            }
        }
        Object.assign(tree, { arcs, children });
        return tree;
    }

    // 计算某个点分支了几层, 用于stroke-width的计算
    const assignBranchCount = (tree: any, branchCount: number) => {
        let branch = tree.children.length > 1;
        const temp = branch ? branchCount + 1 : branchCount;
        tree.branchCount = temp;
        for (let i = 0; i < tree.children.length; i++) {
            assignBranchCount(tree.children[i], temp);
        }
    }

    const tree = dfs(root);
    assignBranchCount(tree, 0);
    console.log(tree);
    return tree;
}

//dp计算树上某点到其叶节点的最长距离，记忆化存储
const longestPath = new Map(); 
// 记录深度Map
const depthMap = new Map();
//树的最大高度
let maxDepth = -1; 

//给定一棵树，计算出每个节点到其叶子节点的最长距离, 并附加额外信息用于绘制(dp)
const getLongestPath = (tree: any, depth: number) => {
    const { index: key } = tree;
    depthMap.set(key, depth);
    const longest = longestPath.get(key);
    if (longest !== undefined ) return longest; //memory
    tree.depth = depth;
    // 叶子节点需要更新最大高度，返回最大距离为0
    if (tree.children.length === 0) {
        longestPath.set(key, 0);
        if (depth > maxDepth) {
            maxDepth = depth;
        }
        return 0;
    } 
    let longestDis = -1;
    let longestIndex = -1;
    for (let i = 0; i < tree.children.length; i++) {
        let currentDis = tree.arcs[i].distance + getLongestPath(tree.children[i], depth + 1);
        if (currentDis > longestDis) {
            longestDis = currentDis;
            longestIndex = i;
        }
    }
    tree.arcs[longestIndex].critical = true;
    longestPath.set(key, longestDis);
    return longestDis;
}

/* 绘制地铁图所需参数 */
const svgWidth = 800;  //SVG默认宽度, 实际取屏幕宽度
let svgHeight = 400;  //SVG高度
const horizontalPadding = 60; //水平padding
let verticalPadding = svgHeight * 0.02; //垂直padding

const visOptions = {
    svgWidth,
    svgHeight,
    padding: {
        horizontal: horizontalPadding,
        vertical: verticalPadding
    },
    startX: horizontalPadding / 2,
    startY: svgHeight / 2,
    circleFillColor: "#ADD8E6", //lightblue
    defaultRadius: 4,
    branchRate: 0.5, // 分支偏移不能大于上一个分支偏移的比例
    depthRate: 0.975, // 层数对应的偏移衰减
    radiusRate: 1,
    strokeWidthRate: 0.96, //troke-width衰减率
    initialPolyLineStrokeWidth: 2.5,
    circleStrokeWidth: 2,
    textFontSize: 11,
    upBound: verticalPadding,  //上边界
    downBound: svgHeight - verticalPadding, //下边界
    timeOut: 100 //绘制函数防抖延迟
};

const reCalSvgHeight = function(h: number) {
    verticalPadding = h * 0.02;
    svgHeight = h;
    visOptions.svgHeight = h;
    visOptions.startY = h / 2;
    visOptions.downBound = h - verticalPadding;
    visOptions.upBound = verticalPadding;
}

let polyLineStrokeWidth: number = visOptions.initialPolyLineStrokeWidth; //线段宽度

const scales = [1, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 100000];
const pixels = [20, 100];
// 将上述距离映射到20px-100px上(比例尺)

const coordinateMap = new Map();    //key与坐标的Map
const radiusMap = new Map();
const colorMap = new Map(); //key与颜色的Map
let circles: Array<[number, number, number]> = [];  //记录点的坐标用于画圆
let collideArr : Array<number[]> = [];  //碰撞检测数组, 记录svg上存在的线段 [Y, startX, endX]
let offsetMap = new Map();  //分支偏移Map,主干为0, 并记录偏移方向 [offset, dir]
let dirMap = new Map(); // 同一主干上的单分支交错生长(上下交替)


// 绘制的初始化函数
const init = () => {
    maxDepth = -1;
    circles = [];
    collideArr = [];
    collideArr.push([visOptions.upBound, 0, visOptions.svgWidth], [visOptions.startY, 0, visOptions.svgWidth], [visOptions.downBound, 0, visOptions.svgWidth]);
    longestPath.clear();
    coordinateMap.clear();
    radiusMap.clear();
    colorMap.clear();
    leafMap.clear();
    depthMap.clear();
    offsetMap.clear();
    dirMap.clear();
    polyLineStrokeWidth = visOptions.initialPolyLineStrokeWidth;
    d3.selectAll(".subway > *").remove(); //清除画布元素
}

/* 工具函数 获取标注点颜色 */
const getColor = (checked: boolean, children: number) => {
    if (!checked) {
        return color.UNCHECKED;
    } else if (children === 0) {
        return color.CHECKED_LEAF;
    } else if (children === 1) {
        return color.CHECKED_FORMAL;
    } else {
        return color.CHECKED_FORK;
    }
}

// 绘制函数防抖处理
function debounce(func: Function, interval: number) {
    let timer: any = undefined;
    return function(this: any) {
        let context = this;
        let args = arguments;

        if(timer) {
            clearTimeout(timer);
        }
        timer = setTimeout(() => {
            func.apply(context, args);
        }, interval);
    }
}

interface Props {
    data: any,
    onClickJumpToVex: Function,
    setData: Function,
    initSelectedKey: Function,
    selectedMapKey: number,
    selectedVertexKey: number
}

interface State {
    rootIndex: number,
    preData: {},
    width: number,
    height: number,
    depth: number,
    menuIndex: number,
    menuVisible: boolean,
    checked: boolean,
    svgRef: any,
    pathColor: string,  //路径颜色
    indexVisible: boolean, //是否显示标注点idx
    circles: Array<[number, number, number]> //记录标注点坐标 

}

const isIntersect = (a1: number, a2: number, b1:number, b2: number) => {
    if ( (a1 >= b1 && a1 < b2) || (b1 >= a1 && b1 < a2)) {
        return true;
    }
    return false;
}

class SubwayVis extends React.Component<Props, State> {
    constructor(props: Props) {
        super(props);
        this.state = {
            rootIndex : props.data.selectedVertexIndex,
            preData: {},
            width: 500,
            height: 300,
            depth:180,
            menuIndex:-1,
            menuVisible: false,
            checked:false,
            svgRef: null,
            pathColor: "red",  //路径颜色
            indexVisible: false, //是否显示标注点idx
            circles: [] //记录标注点坐标 
        };
    }

    componentDidUpdate = () => {
        if( JSON.stringify(this.state.preData) != JSON.stringify(this.props.data) ){
            new Promise((resolve) => {
                this.setState({preData:this.props.data, rootIndex:this.props.data.selectedVertexIndex}, () => {resolve(true);});
            }).then(()=>{
                console.log("update");
                this.TreePlotDebounce(true);
            })            
        }
    }

    componentDidMount() {
        // d3.select('.subway').attr("height", visOptions.svgHeight);
        this.setState({svgRef: d3.select('.subway')});
        console.log('mount');
        setTimeout(() => {
            const svgDom = document.querySelector("svg.subway");
            visOptions.svgWidth = svgDom ? svgDom.clientWidth : svgWidth;
            const h = svgDom?.clientHeight || svgHeight;
            reCalSvgHeight(h);
            if( this.props.data.graphs && this.props.data.graphs[this.props.selectedMapKey].sub[this.props.selectedVertexKey] ){
                new Promise((resolve) => {
                    this.setState({preData:this.props.data, rootIndex:this.props.data.selectedVertexIndex }, () => {resolve(true)});
                }).then(()=>{
                    this.TreePlotDebounce(true);
                });         
            }
        }, visOptions.timeOut);
    }

    /* 比例尺计算 */
    calculateScale(distancePerPixel: number) {
        for (let i = 0; i < scales.length; i++) {
            let pixel = scales[i] / distancePerPixel;
            if (pixel > pixels[0] && pixel < pixels[1]) {
                d3.select(".scale").attr("style", `width: ${pixel.toFixed(2)}px`);
                d3.select(".scaleText").attr("style", `width: ${pixel.toFixed(2)}px`).html(scales[i].toString());
                break;
            }
        }
    }

    /* 广度优先遍历，核心绘制函数 */
    bfs(tree: any) {
        const { index } = tree;
        offsetMap.set(index, [visOptions.svgHeight * 2, 0]);
        
        coordinateMap.set(index, [visOptions.startX, visOptions.startY]);
        circles.push([index, visOptions.startX, visOptions.startY]); radiusMap.set(index, polyLineStrokeWidth / visOptions.radiusRate);
        // 主干最大长度
        const longestDis = longestPath.get(index);
        const pixelsPerDistance = (visOptions.svgWidth - visOptions.padding.horizontal) / longestDis;
        const distancePerPixel = longestDis / (visOptions.svgWidth - visOptions.padding.horizontal);
        //比例尺
        this.calculateScale(distancePerPixel);
        let branchFlag = new Array(maxDepth + 1).fill(false);
        let branchCount = 0; // 第几个分支层
        let firstStroke = tree.children.length > 1;
        let cnt = 0;
        let nextdir = -1;
        dirMap.set(index, 0);
        let queue: [any, boolean] [] = [];
        const expand = expandMap.get(index);
        queue.push([tree, expand]);
        while(queue.length > 0) {
            cnt++;
            const front = queue.shift()!;
            const current = front[0];
            const {index :currentIndex, depth} = current;

            /* 计算stroke-width */
            const offset = firstStroke ? -1 : 0;
            let strokeRateCount = current.branchCount + offset;
            polyLineStrokeWidth = visOptions.initialPolyLineStrokeWidth;
            while (strokeRateCount--) {
                polyLineStrokeWidth *= visOptions.strokeWidthRate;
            }

            const color = getColor(current.checked, current.children.length);
            colorMap.set(currentIndex, color);
            leafMap.set(currentIndex, current.children.length > 0 ? false : true);
            
            // 如果该节点被缩进(非展开形态),则直接跳过其孩子节点的绘制
            const currentExpand = front[1];

            const [startX, startY] = coordinateMap.get(currentIndex);
            let upBoundArr = [visOptions.upBound, 0, visOptions.svgWidth];
            let downBoundArr = [visOptions.downBound, 0, visOptions.svgWidth];
            const len = collideArr.length;
            let maxBranchLength = -1;
            const bcount = current.children.length > 0 ? current.children.length - 1 : 0;
            const singleDir = dirMap.get(currentIndex);
             /* 绘制策略， 如果当前主干是往上(下)的，则其分支应该也向上(下)， 否则使用交替的nextdir */
            let recommendDir = offsetMap.get(currentIndex)[1] === 0 ? nextdir : offsetMap.get(currentIndex)[1];
            let isSingleBranchALeaf = false; // 记录单分支是否为叶子节点
            for (let i = 0; i < current.children.length; i++) {
                const arc = current.arcs[i];
                const {distance, critical} = arc;
                const next: any = current.children[i];
                const { index: nextIndex } = next;
                if (critical) { // 主干
                    if (bcount === 0) {
                        /*只有一条主干需要绘制，处理这种特殊情况*/
                        const expanded = expandMap.get(nextIndex);
                        const nextExpand = currentExpand === false ? false : expanded;
                        queue.push([next, nextExpand]);
                        this.paint(current, arc, nextIndex, 0, branchCount, pixelsPerDistance, currentExpand);
                    }
                    // let currentDir;
                    if (singleDir === 0) { // 当前主干之前未有过单分支
                        // 分支方向优先与当前主干偏移的方向保持一致，否则使用全局交替的nextdir
                        dirMap.set(nextIndex, bcount === 1? -recommendDir: 0);
                    } else { // 当前主干之前有过单分支
                        // 如果这次不是单分支，dir传递下去， 否则取反
                        dirMap.set(nextIndex, bcount === 1? -singleDir: singleDir);
                    }
                    
                    
                } else { // 分支
                    /* 新分支，dir设置为0 */
                    if (next.children.length === 0) isSingleBranchALeaf = true;
                    dirMap.set(nextIndex, 0);
                    // 记录所有分支的最大长度
                    let tmpDis = (distance + longestPath.get(nextIndex)) * pixelsPerDistance;
                    if (tmpDis > maxBranchLength) {
                        maxBranchLength = tmpDis;
                    }
                }
            }
            
            /* 若该节点没有分支，前面已经处理主干了，因此直接处理下一个节点， continue */
            if (bcount === 0) {
                continue;
            }

            const a1 = startX, a2 = startX + maxBranchLength;
            let b1: number, b2: number;
            
            /* 寻找最近的上、下界， 两线段水平方向有重叠， 才有可能相交 */
            for (let i = 0, j = len - 1; i < len && j >= 0; i++, j--) {
                if (startY > collideArr[i][0]) {
                    [b1, b2] = [collideArr[i][1], collideArr[i][2]];
                    if (isIntersect(a1, a2, b1, b2)) {
                        upBoundArr = collideArr[i];
                    }
                }
                if (startY < collideArr[j][0]) {
                    [b1, b2] = [collideArr[j][1], collideArr[j][2]];
                    if (isIntersect(a1, a2, b1, b2)) {
                        downBoundArr = collideArr[j];
                    }
                }
            }

            const hasDiff = function(a: number, b: number) {
                return Math.abs(a - b) > 2;
            }

            /* 计算可用空间， 每一个节点的分支在垂直方向应该是均匀的(适用于分支数>=2)，间隔为step */
            const totalSpace = downBoundArr[0] - upBoundArr[0];
            let upSpace = startY - upBoundArr[0];
            let downSpace = downBoundArr[0] - startY;
            let additional;
            if (bcount === 1) {
                // 分支数=1 时， additional必须 > 1, 若additional = 1, upSpace = downSpace时出错
                additional = 1.5;
            } else if (bcount === 2 && !hasDiff(upSpace, downSpace)) {
                // 分支树=2， 且upSpace和downSpace被认为一样大时， addtional > 0即可， additional越小， 空间利用率越高
                additional = 1;
            } else {
                // 分支数>=2的情况下 step 至少为 totalSpace / (bcount + 2) 才能确保画出所有分支
                additional = 2;
            }
            let step = totalSpace / (bcount + additional);
            console.log(currentIndex,[upSpace, downSpace],bcount + additional, step, depth);

            /* 记录各分支的垂直方向偏移 */
            let offsets: number[] = [];
           
            
            let reduce = false;
            if (bcount === 1) { 
                if (singleDir === 0) {
                    nextdir = recommendDir;
                    // reduce = true;
                } else {
                    nextdir = singleDir;
                }
                /* 若一定要使用该方向进行分支，空间可能不够，需要0.5指数退避 */
                reduce = true;
            } else {
                nextdir = recommendDir;
            }

            /* 根据上下可用空间 计算出各个分支的偏移 */
            const getOffsets = (upSpace: number, downSpace: number) => {
                for (let j = 1; ; j++) {
                    let offset = step * j;
                    const execArr = [nextdir, -nextdir];
                    for (let i = 0; i < execArr.length; i++) {
                        let currentExec = execArr[i];
                        // 处理reduce情况
                        if (reduce) {
                            if (currentExec === -1) {
                                if (isSingleBranchALeaf) offset = upSpace * 0.5;
                                else {
                                    while(offset >=  upSpace) offset*= 0.5;
                                    if (offset > 0.9 * upSpace) offset = upSpace * 0.5;
                                }
                                // offset = upSpace * 0.5;
                                offsets.push(-offset);
                                return;
                            } else {
                                if (isSingleBranchALeaf) offset = downSpace * 0.5;
                                else { 
                                    while (offset >= downSpace) offset*= 0.5;
                                    if (offset > 0.9 * downSpace) offset = downSpace * 0.5;
                                }
                                // offset = downSpace * 0.5;
                                offsets.push(offset);
                                return;
                            }
                        }
                        if (currentExec === -1 && offset < upSpace) {
                            offsets.push(-offset);
                            if (offsets.length === bcount) return;
                        }
                        if (currentExec === 1 && offset < downSpace){
                            offsets.push(offset);
                            if (offsets.length === bcount) return;
                        }
                    }              
                }
            }
            getOffsets(upSpace, downSpace);
            console.log(currentIndex, offsets);

            let j = 0;
            for (let i = 0; i < current.children.length; i++) {
                const next: any = current.children[i];
                const { index: nextIndex } = next;
                const expanded = expandMap.get(nextIndex);
                const nextExpand = currentExpand === false ? false : expanded;
                queue.push([next, nextExpand]);
                const arc = current.arcs[i];
                const {critical} = arc;
                if (!critical && !branchFlag[depth]) { 
                    branchFlag[depth] = true; branchCount++;
                }
                // stroke-width随分支层数衰减
                let verticalOffset: number = 0;
                if (!critical) {
                    verticalOffset = offsets[j++];
                }
                this.paint(current, arc, nextIndex, verticalOffset, branchCount,  pixelsPerDistance, currentExpand);
            }
           nextdir = -nextdir;
        }
        console.log("vs: ", cnt);
    }

    /* 绘制函数， 绘制一条折线 */
    paint({ index: currentIndex }: any, {distance, critical}: any, nextIndex: number, verticalOffset: number, branchCount: number, pixelsPerDistance: number
         , currentExpand : boolean) {
        // 折线终点坐标
        const [startX, startY] = coordinateMap.get(currentIndex);
        const horizontalOffset = distance * pixelsPerDistance;
        let endX, endY;
        const currentOffset = offsetMap.get(currentIndex);
        if (critical) {
            endX = startX + horizontalOffset;
            endY = startY;
            if (currentExpand) {
                this.drawPolyLine(distance, startX, startY, endX, endY);
            }
            offsetMap.set(nextIndex, currentOffset);
        } else {
            const tmpX = startX;
            // 默认的上下界为画布的上下界
            while(branchCount--) {
                verticalOffset *= visOptions.depthRate;
            }
            let ltzero = verticalOffset < 0;
            const abs = Math.min(Math.abs(verticalOffset), currentOffset[0] * visOptions.branchRate);
            verticalOffset = ltzero ? -abs : abs;
            offsetMap.set(nextIndex, [abs, ltzero? -1 : 1]);

            const tmpY = startY + verticalOffset;
            endX = startX + horizontalOffset;
            endY = tmpY;
            /*将线段插入到检测数组中*/
            let totalLength = horizontalOffset + pixelsPerDistance * longestPath.get(nextIndex);
            let len = collideArr.length;
            for (let i = len - 1; i >= 0; i--) {
                if (tmpY < collideArr[i][0]) {
                    collideArr[i + 1] = collideArr[i];
                } else {
                    collideArr[i + 1] = [tmpY, startX, startX + totalLength];
                    break;
                }
            }
            if (currentExpand) {
                this.drawPolyLine(distance, startX, startY, tmpX, tmpY, endX, endY);
            }
        }
        coordinateMap.set(nextIndex, [endX, endY]);
        if (currentExpand) {
            circles.push([nextIndex, endX, endY]);
            radiusMap.set(nextIndex, polyLineStrokeWidth / visOptions.radiusRate);
        }
    }

    /* 绘制一条折线 */
    drawPolyLine(distance: number, ...vertexs: number[]) {
        let str = "";
        for (let i = 0; i < vertexs.length; i++) {
            str = str + vertexs[i];
            str += " ";
        }
        this.state.svgRef
            .append("polyline")
            .attr("points", str)
            .attr("fill", "transparent")
            .attr('stroke', this.state.pathColor)
            .attr("stroke-width", polyLineStrokeWidth)
            .append("title")
            .html(`距离：${distance.toFixed(2)}`);
    }

    /* 为节点添加文字 */
    text(x: number, y: number, content: number) {
        const radius = radiusMap.get(content) || visOptions.defaultRadius;
        this.state.svgRef
            .append("text")
            .attr("x", x + radius)
            .attr("y", y - radius)
            .attr("font-size", visOptions.textFontSize)
            .html(`${content}`);
    }

    /* 为所有节点添加文字 */
    addText(circles: Array<[number, number, number]>) {
        circles.forEach(([index, x, y]) => {
            this.text(x, y, index);
        });
    }

    /* 移除所有节点的文字 */
    removeText() {
        d3.selectAll(".subway > text").remove();
    }
  
    /* 绘制一个节点 */
    drawCircle(index: number, cx: number, cy: number) {
        const color = colorMap.get(index);
        const leaf = leafMap.get(index);
        const expand = expandMap.get(index);
        /* 可以展开的条件： 不是叶节点并且不是展开状态 */
        const canExpand = !leaf && !expand;
        const radius = radiusMap.get(index) || visOptions.defaultRadius;
        this.state.svgRef
            .append("circle")
            .attr("cx", cx)
            .attr("cy", cy)
            .attr("r", radius)
            .attr("fill", canExpand ? visOptions.circleFillColor: "#ffffff" )
            .attr("stroke", color ? color : "#000000")
            .attr("stroke-width", visOptions.circleStrokeWidth)
            .on("contextmenu", () => { this.handleRightClick(index) })
            .on("click", (e: MouseEvent) => {this.handleExpanded(index);})
            .append("title")
            .html(index);
    }

    handleRightClick(index: number) {
        /* 获取当前右键节点的index 和 检查状态*/
        this.setState({ menuIndex: index, checked: colorMap.get(index) === color.UNCHECKED ? false : true, menuVisible: true});
    }

    handleExpanded(index: number) {
        const leaf = leafMap.get(index);
        // 叶子节点无法展开
        if (leaf) return;
        const expand = !expandMap.get(index);
        expandMap.set(index, expand);
        this.TreePlotDebounce(false);
    }
    
    TreePlot = (firstPlot: boolean) => { 
        if(this.props.data.graphs.length != 0 && this.props.data.graphs[this.props.selectedMapKey] ){
            if(!this.props.data.graphs[this.props.selectedMapKey].sub ) return;
            const tree = graph2Tree(this.props.data.graphs[this.props.selectedMapKey], this.state.rootIndex, firstPlot);
            if (!tree) return;
            this.setState({pathColor: this.props.data.graphs[this.props.selectedMapKey].color});
            /* 初始化 */
            init();
            /* 数据预处理 */
            getLongestPath(tree, 0);
            /* 遍历，画折线 */
            this.bfs(tree);
            this.setState({circles});
            /* 画节点 */
            circles.forEach(([index, startX, startY]) => {
                this.drawCircle(index, startX, startY); 
            });
            /* 节点Index标注 */
            if (this.state.indexVisible) {
                this.addText(circles);
            }
        }
    }

    TreePlotDebounce:(firstPlot: boolean) => void = debounce(this.TreePlot, visOptions.timeOut);


    handleIndexVisChange = (vis: boolean) => {
        this.setState({indexVisible: vis});
        if(vis) {
            this.addText(circles);
        } else {
            this.removeText();
        }
    }

    onChange = (value: number) => {
        const graph = this.props.data.graphs[this.props.selectedMapKey];
        for( let i = 0; i < graph.sub.length ; i++ ){
            if( value == graph.sub[i].index ){
                this.setState( {rootIndex: value}, () => {this.TreePlotDebounce(true);})
                message.success("设置成功！");
                return;
            }
        }
        message.error("不存在该节点，请重新输出！");
    }


    // 事件监听方式添加事件绑定
    
    handleMenuClick = ( event: { key: any } ) => {
        let { key } = event ;
        if( key == '1') {
            // 跳转到该节点
            const sub = this.props.data.graphs[this.props.selectedMapKey].sub;
            for (let i = 0; i < sub.length; i++) {
                const current = sub[i];
                const { index } = current;
                if (index == this.state.menuIndex) {
                    const record = {
                        index,
                        key: i
                    }
                    this.props.onClickJumpToVex(record);
                    break;
                }
            }
        } else if( key == '2'){
            const ws = new WebSocket(_SOCKETLINK);
            ws.onopen = () => {
                ws.send(
                    JSON.stringify({
                        modify : {
                            index : this.state.menuIndex,
                            checked : !this.state.checked
                        }
                    })
                );
            };
            ws.onmessage = (msg) => {
                const {data} = msg;
                ws.close();
                try {
                    const obj = JSON.parse(data);
                    if (obj.type == "error") {
                        message.error(obj.message);
                    } else if (obj.type == "success") {
                        message.success(obj.message);
                    } else {
                        new Promise(resolve => {
                            console.log(this.props);
                            resolve(this.props.setData(obj));
                        }).then(() => {
                            this.props.initSelectedKey();
                        })
                    }
                } catch  {
                    console.log(data);
                }
            }
        } else if( key == '3' ){
            this.onChange(this.state.menuIndex);
        }
        this.setState({menuVisible:false});
    }

    render() {
        console.log('render');
        const menu = (
            <Menu onClick={this.handleMenuClick}>
              <Menu.Item key="1">跳转</Menu.Item>
              <Menu.Item key="2">修改检查状态</Menu.Item>
              <Menu.Item key="3">以该顶点为根重置</Menu.Item>
            </Menu>
          );
        return (
            <div className="subway-wrapper">
                <div className="subway-wrapper-limited">
                <Row className="row">
                    <Col>
                        <Space>
                            <span style={{marginLeft: 20}}>选择端点Index</span>： <InputNumber value={this.props.data.selectedVertexIndex} onChange={(v)=>this.onChange(v)} />
                            <span> Index: </span><Switch checkedChildren="显示" unCheckedChildren="隐藏" onChange={this.handleIndexVisChange}></Switch>
                        </Space>
                    
                    </Col>
                    <div className="scale"></div>
                    <div className="scaleText"></div>
                </Row>
                <Dropdown overlay={menu} trigger={['contextMenu']} visible={this.state.menuVisible}>
                    <div className="svg-wrapper" onClick={() => this.setState({menuVisible:false})}>
                            <svg className="subway"></svg>
                    </div>
                </Dropdown>
                </div>
            </div>
        );
    }
}

export default SubwayVis;