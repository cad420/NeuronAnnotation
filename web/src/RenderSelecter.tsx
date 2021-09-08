import React from "react";
import { Button, Radio, Space, Divider } from 'antd';
import {BulbOutlined} from '@ant-design/icons'
import axios from 'axios';

interface Props {};
interface State {
    selectedRender: string
}

const _SOCKETLINK = "ws://127.0.0.1:12121/info";
class RenderSelecter extends React.Component<Props, State> {
    constructor(props: Props){
        super(props);
        this.state = {
            selectedRender: "DVR",
        };
    }

    handleRenderChange = (v:string) => {
        let self = this;
        this.setState({selectedRender:v});
        const ws = new WebSocket(_SOCKETLINK);
        ws.binaryType = "arraybuffer";
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(
              JSON.stringify({
                  modify : {
                    selectedRender : v
                  }
              })
          );
        };
        ws.onmessage = (msg) => {
            const {data} = msg;
            ws.close();
        }
    };

    render(){
        const {selectedRender} = this.state;
        return (
            <span>
                <Space align="baseline" split={<Divider type="vertical" />}>
                    <h4><BulbOutlined />选择渲染方式</h4>
                        <Radio.Group value={selectedRender} onChange={(v)=>this.handleRenderChange(v.target.value)}>
                            <Radio.Button value="MIP">MIP</Radio.Button>
                            <Radio.Button value="DVR">DVR</Radio.Button>
                            <Radio.Button value="LINE">LINE</Radio.Button>
                        </Radio.Group>
                </Space>
          </span>
        );
    }
};

export default RenderSelecter;