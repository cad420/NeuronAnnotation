import { Statistic, Row, Col, Button } from 'antd';
import React from 'react';

class Info extends React.Component {
    constructor(props){
        super(props)
    }
    render(){
        var totalLength = 0;
        var forkCount = 0;
        var leafCount = 0;
        const graph = this.props.data.graphs[this.props.selectedMapKey];
        if( graph ){
            for( let i = 0 ; i < graph.sub.length ; i ++ ){
                if( graph.sub[i].arc.length == 1 ) leafCount ++;
                if( graph.sub[i].arc.length > 2 ) forkCount ++;
                for( let j = 0 ; j < graph.sub[i].arc.length ; j ++ ){
                    totalLength += graph.sub[i].arc[j].distance;
                }
            }
        }

        return (
            <div>
                <Row gutter={16}>
                <Col span={6}>
                    <Statistic title="总顶点数" value={graph ? graph.sub.length : 0} />
                </Col>
                <Col span={6}>
                    <Statistic title="总距离" value={(totalLength/2).toFixed(2)} precision={2} />
                </Col>
                <Col span={6}>
                    <Statistic title="分支点数" value={forkCount} />
                </Col>
                <Col span={6}>
                    <Statistic title="端点数" value={leafCount} />
                </Col>
            </Row>
            </div>
        )
        }
};

export default Info;