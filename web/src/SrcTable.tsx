import React, {useEffect, useRef, useState, useContext} from 'react';
import {
    Table,
    Input,
    Button,
    Popconfirm,
    Form,
    Space,
    message,
    Popover,
    Tooltip
} from 'antd';
import {
    EditOutlined,
    SearchOutlined,
    EyeInvisibleOutlined,
    EyeOutlined,
    CheckCircleOutlined,
    InfoCircleOutlined,
    FileAddOutlined,
    MenuOutlined
} from '@ant-design/icons';
import {SortableContainer, SortableElement, SortableHandle} from 'react-sortable-hoc';
import arrayMove from 'array-move';

import Highlighter from 'react-highlight-words';
import InputColor, { Color } from 'react-input-color';
import { FormInstance } from 'antd/lib/form';
import { arc } from 'd3-shape';
import { subset } from 'd3-array';
import { autoType } from 'd3-dsv';

const EditableContext = React.createContext<FormInstance<any> | null>(null);
const _SOCKETLINK = "ws://10.76.3.92:12121/info";

interface Item {
  key: string;
  name: string;
  color: string;
  status: boolean;   
}

interface EditableRowProps {
  index: number;
}

const EditableRow: React.FC<EditableRowProps> = ({ index, ...props }) => {
    const [form] = Form.useForm();
    return (
      <Form form={form} component={false}>
        <EditableContext.Provider value={form}>
          <tr {...props} />
        </EditableContext.Provider>
      </Form>
    );
  };

interface EditableCellProps {
    title: React.ReactNode;
    editable: boolean;
    children: React.ReactNode;
    dataIndex: keyof Item;
    record: Item;
    handleSave: (record: Item) => void;
}

const EditableCell: React.FC<EditableCellProps> = ({
    title,
    editable,
    children,
    dataIndex,
    record,
    handleSave,
    ...restProps
  }) => {
    const [editing, setEditing] = useState(false);
    const inputRef = useRef<Input>(null);
    const form = useContext(EditableContext)!;
  
    useEffect(() => {
      if (editing) {
        inputRef.current!.focus();
      }
    }, [editing]);
  
    const toggleEdit = () => {
      setEditing(!editing);
      form.setFieldsValue({ [dataIndex]: record[dataIndex] });
    };
  
    const save = async () => {
      try {
        const values = await form.validateFields();
  
        toggleEdit();
        handleSave({ ...record, ...values });
      } catch (errInfo) {
        console.log('Save failed:', errInfo);
      }
    };
  
    let childNode = children;
  
    if (editable) {
      childNode = editing ? (
        <Form.Item
          style={{ margin: 0 }}
          name={dataIndex}
          rules={[
            {
              required: true,
              message: `${title} 不能为空`,
            },
          ]}
        >
          <Input ref={inputRef} onPressEnter={save} onBlur={save} />
        </Form.Item>
      ) : (
        <div className="editable-cell-value-wrap" style={{ paddingRight: 24 }} onClick={toggleEdit}>
          {children}
        </div>
      );
    }
  
    return <td {...restProps}>{childNode}</td>;
  };
  
  type EditableTableProps = Parameters<typeof Table>[0];
  
  interface DataType {
    key: React.Key;
    name: String;
    color: String;
    status: Boolean;
    index: number;
    sub: Array<SubDataType>;
  }
  
  interface SubDataType {
    key: React.Key;
    index: number;
    name: String;
    length: String;
    lastEditTime: String;
  }

  interface EditableTableState {
    popVisible: Boolean;
    searchText: string;
    searchedColumn: string;
    selectedRowKeys: React.Key[];
    //dataSource: DataType[];
    count: number;
  }
  
type ColumnTypes = Exclude<EditableTableProps['columns'], undefined>;

class SrcTable extends React.Component<EditableTableProps, EditableTableState>{
    columns: (ColumnTypes[number] & { editable?: boolean; dataIndex: string })[];
    constructor(props:EditableTableProps){
        super(props);
        this.columns = [
            {
              title: '名称',
              dataIndex: 'name',
              key: 'name',
              editable: true,
            },
            {
              title: '颜色',
              dataIndex: 'color',
              key: 'color',
              render: (c,row) => (
                <div>
                <InputColor
                  initialValue={c}
                  onChange={(color:Color)=>this.setColor(color,row)}
                />
                </div>
              )
            },
            {
                title: '状态',
                dataIndex: 'status',
                key: 'status',
                render: (st,row) => (
                    <div>
                        <a onClick={()=>this.changeStatus(st,row)}>
                            {st ? (<EyeOutlined/>):(<EyeInvisibleOutlined/>)}
                        </a>
                    </div>
                )
            },
            {
              title: '操作',
              key: 'action',
              dataIndex: 'action',
              render: (_, record) => (
                  <div>
                    <Space size="middle">
                        <Popconfirm title="确定删除吗？" onConfirm={() => this.handleDelete(record)}>
                            <Button danger type="primary" size="small" onClick={()=>this.showPopconfirm()}>删除</Button>
                        </Popconfirm>
                        <Popover content={()=>{
                          if( ! record )return;
                          var totalLength = 0;
                          var forkCount = 0;
                          var leafCount = 0;
  
                          for( let i = 0 ; i < record.sub.length ; i ++ ){
                            if( record.sub[i].arc.length == 1 ) leafCount ++;
                            if( record.sub[i].arc.length > 2 ) forkCount ++;
                            for( let j = 0 ; j < record.sub[i].arc.length ; j ++ ){
                              totalLength += record.sub[i].arc[j].distance;
                            }
                          }
                          return (
                          <div>
                            <p>{"顶点数：\t\t"+record.sub.length}</p>
                            <p>{"总长度：\t\t"+(totalLength/2).toFixed(2)}</p>
                            <p>{"分叉点数：\t\t"+forkCount}</p>
                            <p>{"端点数：\t\t"+leafCount}</p>
                          </div>
                          )
                        }
                        } title={record.name + " 详情"} trigger="focus">
                            <Button type="primary" size="small">详情</Button>
                        </Popover>
                    </Space>
                </div>               
              ),
            },
          ];
        this.state = {
            popVisible: false,
            searchText: '',
            selectedRowKeys: [],
            searchedColumn: '',
            count: this.props.data ? this.props.data.graphs.length : 0,
        } 
    }

    getColumnSearchProps = (dataIndex: string) => ({
      filterDropdown: ({ setSelectedKeys, selectedKeys, confirm, clearFilters }) => (
        <div style={{ padding: 8 }}>
          <Input
            ref={node => {
              this.searchInput = node;
            }}
            placeholder={`Search ${dataIndex}`}
            value={selectedKeys[0]}
            onChange={e => setSelectedKeys(e.target.value ? [e.target.value] : [])}
            onPressEnter={() => this.handleSearch(selectedKeys, confirm, dataIndex)}
            style={{ width: 188, marginBottom: 8, display: 'block' }}
          />
          <Space>
            <Button
              type="primary"
              onClick={() => this.handleSearch(selectedKeys, confirm, dataIndex)}
              icon={<SearchOutlined />}
              size="small"
              style={{ width: 90 }}
            >
              Search
            </Button>
            <Button onClick={() => this.handleReset(clearFilters)} size="small" style={{ width: 90 }}>
              Reset
            </Button>
            <Button
              type="link"
              size="small"
              onClick={() => {
                confirm({ closeDropdown: false });
                this.setState({
                  searchText: selectedKeys[0],
                  searchedColumn: dataIndex,
                });
              }}
            >
              Filter
            </Button>
          </Space>
        </div>
      ),
      filterIcon: (filtered: any) => <SearchOutlined style={{ color: filtered ? '#1890ff' : undefined }} />,
      onFilter: (value: string, record: { [x: string]: { toString: () => string; }; }) =>
        record[dataIndex]
          ? record[dataIndex].toString().toLowerCase().includes(value.toLowerCase())
          : '',
      onFilterDropdownVisibleChange: (visible: any) => {
        if (visible) {
          setTimeout(() => this.searchInput.select(), 100);
        }
      },
      render: (text: { toString: () => string; }) =>
        this.state.searchedColumn === dataIndex ? (
          <Highlighter
            highlightStyle={{ backgroundColor: '#ffc069', padding: 0 }}
            searchWords={[this.state.searchText]}
            autoEscape
            textToHighlight={text ? text.toString() : ''}
          />
        ) : (
          text
        ),
    });
  
    handleSearch = (selectedKeys: any[], confirm: () => void, dataIndex: any) => {
      confirm();
      this.setState({
        searchText: selectedKeys[0],
        searchedColumn: dataIndex,
      });
    };
  
    handleReset = (clearFilters: () => void) => {
      clearFilters();
      this.setState({ searchText: '' });
    };  

    handleDelete = (record) => {
        //服务器自删除后更新数据，并提示删除成功
        let self = this;
        const ws = new WebSocket(_SOCKETLINK);
        ws.binaryType = "arraybuffer";
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(JSON.stringify({deleteline : 1 , index: record.index  }));
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
      };


    setColor = (color:Color,row:DataType) =>{
        const newData = [...this.props.data.graphs];
        const index = newData.findIndex(item => row.key === item.key);
        const item = newData[index];
        if(item.color == color.hex) return;
        
        item.color = (color.hex).substr(0,7);
        console.log(item.color);
        const ws = new WebSocket(_SOCKETLINK);
  
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(
                JSON.stringify({
                  modify : {
                    index : row.index,
                    color : item.color
                  }
                })
            );
            const hide = message.loading('正在修改...', 0);
            setTimeout(hide, 500);
        }
        ws.onerror = () =>{
          console.log("连接渲染服务器出错！");
          message.error("连接渲染服务器出错！");
        } 
    }

    showPopconfirm = () =>{
        this.setState({
            popVisible: true,
        })
    }

    changeChecked = ( checked:boolean, row:SubDataType) =>{
      let self = this;
      const ws = new WebSocket(_SOCKETLINK);
      ws.binaryType = "arraybuffer";
      ws.onopen = () => {
          console.log("连接成功，准备发送更新数据");
          ws.send(
            JSON.stringify({
                modify : {
                  index : row.index,
                  checked : !checked
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
    }

    changeStatus = ( st:boolean,row:DataType ) =>{
      let self = this;
      const ws = new WebSocket(_SOCKETLINK);
      ws.binaryType = "arraybuffer";
      ws.onopen = () => {
          console.log("连接成功，准备发送更新数据");
          ws.send(
            JSON.stringify({
                modify : {
                  index : row.index,
                  visible : !st
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
    };

    check = (record:DataType) => {
        console.log(record)
    }

    handleSave = (row:DataType) => {
        const newData = [...this.props.data.graphs];
        const index = newData.findIndex(item => row.key === item.key);
        const item = newData[index];
        if( item.name == row.name ) return;

        let self = this;
        const ws = new WebSocket(_SOCKETLINK);
        ws.binaryType = "arraybuffer";
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(
              JSON.stringify({
                  modify : {
                    index : row.index,
                    name : row.name
                  }
              })
          );
        };
        ws.onmessage = (msg) => {
            const {data} = msg;
            ws.close();
        }

    };

    onClickJump = (record) =>{
        console.log(record);
        this.props.onClickJumpToVex(record);
    }

    timestampToTime = (timestamp) =>{
      var date = new Date(timestamp * 1000);//时间戳为10位需*1000，时间戳为13位的话不需乘1000
      var Y = date.getFullYear() + '-';
      var M = (date.getMonth()+1 < 10 ? '0'+(date.getMonth()+1):date.getMonth()+1) + '-';
      var D = (date.getDate()< 10 ? '0'+date.getDate():date.getDate())+ ' ';
      var h = (date.getHours() < 10 ? '0'+date.getHours():date.getHours())+ ':';
      var m = (date.getMinutes() < 10 ? '0'+date.getMinutes():date.getMinutes()) + ':';
      var s = date.getSeconds() < 10 ? '0'+date.getSeconds():date.getSeconds();
      return Y+M+D+h+m+s;
  }

    render(){
        let self = this;
        var dataSource;
        if ( self.props.data && self.props.data.graphs ) {
            dataSource = [...self.props.data.graphs];
        }else{
            dataSource = [];
        }
        const expandedRowRender = (data: { sub: readonly any[] | undefined; }) => {
          const columns = [
            { title: 'id', dataIndex: 'index', key: 'index'},
            { title: '关联节点数',
              dataIndex: 'linkedVertexNum',
              key: 'linkedVertexNum',
              sorter:(a,b)=>a.arc.length - b.arc.length,
              filters: [
                { text: '端点', value: 1 },
                { text: '中间节点', value: 2 },
                { text: '分叉点', value: 3}
              ],
              onFilter: (value, record) => {
                if(value != 3){
                  return record.arc.length == value;
                }else{
                  return record.arc.length >= value;
                }
              },
              render:(_,record)=>(<p>{record.arc.length}</p>)},
            { title: '状态', dataIndex: 'checked', key: 'checked',
            filters:[
              { text: '已检查', value: true },
              { text: '未检查', value: false }
            ],
            onFilter:(value,record) =>{
              return value == record.checked;
            },
            render:
            (checked,record)=>
            (
                <div>
                      {/* <a onClick={()=>this.changeStatus(checked,row)}> */}
                      <Tooltip
                        title={checked?("已检查"):("未检查")}
                        color={checked?'blue':'orange'}
                      >
                      <a style={{ color: checked?("blue"):("orange")}} onClick={()=>this.changeChecked(checked,record)}>
                          {checked ? (<CheckCircleOutlined />):(<InfoCircleOutlined />)}
                      </a>
                      </Tooltip>
                  </div>
            )
  
              },
            { title: '操作', key:'action', dataIndex: 'action',
            render: ( _, record)=>(
              //如何传入row TODO
              <div>
                <Button onClick={()=>self.onClickJump(record)}>跳转</Button>
              </div>
            )
          }
          ];
          return <Table columns={columns} dataSource={data.sub} pagination={{defaultPageSize:5}}/>;
        };

        const components = {
            body: {
              row: EditableRow,
              cell: EditableCell,
            },
          };
          const columns = self.columns.map(col => {
            if (!col.editable) {
              return col;
            }
            return {
              ...col,
              ...self.getColumnSearchProps('name'), //search
              onCell: (record: DataType) => ({
                record,
                editable: col.editable,
                dataIndex: col.dataIndex,
                title: col.title,
                handleSave: self.handleSave,
              }),
            };
          });
        return(
        <div style={{margin:'0 8px'}}>
            <Table components={components}
            rowSelection={{type:"radio",...this.props.rowSelection}}
            rowClassName={() => 'editable-row'}
            rowKey="index"
            columns={columns as ColumnTypes}
            dataSource={dataSource} 
            pagination={{hideOnSinglePage:true}}
            expandable={{ expandedRowRender }}
            defaultPageSize="10"
            showHeader={true}
            size='small'
            style={{border:10}}
             />
        </div>
        )
    };
};

export default SrcTable;
