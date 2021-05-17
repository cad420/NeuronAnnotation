import React, { useEffect, useState } from "react";
import { Button, Layout, Divider, Card, Space, Row, Col, Alert, message } from 'antd';
import Image from "./InteractiveImage";
import SrcTable from "./SrcTable";
import LoadAndSave from "./LoadAndSave";
import ToolsHeader from "./ToolsHeader";
import RenderSelecter from "./RenderSelecter";
import TreeVisualization from "./TreeVisualization";
import "./style.css";
import Info from "./Info";

const { Header, Footer, Sider, Content } = Layout;
const _SOCKETLINK = "ws://127.0.0.1:12121/info";

const Format: React.FC = () => {

    const [src,setSrc] = useState("");
    const [data,setData] = useState(
        {
            "type":"structure",
            "graphs":[
                {
                "sub":[
                {   "key":0,
                    "index":1,
                    "lastEditTime":"2021-11-10 20:20:20",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 3,
                            "distance" : 10.332
                        },
                        {
                            "headVex" : 1,
                            "tailVex" : 4,
                            "distance" : 7.25
                        },
                        {
                            "headVex" : 1,
                            "tailVex" : 7,
                            "distance" : 8
                        },
                    ]
                },
                {   "key":1,
                    "index":3,
                    "lastEditTime":"2021-11-10 20:20:20",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 3,
                            "distance" : 10.332
                        },
                        {                        
                            "headVex" : 3,
                            "tailVex" : 8,
                            "distance": 11,
                        },
                        {
                            "headVex" : 3,
                            "tailVex" : 9,
                            "distance": 12,
                        }
                    ]
                },
                {
                    "key":2,
                    "index":4,
                    "lastEditTime":"2021-11-10 20:20:21",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 4,
                            "distance" : 7.25
                        },
                        {
                            "headVex" : 4,
                            "tailVex" : 5,
                            "distance" : 12
                        }
                    ]
                },
                {
                    "key":3,
                    "index":5,
                    "lastEditTime":"2021-4-29 20:00:21",
                    "arc":[
                        {
                            "headVex" : 4,
                            "tailVex" : 5,
                            "distance" : 12
                        },
                        {
                            "headVex" : 5,
                            "tailVex" : 6,
                            "distance" : 7
                        }
                    ]
                },{
                    "key":4,
                    "index":6,
                    "lastEditTime":"2021-4-29 08:00:54",
                    "arc":[
                        {
                            "headVex" : 5,
                            "tailVex" : 6,
                            "distance" : 7
                        }
                    ]
                },{
                    "key":5,
                    "index":7,
                    "lastEditTime":"2021-4-29 08:00:54",
                    "arc":[
                        {
                            "headVex":7,
                            "tailVex":1,
                            "distance":8
                        }
                    ]
                },{
                    "key":6,
                    "index":8,
                    "lastEditTime":"2021-4-29 08:00:54",
                    "arc":[
                        {
                            "headVex":3,
                            "tailVex":8,
                            "distance":11
                        }
                    ]
                },{
                    "key":7,
                    "index":9,
                    "lastEditTime":"2021-4-29 08:00:54",
                    "arc":[
                        {
                            "headVex":3,
                            "tailVex":9,
                            "distance":17
                        }
                    ]
                }
                ],
                "color":"#cc00aa",
                "name":"路径1",
                "index":0,
                "status":true,
                "key":0
                }
            ],
            "selectedVertexIndex":1,
            "selectedMapIndex":0,
            "selectedTool":0
        }
    );
    const [selectedMapKey, setSelectedMapKey] = useState(0);
    const [selectedVertexKey, setSelectedVertexKey] = useState(0);
    const [selectedTool,setSelectecTool] = useState(1);
    const handleToolsChange = (e: { target: { value: React.SetStateAction<number>; }; })=>{
        console.log("handleToolsChange",e.target.value);
        setSelectecTool(e.target.value);
    }

    const rowSelection = {
        selectedRowKeys: [data.selectedMapIndex],
        //设置默认选中的map
        onChange: (selectedRowKeys: React.Key[], selectedRows: any[]) => {
            changeData({selectedLineIndex: selectedRows[0].index}); //index是服务器存储的顺序
            setSelectedMapKey(selectedRows[0].key); //key是客户端存储的顺序
            console.log(`selectedRowKeys: ${selectedRowKeys}`, 'selectedRows: ', selectedRows[0]);
        },
    };

    const onClickJumpToVex = (record: { index: any; key: React.SetStateAction<number>; })=>{
        changeData({selectedVertexIndex : record.index});
        setSelectedVertexKey(record.key);
    }

    const initSelectedKey = () => {
        //console.log("initSelectedKey")
        for( let i = 0 ; i < data.graphs.length ; i ++ ){
            if(data.graphs[i].index = data.selectedMapIndex){
                setSelectedMapKey(i);
                return;
            }
        }
    }

    const changeTable = (table) =>{
        changeData({selectedTableName : table});
    }

    const changeData = (_data) => {
        if(_data && data){
            console.log("changedata:",data)
            setData({...data,..._data});
            const ws = new WebSocket(_SOCKETLINK);
            ws.binaryType = "arraybuffer";
            ws.onopen = () => {
                console.log("连接成功，准备发送更新数据");
                ws.send(
                    JSON.stringify({
                        modify : {
                        ..._data
                        }
                    })
                );
            }
            ws.onerror = () =>{
                console.log("连接渲染服务器出错！");
                message.error("连接渲染服务器出错！");
            }
            ws.onmessage = (msg) => {
                const { data } = msg;
                if (typeof msg.data === "object") {
                  const bytes = new Uint8Array(msg.data);
                  const blob = new Blob([bytes.buffer], { type: "image/jpeg" });
                  const url = URL.createObjectURL(blob);
                  setSrc(url);
                  ws.close();
                  return;
                }  
                try {
                  const obj = JSON.parse(data);
                  if( obj.type == "error" ){
                    console.log(obj.message);
                    message.error( obj.message );
                  }else if( obj.type == "success" ){
                    console.log(obj.message);
                    message.success( obj.message );
                  }else{
                    console.log(obj);
                    let p = new Promise(resolve =>{
                      resolve(setData(obj));
                    }).then(()=>{
                      initSelectedKey();
                    })
                    console.log(obj.error);
                  }
                  ws.close();
                } catch {
                  console.log(data);
                }
            };
        }
    }
    return (
            <div>
                <Layout>
                <Header style={{marginTop:40}}>
                    <Row> 
                        <Col flex="1 1 300px">
                            <RenderSelecter />
                        </Col>
                        <Col flex="auto">
                        <ToolsHeader data={data}/>
                        </Col>
                        <Col flex="auto">
                            <LoadAndSave
                                data={data} 
                                changeTable={changeTable}
                            />
                        </Col>
                    </Row>
                </Header>
                <Layout>
                <Sider width={500}>
                    <SrcTable
                        style={{ width: 480 }}
                        rowSelection={rowSelection}
                        onClickJumpToVex={onClickJumpToVex}
                        data={data}
                        setData={setData}
                        setSrc={setSrc}
                        initSelectedKey={initSelectedKey}
                        />
                    <Divider dashed />
                    <Card style={{ width: 480 }}>
                        <Info
                            data={data}
                            selectedMapKey={selectedMapKey}
                        />
                    </Card>
                </Sider>
                <Content >
                    <Divider dashed />
                    <Row>
                        <Image
                            selectedTool={selectedTool}
                            setData={setData}
                            initSelectedKey={initSelectedKey}
                            src={src}
                            setSrc={setSrc}
                        />
                    </Row>
                </Content>
                </Layout>
                </Layout>
                <Footer>
                    <Col span={24}>
                        <Card>
                            <TreeVisualization
                                data={data}
                                onClickJumpToVex={onClickJumpToVex}
                                selectedMapKey={selectedMapKey}
                                selectedVertexKey={selectedVertexKey}
                            />
                        </Card>
                    </Col>
                </Footer>
            </div>
    )
};

export default Format;