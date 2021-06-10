import React from "react";
import { Space, Typography, Divider, Button, Select, Tooltip } from 'antd';
import { DownloadOutlined } from '@ant-design/icons';
import { Upload, message } from 'antd';
import { UploadOutlined } from '@ant-design/icons';
import { withSuccess } from "antd/lib/modal/confirm";
import { linkVertical } from "d3-shape";

const _DOWNLOAD = "http://127.0.0.1:12121/download";
const { Option } = Select;
const LoadAndSave: React.FC = (props) => {
  const UPLOAD = {
    name: 'file',
    action: 'http://127.0.0.1:12121/upload',
    method: 'post',
    headers: {
      authorization: 'authorization-text',
    },
    showUploadList: true,
    showPreviewIcon: true,
 
    onChange(info) {
      if (info.file.status !== 'uploading') {
        console.log(info.file, info.fileList);
      } else if (info.file.status === 'done') {
        message.success(`${info.file.name} 上传成功`);
      } else if (info.file.status === 'error') {
        message.error(`${info.file.name} 上传失败`);
      }
    }
  } 

  const downLoad = () =>{
    //
  }

  return (
      <div className="ls">
      <Space>
        <Tooltip
          placement="left"
          title="选择工作集合"
          arrowPointAtCenter
          color="blue"
          >
          <Select
            defaultValue="请选择工作集合"
            value={props.data.selectedTableName}
            style={{ width: 200 }}
            size="large"
            options={props.data.tableList}
            onChange={props.changeTable}></Select>
      </Tooltip>
      <Upload {...UPLOAD}>
      <Tooltip
          placement="bottom"
          title="上传SWC文件"
          arrowPointAtCenter
          color="blue"
          >
        <Button icon={<UploadOutlined />} size="large" style={{ marginLeft: 10 }} >上传</Button>
        </Tooltip>
    </Upload>
      <Tooltip
            placement="bottom"
            title="导出SWC文件"
            arrowPointAtCenter
            color="blue"
            >
        <Button icon={<DownloadOutlined />} size="large" href={_DOWNLOAD} download={props.data.selectedTableName+".swc"}  >下载</Button>
        </Tooltip>
          </Space>
      </div>
  );
}

export default LoadAndSave;
