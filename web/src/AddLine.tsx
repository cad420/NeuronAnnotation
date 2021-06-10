import React from "react";
import { Button, Radio, Space, Divider, message } from 'antd';

import {BulbOutlined, FileAddOutlined, MenuOutlined } from '@ant-design/icons';
import axios from 'axios';


const _SOCKETLINK = "ws://127.0.0.1:12121/info";
class AddLine extends React.Component {
    constructor(props){
        super(props)
    }

    handleAdd = () => {
        let self = this;
        const ws = new WebSocket(_SOCKETLINK);
        ws.onopen = () => {
            console.log("连接成功，添加路径");
            ws.send(JSON.stringify({addline: 1}));
            const hide = message.loading('正在添加...', 0);
            setTimeout(hide, 500);
        }
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
                    // let p = new Promise(resolve => {
                    //     resolve(self.props.setData(obj));
                    // }).then(() => {
                    //     self
                    //         .props
                    //         .initSelectedKey();
                    // })
                }
            } catch  {
                console.log(data);
            }
        };
    };


    render(){
        const self = this;
        return (
            <div style={{margin:'0 20px'}}>
                <Button onClick={()=>self.handleAdd()} type="primary" style={{ marginBottom: 16 }} ><FileAddOutlined />添加神经元</Button>
          </div>
        );
    }
};

export default AddLine;