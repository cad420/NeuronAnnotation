import React from "react";
import { Radio, RadioChangeEvent, Tooltip } from 'antd';
import {
    DragOutlined,
    ScissorOutlined,
    EditOutlined,
    RadiusSettingOutlined,
    DeleteOutlined,
  } from '@ant-design/icons';
import { Data } from "./format";

const _SOCKETLINK = "ws://127.0.0.1:12121/info";

interface Props {
    data: Data
}

interface State {
    selectedTool: any
}

class ToolsHeader extends React.Component<Props, State> {
    constructor(props: Props){
        super(props);
        this.state = {
            selectedTool : this.props.data.selectedTool
        }
    }

    render(){
        const {selectedTool} = this.state;

        const handleToolsChange = (e: RadioChangeEvent) => {
            const ws = new WebSocket(_SOCKETLINK);
            this.setState({selectedTool: e.target.value});
            ws.binaryType = "arraybuffer";
            ws.onopen = () => {
                console.log("连接成功，准备发送更新数据");
                ws.send(
                  JSON.stringify({
                      modify : {
                        selectedTool : e.target.value
                      }
                  })
              );
            };
            ws.onmessage = (msg) => {
                const {data} = msg;
                ws.close();
            }
        }

        return (
            <div>
                <Radio.Group defaultValue={selectedTool} onChange={handleToolsChange} size="large">
                <Tooltip
                    placement="bottom"
                    title="拖动模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                    <Radio.Button value={0} ><DragOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="点标注模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value={1} ><EditOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="剪切模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value={2} ><ScissorOutlined /></Radio.Button>
                </Tooltip>
                {/* <Tooltip
                    placement="bottom"
                    title="选框模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value={3} ><RadiusSettingOutlined /></Radio.Button>
                </Tooltip> */}
                <Tooltip
                    placement="bottom"
                    title="消除模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value={4} ><DeleteOutlined /></Radio.Button>
                </Tooltip>
            </Radio.Group>
          </div>
        );
    }
};

export default ToolsHeader;